/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_SOLVERBASE_H
#define ISMCTS_SOLVERBASE_H

#include "game.h"
#include "node.h"

#include <random>
#include <chrono>
#include <vector>

namespace ISMCTS
{
// Interface shared by all algorithms
template<class Move>
class SolverBase
{
public:
    /**
     * Constructs a solver that will iterate the given number of times per
     * search operation.
     *
     * @param exploration Sets the algorithm's bias towards unexplored moves.
     *      It must be positive; the authors of the algorithm suggest 0.7 for
     *      a game that reports result values on the interval [0,1].
     */
    explicit SolverBase(std::size_t iterationCount = 1000, double exploration = 0.7)
    {
        setIterationCount(iterationCount);
        setExplorationBias(exploration);
    }

    /**
     * Constructs a solver that will iterate for the given duration per search
     * operation.
     *
     * @param exploration Sets the algorithm's bias towards unexplored moves.
     *      It must be positive; the authors of the algorithm suggest 0.7 for
     *      a game that reports result values on the interval [0,1].
     */
    explicit SolverBase(std::chrono::duration<double> time, double exploration = 0.7)
    {
        setIterationTime(time);
        setExplorationBias(exploration);
    }

    virtual ~SolverBase() {}

    /**
     * Search operator.
     *
     * @return The most promising move from the given state.
     */
    virtual Move operator()(const Game<Move> &rootState) const = 0;

    /// Sets the solver to use a fixed number of iterations in future searches.
    void setIterationCount(std::size_t count)
    {
        m_iterCount = count;
        m_iterTime = std::chrono::duration<double>::zero();
    }

    /// Sets the solver to iterate for a fixed time in future searches.
    void setIterationTime(std::chrono::duration<double> time)
    {
        m_iterTime = time;
        m_iterCount = 0;
    }

    /// Sets the exploration bias of future searches.
    void setExplorationBias(double exploration)
    {
        m_exploration = std::max(exploration, 0.0);
    }

protected:
    std::size_t m_iterCount;
    std::chrono::duration<double> m_iterTime;
    double m_exploration;

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
        return moves[dist(urng())];
    }

    virtual std::mt19937 &urng() const
    {
        thread_local static std::mt19937 urng {std::random_device{}()};
        return urng;
    }
};

} // ISMCTS

#endif // ISMCTS_SOLVERBASE_H
