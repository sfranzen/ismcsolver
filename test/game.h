/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef GAME_H
#define GAME_H

#include "../include/game.h"
#include <valarray>
#include <vector>
#include <ostream>

class Game : public ISMCTS::Game<int>
{
public:
    Game(unsigned side = 3);
    virtual Ptr cloneAndRandomise(unsigned observer) const override;
    virtual unsigned currentPlayer() const override;
    virtual std::vector<int> validMoves() const override;
    virtual void doMove(const int move) override;
    virtual double getResult(unsigned player) const override;
    friend std::ostream &operator<<(std::ostream &out, const Game &g);
private:
    std::valarray<int> m_board;
    std::vector<int> m_moves;
    const unsigned m_side;
    unsigned m_player;
    double m_result;
    bool checkWin(int move) const;
    bool checkSlice(const std::slice &s) const;
};

#endif // GAME_H
