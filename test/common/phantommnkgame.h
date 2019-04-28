/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef PHANTOMMNKGAME_H
#define PHANTOMMNKGAME_H

#include "mnkgame.h"
#include <vector>
#include <array>

/* Phantom m-n-k game
 *
 * In this version of the m-n-k game, the players do not know which fields are
 * occupied by their opponent, so those fields are initially apparently
 * available. The player is notified upon trying to play an occupied field and
 * must choose another field, until an unoccupied one is found and marked.
 */
class PhantomMnkGame : public MnkGame
{
public:
    explicit PhantomMnkGame(int m = 3, int n = 3, int k = 3);
    Ptr cloneAndRandomise(Player observer) const override;
    void doMove(const int move) override;
    std::vector<int> validMoves() const override;

protected:
    std::array<std::vector<int>, 2> m_available;

    // Undo moves for the given player and return the number of moves
    unsigned undoMoves(Player player);
    // Try applying the given number of randomly selected moves for the given
    // player to the current game state, until a non-terminal state is found
    void randomReplay(Player player, unsigned numMoves);
};

#endif // PHANTOMMNKGAME_H
