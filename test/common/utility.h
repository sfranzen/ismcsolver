/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <ismcts/game.h>
#include <ismcts/utility.h>

template<class Game>
auto randomMove(Game const &game)
{
    return ISMCTS::randomElement(game.validMoves());
}

template<class Move>
struct RandomPlayer
{
    auto operator()(ISMCTS::Game<Move> const &game) const
    {
        return randomMove(game);
    }
};

template<class Game>
void doValidMove(Game &game)
{
    game.doMove(randomMove(game));
}

#endif
