/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "catch.hpp"
#include <ismcts/sosolver.h>
#include "defaultmosolver.h"
#include "knockoutwhist.h"
#include "card.h"

#include <chrono>
#include <vector>
#include <iostream>
namespace
{

using namespace std::chrono;
using namespace ISMCTS;
const unsigned int numGames {10};
const unsigned int iterationCount {2000};

class GameRunner
{
public:
    GameRunner(SolverBase<Card> &solver, unsigned int numGames)
        : solver{solver}, numGames{numGames}
    {}

    void run()
    {
        for (unsigned i = 0; i < numGames; ++i)
            score += playGame();
        report();
    }

private:
    SolverBase<Card> &solver;
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

TEMPLATE_PRODUCT_TEST_CASE("Fixed iteration count", "[SOSolver][MOSolver]",
                           (SOSolver, DefaultMOSolver), (Card, (Card, RootParallel)))
{
    TestType solver {iterationCount};
    GameRunner g(solver, numGames);
    REQUIRE_NOTHROW(g.run());
}
