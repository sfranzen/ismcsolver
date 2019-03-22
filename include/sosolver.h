#ifndef ISMCTS_SOSOLVER_H
#define ISMCTS_SOSOLVER_H

#include "game.h"
#include "node.h"

#include <memory>
#include <vector>
#include <random>

namespace ISMCTS
{

/// Single observer information set Monte Carlo tree search solver class.
template<class Move> class SOSolver
{
public:
    /**
     * Constructor.
     *
     * @param iterMax Sets the number of simulations performed for each search.
     * @param exploration Sets the algorithm's bias towards unexplored moves.
     *      It must be positive; the authors of the algorithm suggest 0.7.
     */
    explicit SOSolver(std::size_t iterMax = 1000, double exploration = 0.7)
        : m_iterMax {iterMax}
        , m_exploration {std::max(exploration, 0.0)}
    {}

    virtual ~SOSolver() {}

    /**
     * Search operator.
     *
     * @return The most promising move from the given state.
     */
    Move operator()(const Game<Move> &rootState) const
    {
        Node<Move> root;
        for (std::size_t i {0}; i < m_iterMax; ++i)
            search(&root, rootState);
        const auto &rootList = root.children();
        const auto &mostVisited = *std::max_element(rootList.begin(), rootList.end());
        return mostVisited->move();
    }

protected:
    const std::size_t m_iterMax;
    const double m_exploration;

    /// Traverse a single sequence of moves.
    void search(Node<Move> *rootNode, const Game<Move> &rootState) const
    {
        auto randomState = rootState.cloneAndRandomise(rootState.currentPlayer());
        auto statePtr = randomState.get();
        select(rootNode, statePtr);
        expand(rootNode, statePtr);
        simulate(statePtr);
        backPropagate(rootNode, statePtr);
    }

    /**
     * Selection stage
     *
     * Descend the tree until a node is reached that has unexplored moves, or
     * is a terminal node (no more moves available).
     */
    void select(Node<Move> *&node, Game<Move> *state) const
    {
        auto validMoves = state->validMoves();
        while (!selectNode(node, validMoves)) {
            node = node->ucbSelectChild(validMoves, m_exploration);
            state->doMove(node->move());
            validMoves = state->validMoves();
        }
    }

    static bool selectNode(const Node<Move> *node, const std::vector<Move> &moves)
    {
        return moves.empty() || !node->untriedMoves(moves).empty();
    }

    /**
     * Expansion stage
     *
     * Choose a random unexplored move, add it to the children of the current
     * node and select this new node.
     */
    static void expand(Node<Move> *&node, Game<Move> *state)
    {
        const auto untriedMoves = node->untriedMoves(state->validMoves());
        if (!untriedMoves.empty()) {
            const auto move = randMove(untriedMoves);
            state->doMove(move);
            node = node->addChild(move, state->currentPlayer());
        }
    }

    /**
     * Simulation stage
     *
     * Continue performing random available moves from this state until the end
     * of the game.
     */
    static void simulate(Game<Move> *state)
    {
        auto moves = state->validMoves();
        while (!moves.empty()) {
            state->doMove(randMove(moves));
            moves = state->validMoves();
        }
    }

    /**
     * Backpropagation stage
     *
     * Update the node statistics using the rewards from the terminal state.
     */
    static void backPropagate(Node<Move> *&node, const Game<Move> *state)
    {
        while (node) {
            node->update(state);
            node = node->parent();
        }
    }

    static const Move &randMove(const std::vector<Move> &moves) {
        static std::random_device rd;
        static std::mt19937 rng {rd()};
        std::uniform_int_distribution<std::size_t> dist {0, moves.size() - 1};
        return moves[dist(rng)];
    }
};

} // ISMCTS

#endif // ISMCTS_SOSOLVER_H
