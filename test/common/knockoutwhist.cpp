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
#include <stdexcept>
#include <random>
#include <numeric>

namespace
{

inline std::vector<Card> trumpChoices()
{
    std::vector<Card> cards(4);
    for (int i = 0; i < 4; ++i)
        cards[i] = {Card::Two, Card::Suit(i)};
    return cards;
}

inline std::mt19937 &prng()
{
    static thread_local std::mt19937 prng;
    return prng;
}

}

KnockoutWhist::KnockoutWhist(unsigned players)
    : m_tricksLeft{7}
    , m_numPlayers{std::min(std::max(players, 2u), 7u)}
    , m_dealer{m_numPlayers - 1}
{
    for (auto i = s_deckSize; i > 0; --i)
        m_deck[i] = {Card::Rank(i % 13), Card::Suit(i % 4)};
    m_players.resize(m_numPlayers);
    std::iota(m_players.begin(), m_players.end(), 0);
    m_playerCards.resize(m_numPlayers);
    m_tricksTaken.resize(m_numPlayers, 0);

    // Deal, then remove one of the remaining cards to choose the initial trump
    // suit
    deal();
    const auto choice = m_unknownCards.end() - 1;
    m_trumpSuit = choice->suit;
    m_unknownCards.erase(choice);
}

KnockoutWhist::Ptr KnockoutWhist::cloneAndRandomise(unsigned observer) const
{
    auto clone = new KnockoutWhist(*this);
    Hand unseenCards = m_unknownCards;
    for (auto p : m_players) {
        if (p == observer)
            continue;
        const auto &hand = m_playerCards[p];
        unseenCards.insert(unseenCards.end(), hand.begin(), hand.end());
    }
    std::shuffle(unseenCards.begin(), unseenCards.end(), prng());
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

KnockoutWhist::Player KnockoutWhist::currentPlayer() const
{
    return m_player;
}

KnockoutWhist::Player KnockoutWhist::nextPlayer(Player p) const
{
    ++p %= m_numPlayers;
    const auto next = std::find(m_players.begin(), m_players.end(), p);
    return next < m_players.end() ? *next : nextPlayer(p);
}

void KnockoutWhist::doMove(const Card move)
{
    if (m_requestTrump) {
        m_trumpSuit = move.suit;
        m_requestTrump = false;
        m_player = nextPlayer(m_dealer);
        return;
    }
    m_currentTrick.emplace_back(m_player, move);
    auto &hand = m_playerCards[m_player];
    const auto pos = std::find(hand.begin(), hand.end(), move);
    if (pos < hand.end())
        hand.erase(pos);
    else
        throw std::out_of_range("pos");
    m_player = nextPlayer(m_player);
    if (m_currentTrick.size() == m_players.size())
        finishTrick();
    if (m_playerCards[m_player].empty())
        finishRound();
}

double KnockoutWhist::getResult(unsigned player) const
{
    return player == m_players.front() ? 1 : 0;
}

std::vector<Card> KnockoutWhist::validMoves() const
{
    // Allow first player to bid on rounds other than the first
    if (m_requestTrump)
        return trumpChoices();

    const auto &hand = m_playerCards[m_player];
    if (m_currentTrick.empty() || hand.size() < 2)
        return hand;

    const auto leadCard = m_currentTrick.front().second;
    Hand cardsInSuit;
    std::copy_if(hand.begin(), hand.end(), std::back_inserter(cardsInSuit), [&](const Card &c){ return c.suit == leadCard.suit; });
    return cardsInSuit.empty() ? hand : cardsInSuit;
}

void KnockoutWhist::deal()
{
    std::shuffle(m_deck.begin(), m_deck.end(), prng());

    auto pos = m_deck.begin();
    for (auto p : m_players) {
        auto &hand = m_playerCards[p];
        std::copy_n(pos, m_tricksLeft, std::back_inserter(hand));
        pos += m_tricksLeft;
    }

    // The rest of the cards are unknown to all players
    m_unknownCards.clear();
    std::copy(pos, m_deck.end(), std::back_inserter(m_unknownCards));
}


void KnockoutWhist::finishTrick()
{
    const auto winner = trickWinner();
    ++m_tricksTaken[winner];
    m_currentTrick.clear();
    m_player = winner;
}

void KnockoutWhist::finishRound()
{
    m_players.erase(
        std::remove_if(m_players.begin(), m_players.end(), [&](Player p){ return m_tricksTaken[p] == 0; }),
        m_players.end()
    );
    if (m_players.size() > 1) {
        --m_tricksLeft;
        m_requestTrump = true;
        m_player = roundWinner();
        m_dealer = nextPlayer(m_dealer);
        for (auto p : m_players)
            m_tricksTaken[p] = 0;
    } else {
        m_tricksLeft = 0;
    }
    deal();
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

KnockoutWhist::Player KnockoutWhist::roundWinner() const
{
    auto players = m_players;
    std::sort(players.begin(), players.end(), [&](Player p1, Player p2){
        return m_tricksTaken[p1] > m_tricksTaken[p2];
    });
    const auto maxTricksTaken = m_tricksTaken[players[0]];
    const unsigned playersTied = 1 + std::count_if(players.begin() + 1, players.end(), [&](Player p){
        return m_tricksTaken[p] == maxTricksTaken;
    });
    std::uniform_int_distribution<unsigned> randPlayer {0, playersTied - 1};
    return players[randPlayer(prng())];
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
