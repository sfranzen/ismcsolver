/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include <sosolver.h>
#include <mosolver.h>
#include "knockoutwhist.h"
#include "card.h"

#include <chrono>
#include <vector>
#include <random>
#include <iostream>

// Plays a game using an ISMCTS solver with numIters iterations for each move
// against an opponent that just picks random moves; returns the score for the
// ISMCTS player.
unsigned playGame(ISMCTS::SolverBase<Card> &solver, unsigned &callCounter, std::chrono::duration<double> &timeCounter)
{
    using namespace std;
    using namespace std::chrono;

    static mt19937 urng {random_device{}()};
    KnockoutWhist game {2};

    while (!game.validMoves().empty()) {
        const auto p = game.currentPlayer();
        Card move;
        if (p == 0) {
            const auto t0 = high_resolution_clock::now();
            move = solver(game);
            const auto t1 = high_resolution_clock::now();
            ++callCounter;
            timeCounter += t1 - t0;
        } else {
            const auto validMoves = game.validMoves();
            uniform_int_distribution<size_t> randIdx {0, validMoves.size() - 1};
            move = validMoves[randIdx(urng)];
        }
        game.doMove(move);
    }
    return game.getResult(0);
}

int main(int /*argc*/, char **/*argv*/)
{
    using namespace std;
    using namespace std::chrono;

    const unsigned numGames {10000};
    const unsigned numIters {100};
    ISMCTS::SOSolver<Card> solver {numIters};
    unsigned score {0};
    unsigned totalCalls {0};
    duration<double> t;

    for (unsigned i = 0; i < numGames; ++i)
        score += playGame(solver, totalCalls, t);

    cout << "ISMCTS player won: " << score << " out of " << numGames << " games\n";

    const auto time_us = duration_cast<microseconds>(t).count();
    cout << "Time taken: " << time_us << " microseconds\n";
    cout << "Per iteration: " << double(time_us) / (totalCalls * numIters) << endl;
    return 0;
}
