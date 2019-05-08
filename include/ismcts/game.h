/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_GAME_H
#define ISMCTS_GAME_H

#include <memory>
#include <vector>

namespace ISMCTS
{

template<class Move>
struct Game
{
    using Ptr = std::unique_ptr<Game>;
    using Player = unsigned int;

    virtual ~Game() = default;

    virtual Ptr cloneAndRandomise(Player observer) const = 0;

    virtual Player currentPlayer() const = 0;

    virtual std::vector<Move> validMoves() const = 0;

    virtual void doMove(const Move move) = 0;

    virtual double getResult(Player player) const = 0;

    virtual bool currentMoveSimultaneous() const
    {
        return false;
    }
};

template<class Move>
struct POMGame : public Game<Move>
{
    using typename Game<Move>::Player;

    virtual std::vector<Player> players() const = 0;
};

} // ISMCTS

#endif // ISMCTS_GAME_H
