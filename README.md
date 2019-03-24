# ismcsolver
ismcsolver provides a C++ implementation of information set Monte Carlo tree search (ISMCTS) algorithms. Monte Carlo tree search is a (game) AI decision making algorithm noted for its applicability to many different games of perfect information, requiring no domain-specific knowledge and only little information about a game's state. ISMCTS is an elegant extension of this technique to imperfect information games, where not all information is visible to all players, possibly combined with factors of randomness.

This implementation uses C++ class templates to apply the algorithm to a generic type of game using a generic type of move. At the moment it adheres to the C++11 standard.

## Usage
You can either install the header files or just copy them into your project sources. Your game (or engine) class should implement the abstract interface `ISMCTS::Game<Move>`, replacing the template parameter `Move` with the data type (or class) representing a player's move. Then all that is needed is to instantiate a solver for the AI player(s), e.g.
```cpp
void MyGame::doAIMove()
{
    ISMCTS::SOSolver<MyMove> solver;
    this->doMove(solver(this));
}
```
A simple working example game (tic tac toe) is included [here](test).

## License
This project is licensed under the MIT License, see the LICENSE file for details.
