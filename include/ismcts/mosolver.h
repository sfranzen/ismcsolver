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

namespace ISMCTS
{

template<class Move, class _ExecutionPolicy = Sequential>
class MOSolver : public SolverBase<Move,_ExecutionPolicy>
{
public:
    using SolverBase<Move,_ExecutionPolicy>::SolverBase;
    using typename SolverBase<Move,_ExecutionPolicy>::RootPtr;

    // The search trees for the current observer, one per player
    using TreeMap = std::map<unsigned int, RootPtr>;

    // The set of tree maps, one for each thread
    using TreeList = std::vector<TreeMap>;

    Move operator()(POMGame<Move> const &rootState)
    {
        auto treeSearch = [this](TreeMap &map, Game<Move> const &state){ search(map, state); };
        auto treeGenerator = [&rootState]{ return newTree(rootState); };
        m_trees = MOSolver::execute(treeSearch, treeGenerator, rootState);

        std::vector<RootPtr> currentPlayerTrees(m_trees.size());
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
            auto const &move = this->selectChild(targetNode, state, validMoves)->move();
            auto makeChild = [&move, &state]{ return MOSolver::newChild(move, state); };
            for (auto &node : nodes)
                node.second = node.second->findOrAddChild(move, makeChild);
            state.doMove(move);
            select(nodes, state);
        }
    }

    void expand(NodePtrMap &nodes, Game<Move> &state) const
    {
        auto const player = state.currentPlayer();
        auto const untriedMoves = nodes[player]->untriedMoves(state.validMoves());
        if (!untriedMoves.empty()) {
            auto const move = randomElement(untriedMoves);
            auto makeChild = [&move, &state]{ return MOSolver::newChild(move, state); };
            for (auto &node : nodes)
                node.second = node.second->findOrAddChild(move, makeChild);
            state.doMove(move);
        }
    }

    void static backPropagate(NodePtrMap &nodes, Game<Move> const &state)
    {
        for (auto node : nodes)
            SolverBase<Move,_ExecutionPolicy>::backPropagate(node.second, state);
    }

private:
    TreeList m_trees;

    TreeMap static newTree(POMGame<Move> const &state)
    {
        TreeMap map;
        for (auto player : state.players())
            map.emplace(player, MOSolver::newRoot(state));
        return map;
    }
};

} // ISMCTS

#endif // ISMCTS_MOSOLVER_H
