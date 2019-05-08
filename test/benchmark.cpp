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
#include "common/phantommnkgame.h"

#include <chrono>
#include <vector>
#include <array>
#include <map>
#include <iostream>
#include <iomanip>
#include <random>
#include <string>
#include <cmath>

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

    template<class Game, class G1, class G2>
    void run(Game &&game, G1 &&generator1, G2 &&generator2)
    {
        m_p0Scores.resize(m_numGames);
        m_numCalls.fill(0);
        m_times.fill(Duration::zero());
        std::generate(m_p0Scores.begin(), m_p0Scores.end(), [&]{
            return playGame(std::forward<Game>(game), std::forward<G1>(generator1), std::forward<G2>(generator2));
        });
        report();
    };

private:
    using Clock = std::chrono::high_resolution_clock;
    using Duration = ExecutionPolicy::Duration;

    unsigned m_numGames;
    std::vector<double> m_p0Scores;
    std::array<unsigned, 2> m_numCalls;
    std::array<Duration, 2> m_times;

    template<class Game, class G1, class G2>
    double playGame(Game &&game, G1 &&generator1, G2 &&generator2)
    {
        auto newGame = game;

        while (!newGame.validMoves().empty()) {
            const auto player = newGame.currentPlayer();
            const auto t0 = Clock::now();
            auto move = player == 0 ? generator1(newGame) : generator2(newGame);
            m_times[player] += Clock::now() - t0;
            ++m_numCalls[player];
            newGame.doMove(move);
        }
        return newGame.getResult(0);
    }

    void report() const
    {
        using namespace std;
        using namespace std::chrono;
        static const auto separator = std::string(79, '-') + "\n";

        std::map<double,unsigned> counts;
        for (auto score : m_p0Scores) {
            auto ret = counts.emplace(score, 1);
            if (!ret.second)
                ++ret.first->second;
        }

        auto flags = cout.flags();
        auto precision = cout.precision();
        auto countWidth = std::floor(1 + std::log10(m_numGames));

        cout << separator << "First player score stats after " << m_numGames << " games:\n";
        for (auto &pair : counts)
            cout << setw(3) << setprecision(2) << pair.first << ": " << setw(countWidth) << pair.second
                << " times (" << setprecision(3) << pair.second * 100. / m_numGames << "%)\n";

        cout << setiosflags(flags) << setprecision(precision);
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
