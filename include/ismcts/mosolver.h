/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_MOSOLVER_H
#define ISMCTS_MOSOLVER_H

#include "solverbase.h"
#include "game.h"
#include "tree/nodetypes.h"
#include "tree/policies.h"
#include "execution.h"
#include "utility.h"

#include <memory>
#include <vector>
#include <map>
#include <future>

namespace ISMCTS
{

template<class Move, class _ExecutionPolicy = Sequential>
class MOSolver : public SolverBase<Move,_ExecutionPolicy>
{
public:
    using SolverBase<Move,_ExecutionPolicy>::SolverBase;
    using typename SolverBase<Move,_ExecutionPolicy>::NodePtr;
    using _ExecutionPolicy::numThreads;

    // The search trees for the current observer, one per player
    using TreeMap = std::map<unsigned int, NodePtr>;

    // The set of tree maps, one for each thread
    using TreeList = std::vector<TreeMap>;

    virtual Move operator()(Game<Move> const &rootState) const override
    {
        std::vector<std::future<void>> futures(numThreads());
        setupTrees(rootState);

        std::transform(m_trees.begin(), m_trees.end(), futures.begin(), [&](auto &map){
            return this->launch([&]{ search(map, rootState); });
        });
        for (auto &f : futures)
            f.get();

        std::vector<NodePtr> currentPlayerTrees(numThreads());
        std::transform(m_trees.begin(), m_trees.end(), currentPlayerTrees.begin(), [&](auto &map){
            return map[rootState.currentPlayer()];
        });
        return MOSolver::template bestMove<Move>(currentPlayerTrees);
    }

    TreeList currentTrees() const
    {
        return m_trees;
    }

protected:
    using NodePtrMap = std::map<unsigned int, Node<Move>*>;

    void search(TreeMap &trees, Game<Move> const &rootState) const
    {
        NodePtrMap roots;
        for (auto &pair : trees)
            roots.emplace(pair.first, pair.second.get());
        auto randomState = rootState.cloneAndRandomise(rootState.currentPlayer());
        select(roots, *randomState);
        expand(roots, *randomState);
        this->simulate(*randomState);
        backPropagate(roots, *randomState);
    }

    void select(NodePtrMap &nodes, Game<Move> &state) const
    {
        auto const validMoves = state.validMoves();
        auto const player = state.currentPlayer();
        auto const &targetNode = nodes[player];
        if (!MOSolver::selectNode(targetNode, validMoves)) {
            auto const selection = this->selectChild(targetNode, state, validMoves);
            for (auto &node : nodes)
                node.second = MOSolver::findOrAddChild(node.second, state, selection->move());
            state.doMove(selection->move());
            select(nodes, state);
        }
    }

    Node<Move> static *findOrAddChild(Node<Move> *node, Game<Move> &state, Move const &move)
    {
        auto const &children = node->children();
        auto const pos = std::find_if(children.begin(), children.end(), [&](auto const &c){ return c->move() == move; });
        return pos < children.end() ? pos->get() : MOSolver::addChild(node, state, move);
    }

    void expand(NodePtrMap &nodes, Game<Move> &state) const
    {
        auto const player = state.currentPlayer();
        auto const untriedMoves = nodes[player]->untriedMoves(state.validMoves());
        if (!untriedMoves.empty()) {
            auto const move = randomElement(untriedMoves);
            for (auto &node : nodes)
                node.second = MOSolver::findOrAddChild(node.second, state, move);
            state.doMove(move);
        }
    }

    void static backPropagate(NodePtrMap &nodes, Game<Move> const &state)
    {
        for (auto node : nodes)
            SolverBase<Move,_ExecutionPolicy>::backPropagate(node.second, state);
    }

private:
    mutable TreeList m_trees;

    void setupTrees(Game<Move> const &rootState) const
    {
        auto const &state = dynamic_cast<POMGame<Move> const &>(rootState);
        m_trees.resize(numThreads());
        std::generate(m_trees.begin(), m_trees.end(), [&]{
            TreeMap map;
            for (auto player : state.players())
                map.emplace(player, MOSolver::newNode(state));
            return map;
        });
    }
};

} // ISMCTS

#endif // ISMCTS_MOSOLVER_H
