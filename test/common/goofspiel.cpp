/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "goofspiel.h"
#include <ismcts/utility.h>

#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <random>

namespace {

inline int value(Card const &card)
{
    std::map<Card::Rank, int> static const values {
        {Card::Ace,     1},
        {Card::Two,     2},
        {Card::Three,   3},
        {Card::Four,    4},
        {Card::Five,    5},
        {Card::Six,     6},
        {Card::Seven,   7},
        {Card::Eight,   8},
        {Card::Nine,    9},
        {Card::Ten,     10},
        {Card::Jack,    11},
        {Card::Queen,   12},
        {Card::King,    13}
    };
    return values.at(card.rank);
}

inline auto makeHand(Card::Suit suit)
{
    int static constexpr handSize {13};
    std::vector<Card> hand;
    hand.reserve(handSize);
    for (int r {0}; r < handSize; ++r)
        hand.emplace_back(Card::Rank(r), suit);
    return hand;
}

} // namespace

Goofspiel::Goofspiel()
    : m_prizes{makeHand(Card::Hearts)}
    , m_hands{makeHand(Card::Spades), makeHand(Card::Clubs)}
{
    shufflePrizes();
}

void Goofspiel::shufflePrizes()
{
    std::mt19937 static thread_local prng {std::random_device{}()};
    std::shuffle(m_prizes.begin(), m_prizes.end(), prng);
}

Goofspiel::Ptr Goofspiel::cloneAndRandomise(Player observer) const
{
    auto clone = std::make_unique<Goofspiel>(*this);

    // The order of the remaining prizes is unknown to both players
    if (observer != 2)
        clone->shufflePrizes();

    // The solver always calls this function with observer == m_player and
    // player 0 always goes first, so the only other bit of hidden information
    // can be player 0's current move from player 1's point of view
    assert(observer == m_player);
    if (observer == 1)
        clone->m_moves[0] = ISMCTS::randomElement(clone->m_hands[0]);

    return clone;
}

bool Goofspiel::currentMoveSimultaneous() const
{
    return true;
}

Goofspiel::Player Goofspiel::currentPlayer() const
{
    return m_player;
}

void Goofspiel::doMove(Card const move)
{
    if (m_player != 2) {
        m_moves[m_player] = move;
        ++m_player;
    } else {
        handleP2Turn(move);
    }
}

void Goofspiel::handleP2Turn(Card const &move)
{
    if (m_drawPrize) {
        m_currentPrize = move;
        assert(move == m_prizes.back());
        m_prizes.pop_back();
        m_player = 0;
    } else {
        if (m_moves[0].rank != m_moves[1].rank) {
            auto &score = value(m_moves[0]) > value(m_moves[1]) ? m_scores[0] : m_scores[1];
            score += value(m_currentPrize);
        }
        for (Player p : {0, 1}) {
            auto &hand = m_hands[p];
            hand.erase(std::find(hand.begin(), hand.end(), m_moves[p]));
        }
    }
    m_drawPrize = !m_drawPrize;
}

double Goofspiel::getResult(Player player) const
{
    if (player == 2)
        // Return no score so that the environment player's nodes in the tree
        // will be selected uniformly at random by the EXP3 policy
        return 0;
    else if (m_scores[0] == m_scores[1])
        return 0.5;
    else
        return m_scores[player] > m_scores[1 - player] ? 1 : 0;
}

std::vector<Goofspiel::Player> Goofspiel::players() const
{
    std::vector<Player> static const players {0, 1, 2};
    return players;
}

std::vector<Card> Goofspiel::validMoves() const
{
    if (m_player != 2)
        return m_hands[m_player];
    else if (!m_drawPrize)
        // Return a constant dummy vector because the next turn is only used to
        // "reveal" the bids and award points; it should not lead to different
        // paths in the game tree
        return {Card{}};
    else if (!m_prizes.empty())
        return {m_prizes.back()};

    assert(m_prizes.empty());
    assert(m_hands[0].empty());
    assert(m_hands[1].empty());
    return {};
}
