/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "common/catch.hpp"
#include "common/mnkgame.h"
#include "common/phantommnkgame.h"
#include "common/knockoutwhist.h"
#include "common/goofspiel.h"
#include <vector>
#include <string>
#include <stdexcept>

namespace
{

const unsigned numPlayers = 2;

// Game with two players having known cards
struct MockWhist : public KnockoutWhist
{
    MockWhist() : KnockoutWhist {numPlayers}
    {
        initialDeal();
    }

    void initialDeal()
    {
        m_trumpSuit = Card::Diamonds;
        m_playerCards[0] = {
            {Card::Eight, Card::Diamonds},
            {Card::Ten, Card::Spades},
            {Card::Five, Card::Hearts},
            {Card::Nine, Card::Hearts},
            {Card::Two, Card::Hearts},
            {Card::Jack, Card::Clubs},
            {Card::Queen, Card::Diamonds}
        };
        m_playerCards[1] = {
            {Card::Ten, Card::Clubs},
            {Card::Seven, Card::Clubs},
            {Card::Jack, Card::Spades},
            {Card::Three, Card::Diamonds},
            {Card::King, Card::Clubs},
            {Card::Ace, Card::Diamonds},
            {Card::Seven, Card::Hearts}
        };
    }
};

// Mock phantom game to test cloning
struct MockPhantomGame : public PhantomMnkGame
{
    using PhantomMnkGame::PhantomMnkGame;

    std::vector<std::vector<int>> &board()
    {
        return m_board;
    }

    std::vector<int> &available(unsigned player)
    {
        return m_available[player];
    }

    std::vector<int> &moves()
    {
        return m_moves;
    }
};

template<class Move>
inline void doValidMove(ISMCTS::Game<Move> &game)
{
    game.doMove(game.validMoves().front());
}

// Sequences that should give player 0 a win, one for each row/column/diagonal
const std::vector<std::vector<int>> MNKP0WinSequences {
    {0,3,1,4,2},
    {3,0,4,1,5},
    {6,0,7,1,8},
    {0,2,3,4,6},
    {1,0,4,2,7},
    {2,0,5,1,8},
    {0,1,4,2,8},
    {2,0,4,1,6}
};

const std::vector<unsigned> expectedPlayers2P {0, 1};
const std::vector<unsigned> expectedPlayers3P {0, 1, 2};

}

TEST_CASE("Knockout Whist instantiates correctly", "[KnockoutWhist]")
{
    KnockoutWhist game {numPlayers};
    CHECK(game.currentPlayer() == 0);
    CHECK(game.players() == expectedPlayers2P);
    REQUIRE(game.validMoves().size() == 7);
}

TEMPLATE_TEST_CASE("M-n-k games instantiate correctly", "[MnkGame][PhantomMnkGame]", MnkGame, PhantomMnkGame)
{
    TestType game;
    CHECK(game.currentPlayer() == 0);
    CHECK(game.players() == expectedPlayers2P);
    REQUIRE(game.validMoves().size() == 9);
}

TEST_CASE("Goofspiel constructor", "[Goofspiel]")
{
    Goofspiel game;
    CHECK(game.currentPlayer() == 2);
    CHECK(game.players() == expectedPlayers3P);
    CHECK(game.validMoves().size() == 1);
}

TEMPLATE_TEST_CASE("Games handle valid moves correctly", "[KnockoutWhist][MnkGame][PhantomMnkGame]",
                   KnockoutWhist, MnkGame, PhantomMnkGame)
{
    TestType game;
    SECTION("Valid move is accepted") {
        REQUIRE_NOTHROW(doValidMove(game));
        SECTION("Current player has advanced")
            REQUIRE(game.currentPlayer() == 1);
    }
}

