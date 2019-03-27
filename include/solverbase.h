/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_SOLVERBASE_H
#define ISMCTS_SOLVERBASE_H

#include "game.h"
#include <random>

namespace ISMCTS
{
// Interface shared by all algorithms
template<class Move>
class SolverBase
{
public:
    /**
     * Constructor.
     *
     * @param iterMax Sets the number of simulations performed for each search.
     * @param exploration Sets the algorithm's bias towards unexplored moves.
     *      It must be positive; the authors of the algorithm suggest 0.7.
     */
    explicit SolverBase(std::size_t iterMax = 1000, double exploration = 0.7)
        : m_iterMax {iterMax}
        , m_exploration {std::max(exploration, 0.0)}
    {}

    virtual ~SolverBase() {}

    /**
     * Search operator.
     *
     * @return The most promising move from the given state.
     */
    virtual Move operator()(const Game<Move> &rootState) const = 0;

protected:
    const std::size_t m_iterMax;
    const double m_exploration;

    static const Move &randMove(const std::vector<Move> &moves)
    {
        thread_local static std::random_device rd;
        thread_local static std::mt19937 rng {rd()};
        std::uniform_int_distribution<std::size_t> dist {0, moves.size() - 1};
        return moves[dist(rng)];
    }
};

} // ISMCTS

#endif // ISMCTS_SOLVERBASE_H
