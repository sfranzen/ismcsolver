/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "mnkgame.h"
#include <algorithm>
#include <numeric>
#include <array>
#include <ostream>
#include <iomanip>

namespace
{
    inline int limit(int i) noexcept
    {
        return std::max(i, 0);
    }
}

MnkGame::MnkGame(int m, int n, int k)
    : m_m{limit(m)}, m_n{limit(n)}, m_k{limit(k)}
    , m_board(n, std::vector<int>(m, -1))
    , m_moves(m * n)
    , m_player{0}
    , m_result{-1}
{
    std::iota(m_moves.begin(), m_moves.end(), 0);
}

MnkGame::Ptr MnkGame::cloneAndRandomise(unsigned) const
{
    return Ptr{new MnkGame{*this}};
}

void MnkGame::doMove(const int move)
{
    const auto loc = std::find(m_moves.begin(), m_moves.end(), move);
    if (loc == m_moves.end())
        throw std::out_of_range("Illegal move");

    m_moves.erase(loc);
    markBoard(move, m_player);

    if (isWinningMove(move, m_player))
        m_result = 1 - m_player;
    else if (!m_moves.empty())
        m_player = 1 - m_player;
    else
        m_result = 0.5;
}

double MnkGame::getResult(Player player) const
{
    return player == 0 ? m_result : 1 - m_result;
}

std::vector<int> MnkGame::validMoves() const
{
    return m_result == -1 ? m_moves : std::vector<int>{};
}

MnkGame::Player MnkGame::currentPlayer() const
{
    return m_player;
}

std::vector<MnkGame::Player> MnkGame::players() const
{
    return {0,1};
}

void MnkGame::markBoard(int move, Player player)
{
    m_board[row(move)][column(move)] = player;
}

int MnkGame::row(int move) const
{
    return move / m_m;
}

int MnkGame::column(int move) const
{
    return move % m_m;
}

bool MnkGame::isWinningMove(int move, Player player) const
{
    // Strides representing the directions in which to look for sequences
    static constexpr std::array<const Stride, 4> strides { {
        {0, 1}, // horizontal
        {1, 0}, // vertical
        {1, 1}, // descending diagonal
        {-1, 1} // ascending diagonal
    } };

    return std::any_of(strides.begin(), strides.end(), [&](const Stride &stride) {
        return hasWinningSequence(move, stride, player);
    });
}

// Check from the position of the given move, in both the positive and negative
// directions of the given stride vector, whether the given player occupies
// enough positions to win the game.
bool MnkGame::hasWinningSequence(int move, Stride stride, Player player) const
{
    unsigned count {1};
    const auto r0 {row(move)};
    const auto c0 {column(move)};

    for (int i : {-1, 1}) {
        auto r = r0, c = c0;
        while (continueCounting(r += i * stride.first, c += i * stride.second, player))
            ++count;
    }
    return count == m_k;
}

// Continue if the given position is valid and occupied by the given player
bool MnkGame::continueCounting(int row, int col, Player player) const
{
    if (row < 0 || col < 0 || row >= m_n || col >= m_m)
        return false;

    return m_board[row][col] == player;
}

std::ostream& operator<<(std::ostream &out, const MnkGame &g)
{
    for (const auto &row : g.m_board) {
        for (auto p : row)
            out << std::setw(3) << p;
        out << "\n";
    }
    return out;
}
