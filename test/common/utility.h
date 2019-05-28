/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <ismcts/game.h>
#include <ismcts/utility.h>

template<class Move>
inline Move randomMove(ISMCTS::Game<Move> const &game)
{
    return ISMCTS::randomElement(game.validMoves());
}

template<class Game>
inline void doValidMove(Game &game)
{
    game.doMove(game.validMoves().front());
}

#endif
