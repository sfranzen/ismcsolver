/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "knockoutwhist.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <iterator>
#include <exception>

namespace
{
const unsigned DeckSize {52};
}

KnockoutWhist::KnockoutWhist(unsigned players)
    : m_urng{5489ul}
    , m_tricksLeft{7}
    , m_numPlayers{std::min(std::max(players, 2u), 7u)}
    , m_player{0}
{
    m_players.resize(m_numPlayers);
    std::iota(m_players.begin(), m_players.end(), 0);
    m_playerCards.resize(m_numPlayers);
    m_tricksTaken.resize(m_numPlayers, 0);
    deal();
}

KnockoutWhist::Ptr KnockoutWhist::cloneAndRandomise(unsigned observer) const
{
    auto clone = new KnockoutWhist(*this);
    Hand unseenCards;
    for (auto p : m_players) {
        if (p == observer)
            continue;
        const auto &hand = m_playerCards[p];
        unseenCards.insert(unseenCards.end(), hand.begin(), hand.end());
    }
    std::shuffle(unseenCards.begin(), unseenCards.end(), m_urng);
    auto u = unseenCards.begin();
    for (auto p : m_players) {
        if (p == observer)
            continue;
        auto &hand = clone->m_playerCards[p];
        std::copy_n(u, hand.size(), hand.begin());
        u += hand.size();
    }
    return Ptr(std::move(clone));
}

void KnockoutWhist::doMove(const Card move)
{
    m_currentTrick.emplace_back(m_player, move);
    auto &hand = m_playerCards[m_player];
    const auto pos = std::find(hand.begin(), hand.end(), move);
    if (pos < hand.end())
        hand.erase(pos);
    else
        throw std::out_of_range("pos");
    m_player = nextPlayer();
    if (m_currentTrick.size() == m_players.size())
        finishTrick();
    if (m_playerCards[m_player].empty())
        finishRound();
}

void KnockoutWhist::finishTrick()
{
    const auto winner = trickWinner();
    ++m_tricksTaken[winner];
    m_currentTrick.clear();
    m_player = winner;
}

KnockoutWhist::Player KnockoutWhist::trickWinner() const
{
    auto winner = m_currentTrick.begin();
    for (auto p = winner + 1; p < m_currentTrick.end(); ++p) {
        const auto card = p->second;
        const auto winningCard = winner->second;
        if (card.suit == winningCard.suit) {
            if(card.rank > winningCard.rank)
                winner = p;
        } else if (card.suit == m_trumpSuit) {
            winner = p;
        }
    }
    return winner->first;
}

void KnockoutWhist::finishRound()
{
    m_players.erase(
        std::remove_if(m_players.begin(), m_players.end(), [&](Player p){ return m_tricksTaken[p] == 0; }),
        m_players.end()
    );
    for (auto p : m_players)
        m_tricksTaken[p] = 0;
    if (m_players.size() > 1)
        --m_tricksLeft;
    else
        m_tricksLeft = 0;
    deal();
}

KnockoutWhist::Player KnockoutWhist::nextPlayer() const
{
    auto next = ++std::find(m_players.begin(), m_players.end(), m_player);
    return next == m_players.end() ? m_players.front() : *next;
}

double KnockoutWhist::getResult(unsigned player) const
{
    const auto pos = std::find(m_players.begin(), m_players.end(), player);
    return pos < m_players.end() ? 1 : 0;
}

std::vector<Card> KnockoutWhist::validMoves() const
{
    const auto &hand = m_playerCards[m_player];

    if (m_currentTrick.empty() || hand.size() < 2)
        return hand;

    const auto leadCard = m_currentTrick.front().second;
    Hand cardsInSuit;
    std::copy_if(hand.begin(), hand.end(), std::back_inserter(cardsInSuit), [&](const Card &c){ return c.suit == leadCard.suit; });
    return cardsInSuit.empty() ? hand : cardsInSuit;
}

unsigned KnockoutWhist::currentPlayer() const
{
    return m_player;
}

// Deal cards by generating them from a random permutation of integers
void KnockoutWhist::deal()
{
    std::vector<int> indices(DeckSize);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), m_urng);
    auto i = indices.cend();
    for (auto p : m_players) {
        auto &hand = m_playerCards[p];
        std::generate_n(std::back_inserter(hand), m_tricksLeft, [&]() -> Card {
            --i;
            return {Card::Rank(*i % 13), Card::Suit(*i % 4)};
        });
    }
    // Choose random trump suit
    std::uniform_int_distribution<> randInt {0, 3};
    m_trumpSuit = Card::Suit(randInt(m_urng));
}

std::ostream& operator<<(std::ostream &out, const KnockoutWhist &g)
{
    const auto player = g.m_player;
    const auto &hand = g.m_playerCards[player];
    out << "Round " << g.m_tricksLeft << " | P" << player << ": ";
    std::copy(hand.begin(), hand.end(), std::ostream_iterator<Card>(out, ","));
    out << " | Tricks: " << g.m_tricksTaken[player];
    out << " | Trump: " << g.m_trumpSuit << " | Trick: [";
    for (const auto &pair : g.m_currentTrick)
        out << pair.first << ":" << pair.second << ",";
    return out << "]";
}
