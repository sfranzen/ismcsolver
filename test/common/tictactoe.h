/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <ismcts/game.h>
#include <vector>
#include <valarray>
#include <ostream>

class TicTacToe : public ISMCTS::Game<int>
{
public:
    TicTacToe();
    virtual Ptr cloneAndRandomise(unsigned observer) const override;
    virtual unsigned currentPlayer() const override;
    virtual std::vector<int> validMoves() const override;
    virtual void doMove(const int move) override;
    virtual double getResult(unsigned player) const override;
    friend std::ostream &operator<<(std::ostream &out, const TicTacToe &g);
protected:
    std::valarray<int> m_board;
    std::vector<int> m_moves;
    unsigned m_player;
    double m_result;
    bool checkWin(int move) const;
    bool checkSlice(const std::slice &s) const;
};

#endif // TICTACTOE_H
