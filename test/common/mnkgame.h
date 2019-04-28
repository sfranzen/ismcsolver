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
    explicit MnkGame(int m = 3, int n = 3, int k = 3);
    virtual Ptr cloneAndRandomise(Player observer) const override;
    virtual Player currentPlayer() const override;
    virtual std::vector<Player> players() const override;
    virtual std::vector<int> validMoves() const override;
    virtual void doMove(const int move) override;
    virtual double getResult(Player player) const override;
    friend std::ostream &operator<<(std::ostream &out, const MnkGame &g);

protected:
    using Stride = std::pair<int,int>;

    int m_m, m_n, m_k;
    std::vector<std::vector<int>> m_board;
    std::vector<int> m_moves;
    unsigned m_player;
    double m_result;

    void markBoard(int move, Player player);
    int row(int move) const;
    int column(int move) const;
    bool isWinningMove(int move, Player player) const;
    bool hasWinningSequence(int move, Stride stride, Player player) const;
    bool continueCounting(int row, int col, Player player) const;
};

#endif // MNKGAME_H
