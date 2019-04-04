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
#include <iostream>

int main(int /*argc*/, char **/*argv*/)
{
    using namespace std;
    using namespace std::chrono;

    KnockoutWhist game {3};
    const unsigned iters {500};
    ISMCTS::SOSolver<Card> solver {iters};
    duration<double> t;
    unsigned iter_count {0};

    while (!game.validMoves().empty()) {
        std::cout << game << "\n";
        const auto t0 = high_resolution_clock::now();
        const auto move = solver(game);
        const auto t1 = high_resolution_clock::now();
        game.doMove(move);
        t += t1 - t0;
        iter_count += iters;
    }

    const auto time_us = duration_cast<microseconds>(t).count();
    cout << "Time taken: " << time_us << " microseconds" << endl;
    cout << "Per iteration: " << double(time_us) / iter_count << endl;
    return 0;
}
