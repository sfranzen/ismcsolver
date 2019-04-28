/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_SOLVERBASE_H
#define ISMCTS_SOLVERBASE_H

#include "game.h"
#include "node.h"
#include "ucb1.h"

#include <random>
#include <vector>

namespace ISMCTS
{
/// Interface shared by all algorithms
template<class Move, class _TreePolicy = UCB1<Move>>
class SolverBase
{
public:
    /**
    * @param policy The tree policy to use in the selection stage of the search.
    */
    explicit SolverBase(const _TreePolicy &policy)
        : m_treePolicy{policy}
    {}

    virtual ~SolverBase() = default;

    /**
     * Search operator.
     *
     * @return The most promising move from the given state.
     */
    virtual Move operator()(const Game<Move> &rootState) const = 0;

    /// Set the tree policy for future searches.
    void setTreePolicy(const _TreePolicy &policy)
    {
        m_treePolicy = policy;
    }

protected:
    _TreePolicy m_treePolicy;

    /**
     * Simulation stage
     *
     * Continue performing random available moves from this state until the end
     * of the game.
     */
    void simulate(Game<Move> &state) const
    {
        const auto moves = state.validMoves();
        if (!moves.empty()) {
            state.doMove(this->randomMove(moves));
            simulate(state);
        }
    }

    /**
     * Backpropagation stage
     *
     * Traverse back to the root from the given node, updating the statistics of
     * all visited nodes with the result of the finished game state.
     */
    static void backPropagate(Node<Move> *node, const Game<Move> &state)
    {
        while (node) {
            node->update(state);
            node = node->parent();
        }
    }

    /// Whether to select this node given available moves
    static bool selectNode(const Node<Move> *node, const std::vector<Move> &moves)
    {
        return moves.empty() || !node->untriedMoves(moves).empty();
    }

    const Move &randomMove(const std::vector<Move> &moves) const
    {
        std::uniform_int_distribution<std::size_t> dist {0, moves.size() - 1};
        return moves[dist(prng())];
    }

    virtual std::mt19937 &prng() const
    {
        thread_local static std::mt19937 prng {std::random_device{}()};
        return prng;
    }
};

} // ISMCTS

#endif // ISMCTS_SOLVERBASE_H
