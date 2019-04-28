/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "phantommnkgame.h"
#include <algorithm>
#include <numeric>
#include <random>

PhantomMnkGame::PhantomMnkGame(int m, int n, int k)
    : MnkGame{m, n, k}
{
    for (auto &moves : m_available) {
        moves.resize(m * n);
        std::iota(moves.begin(), moves.end(), 0);
    }
}

/* We have the following information:
 * * Observer's own previous moves;
 * * Observer's available moves;
 * * Number of previous opponent moves;
 * * The game state is not yet terminal.
 *
 * Given these constraints, generate a cloned game state with a random sampling
 * of the hidden information.
 */
PhantomMnkGame::Ptr PhantomMnkGame::cloneAndRandomise(unsigned observer) const
{
    auto clone = new PhantomMnkGame{*this};
    const auto opponent = nextPlayer(observer);
    const auto numMoves = clone->undoMoves(opponent);
    clone->randomReplay(opponent, numMoves);
    return Ptr{clone};
}

// Only undo those moves that are still marked as available in the opponent's
// view of the game state
unsigned PhantomMnkGame::undoMoves(unsigned player)
{
    unsigned numMoves {0};
    auto &ourMoves = m_available[player];
    const auto &opponentMoves = m_available[nextPlayer(player)];
    for (int move = 0; move < m_m * m_n; ++move) {
        auto &pos = m_board[row(move)][column(move)];
        if (pos == player && std::binary_search(opponentMoves.begin(), opponentMoves.end(), move)) {
            pos = -1;
            ourMoves.emplace_back(move);
            m_moves.emplace_back(move);
            ++numMoves;
        }
    }
    // Keep the available moves sorted for binary_search
    std::sort(ourMoves.begin(), ourMoves.end());
    return numMoves;
}

void PhantomMnkGame::randomReplay(unsigned player, unsigned numMoves)
{
    static thread_local std::mt19937 prng {std::random_device{}()};
    auto newState = *this;
    auto moves = m_moves;
    std::shuffle(moves.begin(), moves.end(), prng);
    moves.resize(numMoves);

    // Ensure a non-winning state
    for (auto move : moves)
        if (newState.isWinningMove(move, player))
            randomReplay(player, numMoves);
        else
            newState.markBoard(move, player);

    // Remove replayed sequence from the new state
    for (auto &target : {&newState.m_moves, &newState.m_available[player]})
        for (auto move : moves)
            target->erase(std::find(target->begin(), target->end(), move));

    *this = std::move(newState);
}

void PhantomMnkGame::doMove(const int move)
{
    auto &available = m_available[m_player];
    auto loc = std::find(available.begin(), available.end(), move);
    if (loc == available.end())
        throw std::out_of_range("Illegal move");
    available.erase(loc);

    loc = std::find(m_moves.begin(), m_moves.end(), move);
    if (loc == m_moves.end())
        return;

    m_moves.erase(loc);
    markBoard(move, m_player);

    if (isWinningMove(move, m_player)) {
        m_result = 1 - m_player;
        for (auto &moves : m_available)
            moves.clear();
    } else if (!m_moves.empty())
        m_player = 1 - m_player;
    else
        m_result = 0.5;
}

std::vector<int> PhantomMnkGame::validMoves() const
{
    return m_available[m_player];
}
