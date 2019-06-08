/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef GOOFSPIEL_H
#define GOOFSPIEL_H

#include <ismcts/game.h>
#include "card.h"

#include <vector>
#include <array>

/* Two-player Goofspiel
 *
 * Goofspiel is a simple card game for two or more players featuring simul-
 * taneous moves. One suit is used as the "prize" suit and the players each
 * receive one of the other suits. Play proceeds by turning over one of the
 * (shuffled) prize cards each turn, upon which players simultaneously bid by
 * choosing a rank from their hand (Ace to K). The value of the current prize
 * card goes to the winning bid or to neither player if the bids are tied.
 */
class Goofspiel : public ISMCTS::POMGame<Card>
{
public:
    explicit Goofspiel();

    Ptr cloneAndRandomise(Player observer) const override;
    Player currentPlayer() const override;
    void doMove(Card const move) override;
    double getResult(Player player) const override;
    std::vector<Player> players() const override;
    std::vector<Card> validMoves() const override;
    bool currentMoveSimultaneous() const override;

private:
    using Hand = std::vector<Card>;

    Hand m_prizes;
    Card m_currentPrize;
    std::array<Hand, 2> m_hands;
    std::array<Card, 2> m_moves;
    std::array<int, 2> m_scores {{0, 0}};
    Player m_player {2};
    bool m_drawPrize {true};

    void shufflePrizes();
    void handleP2Turn(Card const &move);
};

#endif // GOOFSPIEL_H
