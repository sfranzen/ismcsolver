/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include <ismcts/sosolver.h>
#include <ismcts/mosolver.h>
#include "common/catch.hpp"
#include "common/knockoutwhist.h"
#include "common/card.h"

#include <chrono>
#include <vector>
#include <array>
#include <iostream>
#include <random>
#include <string>

namespace
{

using namespace ISMCTS;
const unsigned int numGames {10};
const unsigned int iterationCount {2000};

// Test one "move generator" against another in a given game
class SolverTester
{
public:
    explicit SolverTester(unsigned numGames = 100)
        : m_numGames{numGames}
    {}

    template<class Game, class Generator1, class Generator2>
    void run(Game &&game, Generator1 &&g1, Generator2 &&g2)
    {
        for (unsigned i = 0; i < m_numGames; ++i)
            m_score += playGame(game, g1, g2);
        report();
    };

private:
    using Clock = std::chrono::high_resolution_clock;
    using Duration = ExecutionPolicy::Duration;

    unsigned m_numGames;
    double m_score {0};
    std::array<unsigned, 2> m_numCalls {{0, 0}};
    std::array<Duration, 2> m_times;

    template<class Game, class Generator1, class Generator2>
    double playGame(Game &&game, Generator1 &&g1, Generator2 &&g2)
    {
        auto newGame = game;

        while (!newGame.validMoves().empty()) {
            const auto player = newGame.currentPlayer();
            const auto t0 = Clock::now();
            auto move = player == 0 ? g1(newGame) : g2(newGame);
            m_times[player] += Clock::now() - t0;
            ++m_numCalls[player];
            newGame.doMove(move);
        }
        return newGame.getResult(0);
    }

    void report() const
    {
        using namespace std::chrono;
        using std::cout;
        static const auto separator = std::string(79, '-') + "\n";
        cout << separator << "First player scored " << m_score << " points in " << m_numGames << " games.\n";
        for (unsigned p : {0, 1}) {
            const auto time_us = duration_cast<microseconds>(m_times[p]).count();
            cout << "Player " << p << " selected " << m_numCalls[p] << " moves in " << time_us
                 << " µs, average " << double(time_us) / m_numCalls[p] << " µs per move.\n";
        }
        cout << separator;
    }
};

template<class Move>
Move randomMove(const Game<Move> &game)
{
    static thread_local std::mt19937 rng {std::random_device{}()};
    const auto moves = game.validMoves();
    std::uniform_int_distribution<std::size_t> randomMove {0, moves.size() - 1};
    return moves[randomMove(rng)];
}

} // namespace

TEMPLATE_PRODUCT_TEST_CASE("Solver versus random player", "[SOSolver][MOSolver]",
                           (SOSolver, MOSolver), (Card, (Card, RootParallel)))
{
    TestType solver {iterationCount};
    SolverTester tester {numGames};
    REQUIRE_NOTHROW(tester.run(KnockoutWhist{2}, solver, randomMove<Card>));
}
