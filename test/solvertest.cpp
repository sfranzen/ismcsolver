/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */

#include <ismcts/sosolver.h>
#include <ismcts/mosolver.h>
#include "common/catch.hpp"
#include "common/knockoutwhist.h"
#include "common/mnkgame.h"
#include "common/goofspiel.h"
#include "common/utility.h"

#include <vector>
#include <memory>

namespace
{

using namespace std::chrono;
using namespace ISMCTS;
using Duration = ::ExecutionPolicy::Duration;

const unsigned int numPlayers {2};
const unsigned int iterationCount {10};
const auto iterationTime {milliseconds(5)};

// In this state, player 1 has to choose between move 2, ending in a draw, and
// move 0, ending in a loss
struct P1DrawOrLose : public MnkGame
{
    P1DrawOrLose()
        : MnkGame{}
    {
        m_board = {
            {-1, 1,-1},
            {1, 0, 0},
            {0, 0, 1}
        };
        m_moves = {0, 2};
        m_player = 1;
    }
};

}

TEMPLATE_PRODUCT_TEST_CASE("Solvers construct properly", "[SOSolver][MOSolver]",
    (SOSolver, MOSolver), (Card, (Card, RootParallel)))
{
    SECTION("By iteration count") {
        TestType solver {iterationCount};
        CHECK(solver.iterationTime() == Duration::zero());
        REQUIRE(solver.iterationCount() == iterationCount);
    }

    SECTION("By iteration time") {
        TestType solver {iterationTime};
        CHECK(solver.iterationCount() == 0);
        REQUIRE(solver.iterationTime() == iterationTime);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("Solver settings can be modified", "[SOSolver][MOSolver]",
    (SOSolver, MOSolver), (Card, (Card, RootParallel)))
{
    TestType solver {iterationCount};

    SECTION("Modifying iteration count") {
        const auto newIterationCount = 2 * iterationCount;
        solver.setIterationCount(newIterationCount);

        CHECK(solver.iterationTime() == Duration::zero());
        REQUIRE(solver.iterationCount() == newIterationCount);
    }
    SECTION("Modifying iteration time") {
        const auto newIterationTime = 2 * iterationTime;
        solver.setIterationTime(newIterationTime);

        CHECK(solver.iterationCount() == 0);
        REQUIRE(solver.iterationTime() == newIterationTime);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("Solvers' operator() returns a valid move", "[SOSolver][MOSolver]",
    (SOSolver, MOSolver), (Card, (Card, RootParallel)))
{
    auto solver = GENERATE(std::make_shared<TestType>(iterationCount), std::make_shared<TestType>(iterationTime));
    Card move;

    auto testSection = [&](auto &game){
        SECTION(solver->iterationCount() != 0 ? "By iteration count" : "By iteration time") {
            const auto validMoves = game.validMoves();
            CHECK_NOTHROW([&]{ move = (*solver)(game); }());
            REQUIRE(std::find(validMoves.begin(), validMoves.end(), move) != validMoves.end());
        }
    };

    // Test UCB1 policy using Whist, which has sequential moves
    SECTION("UCB1 policy") {
        KnockoutWhist game {numPlayers};
        testSection(game);
    }

    // Test EXP3 policy using Goofspiel, which has simultaneous moves
    SECTION("EXP3 policy") {
        Goofspiel game;

        // Advance to regular player before testing
        doValidMove(game);
        testSection(game);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("Solvers select the most rewarding final move", "[SOSolver][MOSolver]",
    (SOSolver, MOSolver), (int, (int, RootParallel)))
{
    P1DrawOrLose game;
    TestType solver {16};
    const auto move = solver(game);
    REQUIRE(move == 2);
}
