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

#include <memory>
#include <vector>
#include <map>
#include <thread>
#include <chrono>

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

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        std::vector<std::thread> threads(numThreads());
        setupTrees(rootState);

        std::transform(m_trees.begin(), m_trees.end(), threads.begin(), [&](auto &map){
            return this->launch([&]{ search(map, rootState); });
        });
        for (auto &t : threads)
            t.join();

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

    void iterate(TreeMap &trees, const Game<Move> &state) const
    {
        thread_local auto func = [&]{ search(trees, state); };
        if (this->iterationCount() > 0) {
            static std::atomic_size_t counter;
            counter = this->iterationCount() * numThreads();
            executeFor(counter, func);
        } else {
            executeFor(this->iterationTime(), func);
        }
    }

    void search(TreeMap &trees, const Game<Move> &rootState) const
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
        const auto validMoves = state.validMoves();
        const auto player = state.currentPlayer();
        const auto &targetNode = nodes[player];
        if (!MOSolver::selectNode(targetNode, validMoves)) {
            const auto selection = this->selectChild(targetNode, state, validMoves);
            for (auto &node : nodes)
                node.second = MOSolver::findOrAddChild(node.second, state, selection->move());
            state.doMove(selection->move());
            select(nodes, state);
        }
    }

    static Node<Move> *findOrAddChild(Node<Move> *node, Game<Move> &state, const Move &move)
    {
        const auto &children = node->children();
        const auto pos = std::find_if(children.begin(), children.end(), [&](const auto &c){ return c->move() == move; });
        return pos < children.end() ? pos->get() : MOSolver::addChild(node, state, move);
    }

    void expand(NodePtrMap &nodes, Game<Move> &state) const
    {
        const auto player = state.currentPlayer();
        const auto untriedMoves = nodes[player]->untriedMoves(state.validMoves());
        if (!untriedMoves.empty()) {
            const auto move = this->randomMove(untriedMoves);
            for (auto &node : nodes)
                node.second = MOSolver::findOrAddChild(node.second, state, move);
            state.doMove(move);
        }
    }

    static void backPropagate(NodePtrMap &nodes, const Game<Move> &state)
    {
        for (auto node : nodes)
            SolverBase<Move,_ExecutionPolicy>::backPropagate(node.second, state);
    }

private:
    mutable TreeList m_trees;

    void setupTrees(const Game<Move> &rootState) const
    {
        const auto &state = dynamic_cast<const POMGame<Move>&>(rootState);
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
