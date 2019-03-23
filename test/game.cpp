/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "game.h"
#include <algorithm>
#include <functional>
#include <numeric>
#include <iostream>

Game::Game(unsigned side)
    : m_board(-side, side*side)
    , m_moves(side*side)
    , m_side(side)
    , m_player(0)
    , m_result(-1)
{
    std::iota(begin(m_moves), end(m_moves), 0);
}

Game::Ptr Game::cloneAndRandomise(unsigned) const
{
    return std::unique_ptr<ISMCTS::Game<int>>(new Game(*this));
}

void Game::doMove(const int move)
{
    m_board[move] = m_player;
    m_moves.erase(std::find(begin(m_moves), end(m_moves), move));
    if (checkWin(move))
        m_result = 1 - m_player;
    else if (!m_moves.empty())
        m_player = 1 - m_player;
    else
        m_result = 0.5;
}

double Game::getResult(unsigned player) const
{
    return player == 0 ? m_result : 1 - m_result;
}

std::vector<int> Game::validMoves() const
{
    if (m_result == -1)
        return m_moves;
    return {};
}

unsigned Game::currentPlayer() const
{
    return m_player;
}

bool Game::checkWin(int move) const
{
    using namespace std;
    vector<slice> slices;
    slices.push_back({move / m_side * m_side, m_side, 1});
    slices.push_back({move % m_side, m_side, m_side});
    // Add one or both diagonals depending on move
    for (unsigned i : {0u, m_side - 1})
        if (move % (m_side + 1) == i || move % (m_side - 1) == 0)
            slices.push_back({i, m_side, m_side + 1 - 2 * i / (m_side - 1)});

    // Check whether any slice is occupied by a single player
    return any_of(begin(slices), end(slices), [&](const slice &s){
        return checkSlice(s);
    });
}

bool Game::checkSlice(const std::slice &s) const
{
    using namespace std;
    const valarray<int> section = m_board[s];
    return section[0] != -int(m_side) && all_of(begin(section), end(section), bind1st(equal_to<int>(), section[0]));
}

std::ostream& operator<<(std::ostream &out, const Game &g)
{
    using namespace std;
    for (unsigned i = 0; i < g.m_side; ++i) {
        const auto start = begin(g.m_board) + i * g.m_side;
        for_each(start, start + g.m_side, [](int i){ printf("% 3d", i); });
        out << endl;
    }
    return out;
}
