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

/**
 * Required interface for a game to be simulated by ISMCTS algorithms.
 *
 * The game should be able to function as a finite state machine, so that after
 * processing each move in doMove it is ready to accept the next.
 */
template<class Move>
struct Game
{
    using Ptr = std::unique_ptr<Game>;

    virtual ~Game() = default;

    /// Constructs a copy of the game state, with the information unknown to the
    /// observer player randomised.
    virtual Ptr cloneAndRandomise(unsigned int observer) const = 0;

    /// The player making a move from this state.
    virtual unsigned int currentPlayer() const = 0;

    /// Valid moves for the current state.
    /// @return A list of valid moves or an empty list if the game is finished.
    virtual std::vector<Move> validMoves() const = 0;

    /// Apply the given move to the state, and update current player to specify
    /// whose turn is next.
    virtual void doMove(const Move move) = 0;

    /// Return result for the given player.
    /// This function should preferentially return numbers in the range [0, 1],
    /// for example 0 for a loss, 0.5 for a draw and 1 for a win. It is only
    /// called on finished game states.
    virtual double getResult(unsigned int player) const = 0;
};

/**
 * Interface for games that have partially observable moves (POM).
 *
 * This is required for the game to work with the MOSolver class, which needs to
 * be able to identify all players in order to maintain individual trees.
 */
template<class Move>
struct POMGame : public Game<Move>
{
    /// Return the player whose turn it will be after the given player.
    virtual unsigned int nextPlayer(unsigned int player) const = 0;
};

} // ISMCTS

#endif // ISMCTS_GAME_H
