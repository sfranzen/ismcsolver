/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include <sosolver.h>
#include <mosolver.h>
#include "game.h"
#include <chrono>
#include <iostream>

int main(int /*argc*/, char **/*argv*/)
{
    using namespace std;
    using namespace std::chrono;

    Game g {3};
    const unsigned iters {500};
    ISMCTS::MOSolver<int,ISMCTS::RootParallel> solver {2, iters};
    duration<double> t;
    unsigned iter_count {0};

    while (!g.validMoves().empty()) {
        const auto t0 = high_resolution_clock::now();
        const auto move = solver(g);
        const auto t1 = high_resolution_clock::now();
        g.doMove(move);
        t += t1 - t0;
        iter_count += iters;
    }

    const auto time_us = duration_cast<microseconds>(t).count();
    cout << g.getResult(0) << endl << g << endl;
    cout << "Time taken: " << time_us << " microseconds" << endl;
    cout << "Per iteration: " << double(time_us) / iter_count << endl;
    return 0;
}
