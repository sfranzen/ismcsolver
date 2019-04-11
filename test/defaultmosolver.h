/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef DEFAULTMOSOLVER_H
#define DEFAULTMOSOLVER_H

#include <ismcts/mosolver.h>
#include <ismcts/execution.h>

// MO solvers for two players for ease of testing
template<class Move, class ExecutionPolicy = ISMCTS::Sequential>
struct DefaultMOSolver : public ISMCTS::MOSolver<Move,ExecutionPolicy>
{
    using ISMCTS::MOSolver<Move,ExecutionPolicy>::MOSolver;

    DefaultMOSolver(std::size_t iterationCount = 1000)
        : ISMCTS::MOSolver<Move,ExecutionPolicy>{2, iterationCount}
    {}

    DefaultMOSolver(ISMCTS::ExecutionPolicy::Duration iterationTime)
        : ISMCTS::MOSolver<Move,ExecutionPolicy>{2, iterationTime}
    {}
};

#endif // DEFAULTMOSOLVER_H