TEST_CASE("Knockout Whist functions correctly", "[KnockoutWhist]")
{
    MockWhist game;
    // Note: invalid means any card not held by the player as the game does not
    // check for failures to follow suit
    SECTION("Invalid move causes out of range exception") {
        const Card invalidMove {Card::Seven, Card::Diamonds};
        REQUIRE_THROWS_AS(game.doMove(invalidMove), std::out_of_range);
    }

    SECTION("Tricks are evaluated correctly") {
        SECTION("Player 1 follows suit") {
            CHECK_NOTHROW(game.doMove({Card::Eight, Card::Diamonds}));
            SECTION("Player 1 does not beat card led") {
                CHECK_NOTHROW(game.doMove({Card::Three, Card::Diamonds}));
                REQUIRE(game.currentPlayer() == 0);
            }
            SECTION("Player 1 beats card led") {
                CHECK_NOTHROW(game.doMove({Card::Ace, Card::Diamonds}));
                REQUIRE(game.currentPlayer() == 1);
            }
        }
        SECTION("Player 1 does not follow suit") {
            SECTION("Neither suit played is trumps") {
                CHECK_NOTHROW(game.doMove({Card::Ten, Card::Spades}));
                CHECK_NOTHROW(game.doMove({Card::Ten, Card::Clubs}));
                REQUIRE(game.currentPlayer() == 0);
            }
            SECTION("One of the suits is trumps") {
                SECTION("Player 0 trumped") {
                    CHECK_NOTHROW(game.doMove({Card::Eight, Card::Diamonds}));
                    CHECK_NOTHROW(game.doMove({Card::Ten, Card::Clubs}));
                    REQUIRE(game.currentPlayer() == 0);
                }
                SECTION("Player 1 trumped") {
                    CHECK_NOTHROW(game.doMove({Card::Ten, Card::Spades}));
                    CHECK_NOTHROW(game.doMove({Card::Three, Card::Diamonds}));
                    REQUIRE(game.currentPlayer() == 1);
                }
            }
        }
    }
}

TEMPLATE_TEST_CASE("M-n-k games function correctly", "[MnkGame][PhantomMnkGame]", MnkGame, PhantomMnkGame)
{
    TestType game;
    SECTION("Invalid move causes out of range exception") {
        REQUIRE_THROWS_AS(game.doMove(9), std::out_of_range);
    }
    SECTION("Wins are detected properly") {
        int seqNum = 0;
        for (auto &sequence : MNKP0WinSequences) {
            DYNAMIC_SECTION("Checking test sequence " << ++seqNum) {
                game = TestType{};
                for (auto move : sequence)
                    game.doMove(move);
                CHECK(game.validMoves().empty());
                REQUIRE(game.getResult(0) == 1);
            }
        }
    }
}

TEST_CASE("Goofspiel functions correctly", "[Goofspiel]")
{
    Goofspiel game;

    // Set up initial prize
    doValidMove(game);

    SECTION("Players and hands") {
        for (unsigned player : {0, 1}) {
            DYNAMIC_SECTION("Player " << player) {
                CHECK(game.currentPlayer() == player);
                CHECK(game.validMoves().size() == 13);
            }
            doValidMove(game);
        }
        REQUIRE(game.currentPlayer() == 2);
    }

    SECTION("Equal bids give equal scores") {
        for (int i {0}; i < 3; ++i)
            doValidMove(game);
        CHECK(game.getResult(0) == 0.5);
        REQUIRE(game.getResult(1) == 0.5);
    }

    SECTION("Unequal bids give unequal scores") {
        for (int i : {0, 1})
            game.doMove(game.validMoves()[i]);
        doValidMove(game);
        CHECK(game.getResult(0) == 0);
        REQUIRE(game.getResult(1) == 1);
    }
}

// Test game with a sequence of moves that leads to the board state
//  1 -1  1
// -1  0 -1
//  0 -1 -1,
// with player 0 not knowing any of player 1's positions
TEST_CASE("Phantom m-n-k game clones correctly", "[PhantomMnkGame]")
{
    MockPhantomGame game;
    const std::vector<int> sequence {4,4,0,6,2};

    for (auto move : sequence)
        game.doMove(move);
    auto clone = game.cloneAndRandomise(0);
    auto &rClone = static_cast<MockPhantomGame&>(*clone);

    CHECK(game.available(0) == rClone.available(0));
    CHECK(game.available(1).size() == rClone.available(1).size());
    CHECK(game.moves().size() == rClone.moves().size());
    REQUIRE(rClone.board()[1][1] == 0);
    REQUIRE(rClone.board()[2][0] == 0);
}

TEST_CASE("Knockout Whist terminates correctly", "[KnockoutWhist]")
{
    KnockoutWhist game {numPlayers};
    // Maximum number of turns is nPlayers * (7 + 6 + ... + 1) + 6 (selection turns)
    const unsigned int maxTurns {6 + numPlayers * 28};
    unsigned int turn {0};

    while (!game.validMoves().empty() && turn < maxTurns) {
        doValidMove(game);
        ++turn;
    }
    INFO("Number of turns: " << turn);
    REQUIRE(game.validMoves().empty());
}
