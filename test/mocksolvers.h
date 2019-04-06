/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef MOCKSOLVERS_H
#define MOCKSOLVERS_H

#include <ismcts/sosolver.h>
#include <ismcts/mosolver.h>
#include <ismcts/execution.h>
#include <random>

inline std::mt19937 &mockrng()
{
    thread_local static std::mt19937 urng;
    return urng;
}

template<class Move, class ExecutionPolicy = ISMCTS::Sequential>
class MockSOSolver : public ISMCTS::SOSolver<Move,ExecutionPolicy>
{
public:
    using ISMCTS::SOSolver<Move,ExecutionPolicy>::SOSolver;

protected:
    std::mt19937 &urng() const override
    {
        return mockrng();
    }
};

template<class Move, class ExecutionPolicy = ISMCTS::Sequential>
class MockMOSolver : public ISMCTS::MOSolver<Move,ExecutionPolicy>
{
public:
    using ISMCTS::MOSolver<Move,ExecutionPolicy>::MOSolver;

protected:
    std::mt19937 &urng() const override
    {
        return mockrng();
    }
};

#endif //MOCKSOLVERS_H
