/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "catch.hpp"
#include "knockoutwhist.h"
#include <vector>
#include <string>
#include <exception>

namespace
{

// Game with two players having known cards
struct MockGame : public KnockoutWhist
{
    MockGame() : KnockoutWhist {2}
    {
        deal();
    }

    void deal()
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

void doValidMove(ISMCTS::Game<Card> &game)
{
    game.doMove(game.validMoves().front());
}

}

TEST_CASE("Game instantiates correctly", "[KnockoutWhist]")
{
    KnockoutWhist game {2};

    CHECK(game.currentPlayer() == 0);
    REQUIRE(game.validMoves().size() == 7);
}

TEST_CASE("Game handles moves correctly", "[KnockoutWhist]")
{
    MockGame game;

    SECTION("Valid move is accepted") {
        REQUIRE_NOTHROW(doValidMove(game));
        SECTION("Current player has advanced")
            REQUIRE(game.currentPlayer() == 1);
    }

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

TEST_CASE("Game terminates correctly", "[KnockoutWhist]")
{
    MockGame game;
    unsigned int turn {0};
    const unsigned int correctTurnCount {44};

    while (!game.validMoves().empty() && turn < correctTurnCount) {
        doValidMove(game);
        ++turn;
    }

    CHECK(turn == correctTurnCount);
    REQUIRE(game.validMoves().empty());
    CHECK(game.getResult(0) == 1);
    CHECK(game.getResult(1) == 0);
}
