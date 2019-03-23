# ismcsolver
ismcsolver provides a C++ implementation of information set Monte Carlo tree search (ISMCTS) algorithms. Monte Carlo tree search is a (game) AI decision making algorithm noted for its applicability to many different games of perfect information, requiring no domain-specific knowledge and only little information about a game's state. ISMCTS is an elegant extension of this technique to imperfect information games, where not all information is visible to all players, possibly combined with factors of randomness.

This implementation uses C++ class templates to apply the algorithm to a generic type of game using a generic type of move.
