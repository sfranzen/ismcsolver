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

class KnockoutWhist : public ISMCTS::POMGame<Card>
{
public:
    using Player = unsigned;
    KnockoutWhist(unsigned players = 4);
    virtual Ptr cloneAndRandomise(unsigned observer) const override;
    virtual Player currentPlayer() const override;
    virtual Player nextPlayer(Player p) const override;
    virtual std::vector<Card> validMoves() const override;
    virtual void doMove(const Card move) override;
    virtual double getResult(unsigned player) const override;
    friend std::ostream &operator<<(std::ostream &out, const KnockoutWhist &g);

protected:
    using Hand = std::vector<Card>;
    using Play = std::pair<Player,Card>;

    const static unsigned int s_deckSize {52};
    std::vector<Card> m_deck {s_deckSize};
    std::vector<Card> m_unknownCards;
    std::vector<Player> m_players;
    std::vector<Hand> m_playerCards;
    std::vector<Play> m_currentTrick;
    std::vector<unsigned> m_tricksTaken;
    unsigned m_tricksLeft {7};
    unsigned m_numPlayers;
    Player m_player {0};
    Player m_dealer;
    Card::Suit m_trumpSuit;
    bool m_requestTrump = false;

    void deal();
    void finishTrick();
    void finishRound();
    Player trickWinner() const;
    Player roundWinner() const;
};

#endif // KNOCKOUTWHIST_H
