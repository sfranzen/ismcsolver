/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "common/catch.hpp"
#include "common/knockoutwhist.h"
#include <vector>
#include <string>
#include <stdexcept>

namespace
{

const unsigned numPlayers = 2;

// Game with two players having known cards
struct MockGame : public KnockoutWhist
{
    MockGame() : KnockoutWhist {numPlayers}
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

inline void doValidMove(ISMCTS::Game<Card> &game)
{
    game.doMove(game.validMoves().front());
}

}

TEST_CASE("Game instantiates correctly", "[KnockoutWhist]")
{
    KnockoutWhist game {numPlayers};

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
