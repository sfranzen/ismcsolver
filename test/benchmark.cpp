/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "catch.hpp"
#include <ismcts/sosolver.h>
#include <ismcts/mosolver.h>
#include "knockoutwhist.h"
#include "card.h"

#include <chrono>
#include <vector>
#include <iostream>

using namespace std::chrono;

namespace
{

const unsigned int numGames {100};

class GameRunner
{
public:
    GameRunner(ISMCTS::SolverBase<Card> &solver, unsigned int numGames)
        : solver{solver}, numGames{numGames}
    {}

    void run()
    {
        for (unsigned i = 0; i < numGames; ++i)
            score += playGame();
        report();
    }

private:
    ISMCTS::SolverBase<Card> &solver;
    unsigned int numGames {100};
    unsigned int score {0};
    unsigned int numCalls {0};
    duration<double> time {0};

    unsigned int playGame()
    {
        KnockoutWhist game {2};

        while (!game.validMoves().empty()) {
            Card move;
            if (game.currentPlayer() == 0) {
                const auto t0 = high_resolution_clock::now();
                move = solver(game);
                time += high_resolution_clock::now() - t0;
                ++numCalls;
            } else {
                move = game.validMoves().front();
            }
            game.doMove(move);
        }
        return game.getResult(0);
    }
    void report() const
    {
        using std::cout;
        const auto time_us = duration_cast<microseconds>(time).count();
        cout << std::string(79, '-') << "\nISMCTS player won: " << score << " out of " << numGames << " games,\n";
        cout << "selecting " << numCalls << " moves in " << time_us << " microseconds.\n";
        cout << "Average per search: " << double(time_us) / (numCalls) << " µs\n";
        cout << std::string(79, '-') << "\n";
    }
};

} // namespace

TEMPLATE_PRODUCT_TEST_CASE("Fixed iteration count", "[SOSolver]",
                           ISMCTS::SOSolver, (Card, (Card, ISMCTS::RootParallel)))
{
    TestType solver {100};
    GameRunner g(solver, numGames);
    REQUIRE_NOTHROW(g.run());
}

TEMPLATE_PRODUCT_TEST_CASE("Fixed iteration count", "[MOSolver]",
                           ISMCTS::MOSolver, (Card, (Card, ISMCTS::RootParallel)))
{
    TestType solver {2, 100};
    GameRunner g(solver, numGames);
    REQUIRE_NOTHROW(g.run());
}
