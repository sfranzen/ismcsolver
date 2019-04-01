/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_MOSOLVER_H
#define ISMCTS_MOSOLVER_H

#include "solverbase.h"
#include "game.h"
#include "node.h"
#include "execution.h"

#include <memory>
#include <vector>
#include <iostream>

namespace ISMCTS
{

template<class Move>
class MOSolverBase : public SolverBase<Move>
{
public:
    using SolverBase<Move>::SolverBase;

protected:
    /// Traverse a single sequence of moves
    void search(std::vector<Node<Move>> &trees, const Game<Move> &rootState) const
    {
        std::vector<Node<Move>*> roots(trees.size());
        std::transform(trees.begin(), trees.end(), roots.begin(), [](Node<Move> &n){ return &n; });
        auto randomState = rootState.cloneAndRandomise(rootState.currentPlayer());
        auto statePtr = randomState.get();
        select(roots, statePtr);
        expand(roots, statePtr);
        SolverBase<Move>::simulate(statePtr);
        backPropagate(roots, statePtr);
    }

    /**
     * Selection stage
     *
     * Descend all trees until a node is reached that has unexplored moves, or
     * is a terminal node (no more moves available).
     */
    void select(std::vector<Node<Move>*> &nodes, Game<Move> *state) const
    {
        bool selected = false;
        while (!selected) {
            const auto validMoves = state->validMoves();
            const auto player = state->currentPlayer();
            const auto &targetNode = nodes[player];
            selected = SolverBase<Move>::selectNode(targetNode, validMoves);
            if (!selected) {
                const auto selection = targetNode->ucbSelectChild(validMoves, this->m_exploration);
                for (auto &node : nodes)
                    node = node->findOrAddChild(selection->move(), player);
                state->doMove(selection->move());
            }
        }
    }

    /**
     * Expansion stage
     *
     * Choose a random unexplored move, add it to the children of all current
     * nodes and select these new nodes.
     */
    static void expand(std::vector<Node<Move>*> &nodes, Game<Move> *state)
    {
        const auto player = state->currentPlayer();
        const auto untriedMoves = nodes[player]->untriedMoves(state->validMoves());
        if (!untriedMoves.empty()) {
            const auto move = SolverBase<Move>::randMove(untriedMoves);
            for (auto &node : nodes)
                node = node->findOrAddChild(move, player);
            state->doMove(move);
        }
    }

    /**
     * Backpropagation stage
     *
     * Propagate game result back up to the root of each tree.
     */
    static void backPropagate(std::vector<Node<Move>*> &nodes, const Game<Move> *state)
    {
        for (auto node : nodes)
            while (node) {
                node->update(state);
                node = node->parent();
            }
    }
};

// Partial specialisations for each execution policy
template<class Move, class ExecutionPolicy = Sequential>
class MOSolver {};

/// Sequential multiple observer solver
template<class Move>
class MOSolver<Move, Sequential> : public MOSolverBase<Move>
{
public:
    using MOSolverBase<Move>::MOSolverBase;

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        std::vector<Node<Move>> trees(2);

        for (std::size_t i {0}; i < this->m_iterMax; ++i) {
            std::cout << "starting iteration " << i << "\n";
            this->search(trees, rootState);
            std::cout << "iteration completed; player trees: ";
            for (const auto &tree : trees)
                std::cout << tree;
            std::cout << std::endl;
        }

        const auto &rootList = trees.at(rootState.currentPlayer()).children();
        using value = typename Node<Move>::Ptr;
        const auto &mostVisited = *std::max_element(rootList.begin(), rootList.end(), [](const value &a, const value &b){
            return a->visits() < b->visits();
        });
        return mostVisited->move();
    }
};

}

#endif // ISMCTS_MOSOLVER_H
