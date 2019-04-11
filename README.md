# ismcsolver
[![Build Status](https://travis-ci.org/sfranzen/ismcsolver.svg?branch=master)](https://travis-ci.org/sfranzen/ismcsolver)
[![codecov](https://codecov.io/gh/sfranzen/ismcsolver/branch/master/graph/badge.svg)](https://codecov.io/gh/sfranzen/ismcsolver)

ismcsolver is a header-only C++11 library providing [information set Monte Carlo tree search][ISMCTS] (ISMCTS) algorithms. Monte Carlo tree search is a (game) AI decision making algorithm noted for its applicability to many different games of perfect information, requiring no domain-specific knowledge and only little information about a game's state. ISMCTS is an elegant extension of this technique to imperfect information games, where not all information is visible to all players, possibly combined with factors of randomness.

The code began its life inside a Qt game I wrote to explore game AI, eventually extracted to make testing easier and ported to standard C++ to make it portable to any future projects. It provides class templates to allow using the algorithm with a generic type of game using a generic type of move.

## Usage
You can either install the header files or just copy them into your project sources. Your game (or engine) class should implement the abstract interface [`ISMCTS::Game<Move>`][game], replacing the template parameter `Move` with the data type (or class) representing a player's move. The library provides two solver class templates, [`ISMCTS::SOSolver<Move>`][SO] and [`ISMCTS::MOSolver<Move>`][MO]. Their differences are explained [below](#features), but they should be instantiated with the same `Move` type to obtain an object that can select moves for the AI player(s), e.g.
```cpp
#include <ismcts/sosolver.h>
// ...
void MyGame::doAIMove()
{
    ISMCTS::SOSolver<MyMove> solver;
    this->doMove(solver(*this));
}
```
A simple card game (Knockout Whist) is included [here][whist], based on the example Python implementation available [here][py]. Its main purpose is to test the compilation and functioning of the library code, but it also serves as an example implementation of the `ISMCTS::Game` interface.

Documentation of the library templates, generated by doxygen, is also hosted in this repository and can be viewed at its [GitHub Page][docs].

[ISMCTS]: https://pure.york.ac.uk/portal/files/13014166/CowlingPowleyWhitehouse2012.pdf
[game]: include/ismcts/game.h
[whist]: test/knockoutwhist.cpp
[SO]: include/ismcts/sosolver.h
[MO]: include/ismcts/mosolver.h
[py]: https://gist.github.com/kjlubick/8ea239ede6a026a61f4d
[docs]: https://sfranzen.github.io/ismcsolver/

## Type requirements
The type `Move` specified for the game and sequential solver templates must be a *[TrivialType]* that is *[EqualityComparable]*. Solvers with root parallelisation use a `std::map` to evaluate visit counts, so the `Move` must additionally be *[LessThanComparable]*.

[TrivialType]: https://en.cppreference.com/w/cpp/named_req/TrivialType
[EqualityComparable]: https://en.cppreference.com/w/cpp/named_req/EqualityComparable
[LessThanComparable]: https://en.cppreference.com/w/cpp/named_req/LessThanComparable

## About random numbers
Game implementations will need a source of random numbers. Because `std::rand` does not guarantee a good quality sequence and may not be thread safe, modern code should use the `<random>` header. The objects representing generator engines are large, so the following is a good way of providing one to your class:
```cpp
#include <random>
// definition as member of MyGame, could be const or static
std::mt19937 &MyGame::prng()
{
    static thread_local std::mt19937 prng {std::random_device{}()};
    return prng;
}
```
This sets up one pseudorandom Mersenne Twister engine per thread, seeded with random numbers from a non-deterministic source if available. It can be used for shuffling, e.g. `std::shuffle(cards.begin(), cards.end(), prng());`. Single numbers on (closed) intervals should be generated using one of the available distributions:
```cpp
std::uniform_int_distribution<> singleDie {1, 6};
auto result = singleDie(prng());
```

## Features
The basic algorithm can be applied, modified and executed in different ways. This section lists what is currently implemented.

### SO- and MO-ISMCTS
These are two variants of the algorithm, respectively Single-Observer and Multiple-Observer. The former is implemented in `ISMCTS::SOSolver` and, as its name implies, observes the game from only one perspective: that of the player conducting the search. It only builds a tree for that player and treats all opponent moves as fully observable. While that is indeed the case in many games, some additionally feature actions that are hidden from the other players. MO-ISMCTS, implemented in `ISMCTS::MOSolver`, is intended for such games; it builds a tree for each player to model the presence of this extra hidden information.

### Multithreading
Several approaches to parallel tree searching exist, see e.g. [Parallelization of Information Set Monte Carlo Tree Search][par] by Nick Sephton and the authors of ISMCTS. At present, this library provides the most straightforward method of root parallelisation. It distributes the iterations over the system threads, with each thread searching a separate tree. The statistics from the root of each tree are then combined to find the overall best move. This method is very fast as it avoids synchronisation issues and the additional work is limited to a single layer of nodes in each tree.

This is implemented using the `std::thread` class and can be used by providing the optional second `ExecutionPolicy` parameter to either class template. Template specialisations are provided for two policies, defined in [execution.h]: `ISMCTS::Sequential` (the default) and `ISMCTS::RootParallel`. For example, the following instantiates a parallel `MOSolver` for a two player game where `int` represents a move:
```cpp
ISMCTS::MOSolver<int, ISMCTS::RootParallel> solver {2};
```
[execution.h]: include/ismcts/execution.h
[par]: https://www-users.cs.york.ac.uk/~nsephton/papers/wcci2014-ismcts-parallelization.pdf

### Time-limited execution
Each solver type has two constructors; one that sets the search operator to iterate a fixed number of times, the other instead letting it search for a fixed length of time (a `std::chrono::duration<double>`). Both the mode of operation and the length of the search can be changed after instantiation. A duration can be created using any of the convenience typedefs in [`std::chrono`][chrono], e.g.:

```cpp
using namespace std::chrono;
ISMCTS::SOSolver<int> solver {milliseconds(5)};
```
or even shorter with literal operators, since C++14:
```cpp
using namespace std::chrono_literals;
ISMCTS::SOSolver<int> solver {5ms};
```
[chrono]: https://en.cppreference.com/w/cpp/header/chrono

## The algorithm
The following is only a short summary of how ISMCTS works. For the full technical details, see the [official article][ISMCTS] by Peter I. Cowling, Edward J. Powley and Daniel Whitehouse.

Most importantly, the "information set" part of ISMCTS refers to the set of all possible game states consistent with a given player's observation of a game so far. In a typical card game, for example, it contains the permutations of the cards possibly held by the player's opponents, given the game's rules and the sequence of cards already played. The algorithm works by taking random samples from this set, called determinisations, to gradually build an information tree using regular Monte Carlo searches:

0. *Determinise*;
1. *Select:* Using a selection algorithm, choose a sequence of moves from the root of the tree until either a node with unexplored moves is reached or the game ends;
2. *Expand:* If there are unexplored moves, choose one at random and create a new node for it;
3. *Simulate:* Continue applying random moves from this state until the game ends;
4. *Backpropagate:* Update the tree by incrementing the visit counter and adding the score for the given player to this final node and each of its parents.

Successive iterations of these steps result in sequences of moves that are initially random, but increasingly become shaped by the availability of moves in different determinisations as well as the selection algorithm. Different selection algorithms are possible, but this library for now uses the UCB (Upper Confidence Bound) algorithm also used by the authors of ISMCTS.

The total number of iterations performed per search may be dictated, for example, by the available computational budget or a desired player strength. Upon finishing the search, the move corresponding to the most visited child node of the root of the resulting tree is selected as the most promising move.

## License
This project is licensed under the MIT License, see the [LICENSE](LICENSE) file for details.
