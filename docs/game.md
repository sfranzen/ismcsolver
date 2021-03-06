---
title: Game interfaces
---

# Game interfaces
The abstract class templates listed [below](#game) specify two interfaces a game could implement to be used with the ISMCTS algorithm, one for each of the solver classes. While a game implementation is free to store and manipulate any data it needs, a few things should be kept in mind to ensure that the trees generated by the algorithm accurately reflect the structure of the game. These are explained in the next section. Additionally, some simple games are included in the repository to test the algorithms, which may also serve as example implementations. They can be found in the [test/common] folder.

[test/common]: https://github.com/sfranzen/ismcsolver/tree/master/test/common

## Implementation guidelines
Central to the creation of trees is the interaction between the game functions `cloneAndRandomise`, `validMoves`, `doMove` and `getResult`. They tie into the algorithm as follows:
* `cloneAndRandomise` is called at the start of every search iteration;
* `validMoves` and `doMove` are then used repeatedly to generate a path through the tree, until either:
    1. A node is reached that does not yet have branches for all of the valid moves;
    2. The vector of valid moves is empty, indicating that the game has terminated.
* In the first case, one node is created for one of the unexplored moves and selected, followed by a sequence of randomly chosen valid moves until an empty vector is returned;
* In both cases, the `getResult` function is then called at each of the visited nodes for the player whose action led to that node, updating its statistics for the next iteration.

These are therefore explained in some more detail here.

### Generating clones
A clone must be returned in the form of a `std::unique_ptr` holding a game instance. In general, the implementation should follow a pattern like the following:
```cpp
Ptr FooGame::cloneAndRandomise(Player observer) const
{
    // A std::unique_ptr containing a copy-constructed instance
    auto clone = std::make_unique<FooGame>(*this);

    // A non-const member function can be used to randomise the clone
    clone->randomiseStuff(observer);
    return clone;
}
```
In any game with imperfect information, each player has different observations of the game state. This means the clone should only duplicate any information the given player could have deduced from his observations of the game so far. All other information should be randomised. For example, a card game player can generally see his own cards and those his opponents have revealed during play, so this information should be conserved. However, the player may at some point discover that another player does not have a particular suit. From that point onwards, `cloneAndRandomise` with this player as the `observer` should only return clones where that opponent lacks the same suit, and cards from the other suits are taken at random from the set of all cards unknown by the observer. This ensures that the algorithm will not explore paths that are not available in the real game. Obtaining a cloned state subject to such constraints may be a trial-and-error process, as it is often simpler to repeatedly validate a random permutation of the state than it is to guarantee that a valid one is created on every try.

### Handling partially observable moves and chance events
It may be the case that players cannot (fully) observe the positions occupied or moves played by their opponents. Game mechanics involving partial observability are varied; strategy games for example frequently have a "fog of war" where players cannot see enemy units outside some range from their own positions. Simultaneous moves are also partially observable, since the players cannot see the others' choices before revealing their own.

A chance event is any point in the gameplay where the outcome depends on some random process. Common examples are rolling dice and revealing a card from a shuffled deck to all players, as in Blackjack. Shuffling and dealing cards *before* a game is technically also a chance event, but does not need special handling as it takes place before any of the players can act, so before any game trees are created.

The essence of both of these kinds of game mechanics is that players cannot be sure about the evolution of the game state, until information is revealed to them by some means. This can be implemented in a game by including a so-called *Environment* or *Nature* player in the sequence of players. Such a player can be used for any required task like resolving combat, generating random outcome(s), or revealing information about partially observable or simultaneous moves (which have of course been selected in turn-based fashion, but have not yet affected the game from the players' perspective). Most importantly, it should update the game state accordingly so that future clones can make use of the information.

Which `validMoves` should be returned when it is the environment player's turn depends on the use case. An empty vector should only be returned if the game is finished, so if the player is simply acting as a referee, a single constant dummy element should be provided, to cause the algorithm to create only one branch in a player's tree at this point. The subsequent `doMove` handling this move can be used to perform the required actions and need not use the move's information. If the action is a chance event instead, the vector could contain meaningful  moves. In this case, the `getResult` function should return a constant score of 0 for the environment player, to keep the algorithm from developing a preference for any of the resulting branches.

Examples of both techniques are demonstrated in the test game [Goofspiel]. The environment player (player 2) acts at the beginning of each round to reveal one of the "prize" cards, as well at the end to reveal and evaluate the (closed) bids selected by the players.

[goofspiel]: https://github.com/sfranzen/ismcsolver/tree/master/test/common/goofspiel.cpp

## Game class template
Defined in `<ismcts/game.h>`
```cpp
template<class Move> struct Game;
```
This class represents a game that has fully observable moves, in other words, players can always see the actions done by the other players. A game of this kind should be used with the single-observer `SOSolver` as there is no advantage to building trees for multiple players.

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
virtual void doMove(Move const move) = 0;
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

## POMGame class template
Defined in `<ismcts/game.h>`
```cpp
template<class Move> struct POMGame : public Game<Move>;
```
Represents a game with Partially Observable Moves (POMs). It adds one method to the `Game` interface to query the participating players, which is required for the MOSolver class to be able to maintain separate trees for each player.

### Member functions
```cpp
virtual std::vector<Player> players() const = 0;
```
Returns the list of all player identifiers.
