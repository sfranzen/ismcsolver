/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "tictactoe.h"
#include <algorithm>
#include <numeric>
#include <iostream>

TicTacToe::TicTacToe()
    : m_board(-1, 9)
    , m_moves(9)
    , m_player(0)
    , m_result(-1)
{
    std::iota(m_moves.begin(), m_moves.end(), 0);
}

TicTacToe::Ptr TicTacToe::cloneAndRandomise(unsigned) const
{
    return Ptr(new TicTacToe(*this));
}

void TicTacToe::doMove(const int move)
{
    const auto loc = std::find(m_moves.begin(), m_moves.end(), move);
    if (loc == m_moves.end())
        throw std::out_of_range("Illegal move");
    m_moves.erase(loc);
    m_board[move] = m_player;
    if (checkWin(move))
        m_result = 1 - m_player;
    else if (!m_moves.empty())
        m_player = 1 - m_player;
    else
        m_result = 0.5;
}

double TicTacToe::getResult(unsigned player) const
{
    return player == 0 ? m_result : 1 - m_result;
}

std::vector<int> TicTacToe::validMoves() const
{
    return m_result == -1 ? m_moves : std::vector<int>{};
}

unsigned TicTacToe::currentPlayer() const
{
    return m_player;
}

bool TicTacToe::checkWin(int move) const
{
    using namespace std;
    vector<slice> slices;

    // Check the row and column corresponding to the move and possibly one or
    // both diagonals
    slices.emplace_back(move / 3, 3, 1);
    slices.emplace_back(move % 3, 3, 3);
    for (int i : {0, 2})
        if (move % 4 == i || move % 2 == 0)
            slices.emplace_back(i, 3, 4 - i);

    // Check whether any slice is occupied by a single player
    return any_of(slices.begin(), slices.end(), [&](const slice &s){
        return checkSlice(s);
    });
}

bool TicTacToe::checkSlice(const std::slice &s) const
{
    using namespace std;
    const valarray<int> section = m_board[s];
    return section[0] != -1 && all_of(begin(section) + 1, end(section), [&](int i){ return i == section[0]; });
}

std::ostream& operator<<(std::ostream &out, const TicTacToe &g)
{
    for (unsigned i = 0; i < 3; ++i) {
        const auto start = std::begin(g.m_board) + i * 3;
        std::for_each(start, start + 3, [](int i){ std::printf("% 3d", i); });
        out << "\n";
    }
    return out;
}
