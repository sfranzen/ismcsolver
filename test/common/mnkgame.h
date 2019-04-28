/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef MNKGAME_H
#define MNKGAME_H

#include <ismcts/game.h>
#include <vector>
#include <utility>
#include <iosfwd>

/* The m-n-k game
 *
 * The m-n-k game is a generalised tic-tac-toe, where the board has m by n
 * fields and the goal is to be the first to connect k fields in a row, which
 * may be horizontal, vertical or diagonal.
 */
class MnkGame : public ISMCTS::POMGame<int>
{
public:
    explicit MnkGame(unsigned m = 3, unsigned n = 3, unsigned k = 3);
    virtual Ptr cloneAndRandomise(unsigned observer) const override;
    virtual unsigned currentPlayer() const override;
    virtual unsigned nextPlayer(unsigned player) const override;
    virtual std::vector<int> validMoves() const override;
    virtual void doMove(const int move) override;
    virtual double getResult(unsigned player) const override;
    friend std::ostream &operator<<(std::ostream &out, const MnkGame &g);

protected:
    using Stride = std::pair<int,int>;

    unsigned m_m, m_n, m_k;
    std::vector<std::vector<int>> m_board;
    std::vector<int> m_moves;
    unsigned m_player;
    double m_result;

    void markBoard(int move, unsigned player);
    unsigned row(int move) const;
    unsigned column(int move) const;
    bool isWinningMove(int move, unsigned player) const;
    bool hasWinningSequence(int move, Stride stride, unsigned player) const;
    bool continueCounting(long row, long col, unsigned player) const;
};

#endif // MNKGAME_H
