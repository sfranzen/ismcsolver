---
title: Game interfaces
---

# Game interfaces
The header `<ismcts/game.h>` provides two abstract class templates specifying the interface a game should provide to the solver classes.

## Game
```cpp
template<class Move> struct Game;
```
This class represents a game that has fully observable moves, in other words, players can always see the actions done by the other players.
### Member types

| Type      | Definition                |
|:----------|:--------------------------|
|`Ptr`      |`std::unique_ptr<Game>`    |
|`Player`   |`unsigned int`             |

### Member functions
```cpp
virtual Ptr cloneAndRandomise(Player observer) const = 0;
```
Constructs a copy of the game state that has been *determinised:* any information which the `observer` cannot know with certainty has been randomly redistributed. The copy should, however, be consistent with deductions this player can make from observing the history of the game, such as some moves being unavailable to other players.

---
```cpp
virtual Player currentPlayer() const = 0;
```
The player about to make a move in the current state.

---
```cpp
virtual std::vector<Move> validMoves() const = 0;
```
The valid moves for the current player. An empty vector must be returned if the game is finished.

---
```cpp
virtual void doMove(const Move move) = 0;
```
Applies the given move to the state and updates the current player to specify whose turn is next.

---
```cpp
virtual double getResult(Player player) const = 0;
```
Returns the result for the given player. This function should preferably return numbers in the range [0, 1], for example 0 for a loss, 0.5 for a draw and 1 for a win. It is only called on finished game states.

---
```cpp
virtual bool currentMoveSimultaneous() const;
```
Indicates whether the current game state has a simultaneous move, i.e. multiple players must decide on a move without first seeing any of the other(s) decisions as in turn-based play. Override if the game features such moves; the default implementation always returns false.

## POMGame
```cpp
template<class Move> struct POMGame : public Game<Move>;
```
Represents a game with Partially Observable Moves (POMs). It adds one method to the `Game` interface to query the participating players, which is required for the MOSolver class to be able to maintain separate trees for each player.

### Member functions
```cpp
virtual std::vector<Player> players() const = 0;
```
Returns the list of all player identifiers.
