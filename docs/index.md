---
layout: page
title: ismcsolver documentation
---

# {{ page.title }}

This documentation contains a detailed reference of the ismcsolver library, automatically generated from the source code and included comments. Below is a brief outline of the template classes provided by the library, as well as a summary of the ISMCTS algorithm.

## The library
The procedure for using this library is straightforward:

* Implement the appropriate [interface](game.md) to be able to use your game with the solvers;
* Instantiate one of the [solver class templates](solvers.md):
    * ISMCTS::SOSolver for Single Observer solvers;
    * ISMCTS::MOSolver for Multiple Observer solvers.

    Each of these templates takes an ISMCTS::ExecutionPolicy as its second parameter, two of which are currently implemented:
    * ISMCTS::Sequential (the default, may be omitted) for single-threaded solvers;
    * ISMCTS::RootParallel for multi-threaded solvers that search a separate tree on each thread.
* Use the solver with an instance of your game to find a move.

## The algorithm
The following is only a short summary of the algorithm as introduced by Peter I. Cowling, Edward J. Powley and Daniel Whitehouse; for the full technical details, see the [official article][ISMCTS].

Most importantly, the "information set" part of ISMCTS refers to the set of all possible game states that are consistent with a given player's observation of the game. In a typical card game, for example, the player can see his own cards and those revealed in play, but not the cards held by his fellow players, or not dealt to anyone and left in the stack. The information set for this player contains all permutations of the cards he cannot see that are possible in a given game state. The algorithm works by taking random samples from this set, referred to as *determinisations*, to gradually populate a tree with information about sequences of moves that would be possible from the current state of the game, using a regular Monte Carlo tree search:

1. *Determinise*;
2. *Select:* Using a selection algorithm, choose a sequence of moves from the root of the tree until either a node with unexplored moves is reached or the game ends;
3. *Expand:* If there are unexplored moves, choose one at random and create a new node for it;
4. *Simulate:* Continue applying random moves from this state until the game ends;
5. *Backpropagate:* Update the tree by incrementing the visit counter and adding the score for the given player to this final node and each of its parents.

These steps may be repeated any number of times, resulting in sequences of moves that are initially random, but increasingly become shaped by the availability of moves in different determinisations as well as the selection algorithm. Different selection algorithms are possible, but this library for now uses the UCB (Upper Confidence Bound) algorithm also used by the authors of ISMCTS, which ranks candidate nodes using a combination of the number of times they were selected before and the cumulative game score obtained.

The total number of iterations performed per move may be dictated, for example, by the available computational budget or a desired player strength. Upon finishing the search, the move corresponding to the most visited child node of the root of the resulting tree is selected as the most promising move.

Samples from the information set are obtained using the ISMCTS::Game::cloneAndRandomise function. Because the set may be very large and require impractical numbers of iterations to sample representatively, it is important that the game states returned by this function are suitably constrained. For example, many games have rules that can force players to reveal information about their available moves, such as a failure to follow suit in a card game that requires it. The information sets for the other players should then be constrained to those permutations where the given player also does not have that suit.

[ISMCTS]: https://pure.york.ac.uk/portal/files/13014166/CowlingPowleyWhitehouse2012.pdf
