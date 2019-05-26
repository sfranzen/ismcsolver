/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */

#include "goofspiel.h"
#include <ismcts/utility.h>

#include <map>
#include <algorithm>
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
    if (observer != 2)
        clone->shufflePrizes();
    if (observer == 1) {
        // Randomly select a new move for the first player
        auto &hand = clone->m_hands[0];
        hand.emplace_back(clone->m_moves[0]);
        clone->handleNormalTurn(0, ISMCTS::randomElement(hand));
    }
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
    if (m_player != 2)
        handleNormalTurn(m_player, move);
    else
        handleP2Turn(move);
    ++m_player %= 3;
}

void Goofspiel::handleNormalTurn(Player player, Card const &move)
{
    auto &hand = m_hands[player];
    hand.erase(std::find(hand.begin(), hand.end(), move));
    m_moves[player] = move;
}

void Goofspiel::handleP2Turn(Card const &move)
{
    if (m_drawPrize) {
        m_currentPrize = move;
        m_prizes.pop_back();
    } else if (m_moves[0].rank != m_moves[1].rank) {
        auto winner = std::max_element(m_moves.begin(), m_moves.end(), [](Card const &a, Card const &b){
            return value(a) < value(b);
        });
        auto &score = winner == m_moves.begin() ? m_scores[0] : m_scores[1];
        score += value(m_currentPrize);
    }
    m_drawPrize = !m_drawPrize;
}

double Goofspiel::getResult(Player player) const
{
    if (player == 2)
        return 1;
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
    if (m_drawPrize && !m_prizes.empty())
        return {m_prizes.back()};
    return {Card{}};
}
