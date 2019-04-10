/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "catch.hpp"
#include "mocksolvers.h"
#include "knockoutwhist.h"
#include <vector>

namespace
{

using namespace std::chrono;
using namespace ISMCTS;
using Duration = ::ExecutionPolicy::Duration;

const unsigned int numPlayers {2};
const unsigned int iterationCount {10};
const auto iterationTime {milliseconds(5)};

// Solver with default player number for ease of testing
template<class Move, class ExecutionPolicy = Sequential>
class DefaultMockMOSolver : public MockMOSolver<Move,ExecutionPolicy>
{
public:
    DefaultMockMOSolver(std::size_t iterationCount)
        : MockMOSolver<Move,ExecutionPolicy>{numPlayers, iterationCount}
    {}
    DefaultMockMOSolver(Duration iterationTime)
        : MockMOSolver<Move,ExecutionPolicy>{numPlayers, iterationTime}
    {}

};

}

TEMPLATE_PRODUCT_TEST_CASE("Solver instantiation", "[SOSolver][MOSolver]",
    (MockSOSolver, DefaultMockMOSolver), (Card, (Card, RootParallel)))
{
    SECTION("By iteration count") {
        TestType solver {iterationCount};
        CHECK(solver.iterationTime() == Duration::zero());
        REQUIRE(solver.iterationCount() == iterationCount / solver.numThreads());
    }

    SECTION("By iteration time") {
        TestType solver {iterationTime};
        CHECK(solver.iterationCount() == 0);
        REQUIRE(solver.iterationTime() == iterationTime);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("Modification of settings", "[SOSolver][MOSolver]",
    (MockSOSolver, DefaultMockMOSolver), (Card, (Card, RootParallel)))
{
    TestType solver {iterationCount};

    SECTION("Modifying iteration count") {
        const auto newIterationCount = 2 * iterationCount;
        solver.setIterationCount(newIterationCount);

        CHECK(solver.iterationTime() == Duration::zero());
        REQUIRE(solver.iterationCount() == newIterationCount / solver.numThreads());
    }
    SECTION("Modifying iteration time") {
        const auto newIterationTime = 2 * iterationTime;
        solver.setIterationTime(newIterationTime);

        CHECK(solver.iterationCount() == 0);
        REQUIRE(solver.iterationTime() == newIterationTime);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("Search execution", "[SOSolver][MOSolver]",
    (MockSOSolver, DefaultMockMOSolver), (Card, (Card, RootParallel)))
{
    KnockoutWhist game {numPlayers};
    TestType solver {iterationCount};
    const auto validMoves = game.validMoves();
    Card move;

    CHECK_NOTHROW([&](){ move = solver(game); }());
    REQUIRE(std::find(validMoves.begin(), validMoves.end(), move) < validMoves.end());
}
