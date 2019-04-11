/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef KNOCKOUTWHIST_H
#define KNOCKOUTWHIST_H

#include <ismcts/game.h>
#include "card.h"

#include <vector>
#include <random>

class KnockoutWhist : public ISMCTS::Game<Card>
{
public:
    KnockoutWhist(unsigned players = 4);
    virtual Ptr cloneAndRandomise(unsigned observer) const override;
    virtual unsigned currentPlayer() const override;
    virtual std::vector<Card> validMoves() const override;
    virtual void doMove(const Card move) override;
    virtual double getResult(unsigned player) const override;
    friend std::ostream &operator<<(std::ostream &out, const KnockoutWhist &g);

protected:
    using Player = unsigned;
    using Hand = std::vector<Card>;
    using Play = std::pair<Player,Card>;

    mutable std::mt19937 m_urng;
    std::vector<Player> m_players;
    std::vector<Hand> m_playerCards;
    std::vector<Play> m_currentTrick;
    std::vector<unsigned> m_tricksTaken;
    unsigned m_tricksLeft;
    unsigned m_numPlayers;
    Player m_player;
    Card::Suit m_trumpSuit;

    void deal();
    void finishTrick();
    void finishRound();
    Player nextPlayer() const;
    Player trickWinner() const;
};

#endif // KNOCKOUTWHIST_H
