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

using namespace std::chrono_literals;
using namespace ISMCTS;
using Duration = ::ExecutionPolicy::Duration;

template<template<class,class,template<class>class...> class S>
struct Default
{
    template<class... Ts>
    struct Type : public S<Ts...>
    {
        using S<Ts...>::S;
    };
};

template<class... Ts>
using SODefault = Default<SOSolver>::Type<Ts...>;

template<class... Ts>
using MODefault = Default<MOSolver>::Type<Ts...>;

unsigned int constexpr numPlayers {2};
unsigned int constexpr iterationCount {10};
auto constexpr iterationTime {5ms};

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
    (SODefault, MODefault), (Card, (Card, RootParallel), (Card, TreeParallel)))
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
    (SODefault, MODefault), (Card, (Card, RootParallel), (Card, TreeParallel)))
{
    TestType solver {iterationCount};

    SECTION("Modifying iteration count") {
        auto const newIterationCount = 2 * iterationCount;
        solver.setIterationCount(newIterationCount);

        CHECK(solver.iterationTime() == Duration::zero());
        REQUIRE(solver.iterationCount() == newIterationCount);
    }
    SECTION("Modifying iteration time") {
        auto const newIterationTime = 2 * iterationTime;
        solver.setIterationTime(newIterationTime);

        CHECK(solver.iterationCount() == 0);
        REQUIRE(solver.iterationTime() == newIterationTime);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("Solvers' operator() returns a valid move", "[SOSolver][MOSolver]",
    (SODefault, MODefault), (Card, (Card, RootParallel), (Card, TreeParallel)))
{
    auto solver = GENERATE(std::make_shared<TestType>(iterationCount), std::make_shared<TestType>(iterationTime));
    Card move;

    auto testSection = [&](auto &game){
        SECTION(solver->iterationCount() != 0 ? "By iteration count" : "By iteration time") {
            auto const validMoves = game.validMoves();
            CHECK_NOTHROW([&]{ move = (*solver)(game); }());
            REQUIRE(std::find(validMoves.begin(), validMoves.end(), move) != validMoves.end());
        }
    };

    // Test sequential tree policy using Whist
    SECTION("Sequential tree policy") {
        KnockoutWhist game {numPlayers};
        testSection(game);
    }

    // Test simultaneous move policy using Goofspiel
    SECTION("Simultaneous tree policy") {
        Goofspiel game;

        // Advance to regular player before testing
        doValidMove(game);
        testSection(game);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("Solvers select the most rewarding final move", "[SOSolver][MOSolver]",
    (SODefault, MODefault), (int, (int, RootParallel), (int, TreeParallel)))
{
    P1DrawOrLose game;
    TestType solver {16};
    auto const move = solver(game);
    REQUIRE(move == 2);
}
