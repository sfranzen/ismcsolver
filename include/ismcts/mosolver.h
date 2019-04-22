/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_MOSOLVER_H
#define ISMCTS_MOSOLVER_H

#include "solverbase.h"
#include "game.h"
#include "node.h"
#include "execution.h"
#include "ucb1.h"

#include <memory>
#include <vector>
#include <thread>
#include <chrono>

namespace ISMCTS
{
/**
 * Multiple observer solver class template.
 *
 * The multiple observer solvers implement the MO-ISMCTS algorithm, which builds
 * a separate tree for each player and searches these simultaneously. This makes
 * it applicable to games with partially observable moves, i.e. where players
 * cannot always fully observe the other players' or teams' moves.
 */
template<class Move, class ExecutionPolicy = Sequential, class TreePolicy = UCB1<Move>>
class MOSolver : public SolverBase<Move, TreePolicy>, public ExecutionPolicy
{
public:
    using ExecutionPolicy::numThreads;

    /// The search trees for the current observer, one per player
    using TreeList = std::vector<Node<Move>>;
    /// The set of tree lists, one for each thread
    using TreeSet = std::vector<TreeList>;

    /**
     * Constructs a solver for a game with the given number of players that will
     * iterate the given number of times per search operation.
     *
     * @param exploration Sets the algorithm's bias towards unexplored moves.
     *      It must be positive; the authors of the algorithm suggest 0.7 for
     *      a game that reports result values on the interval [0,1].
     */
    explicit MOSolver(std::size_t numPlayers, std::size_t iterationCount = 1000, const TreePolicy &policy = TreePolicy{})
        : SolverBase<Move,TreePolicy>{policy}
        , ExecutionPolicy(iterationCount)
        , m_numPlayers{numPlayers}
    {}

    /**
     * Constructs a solver for a game with the given number of players that will
     * iterate for the given duration per search operation.
     *
     * @param exploration Sets the algorithm's bias towards unexplored moves.
     *      It must be positive; the authors of the algorithm suggest 0.7 for
     *      a game that reports result values on the interval [0,1].
     */
    explicit MOSolver(std::size_t numPlayers, std::chrono::duration<double> iterationTime, const TreePolicy &policy = TreePolicy{})
        : SolverBase<Move,TreePolicy>{policy}
        , ExecutionPolicy(iterationTime)
        , m_numPlayers{numPlayers}
    {}

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        std::vector<std::thread> threads(numThreads());
        m_trees = TreeSet(numThreads());
        for (auto &set : m_trees)
            set = TreeList(m_numPlayers);

        for (std::size_t t = 0; t < numThreads(); ++t)
            threads[t] = std::thread(&MOSolver::iterate, this, std::ref(m_trees[t]), std::ref(rootState));
        for (auto &t : threads)
            t.join();

        // Gather results for the current player from each thread
        TreeList currentPlayerTrees(numThreads());
        std::transform(m_trees.begin(), m_trees.end(), currentPlayerTrees.begin(), [&](TreeList &set){
            return set[rootState.currentPlayer()];
        });
        return MOSolver::bestMove(currentPlayerTrees);
    }

    /// Return the decision trees resulting from the most recent call to
    /// operator(). The resulting vector contains one vector of root nodes for
    /// each thread used for the search, where each node represents a different
    /// player's information tree.
    TreeSet &currentTrees() const
    {
        return m_trees;
    }

protected:
    using NodePtrList = std::vector<Node<Move>*>;

    std::size_t m_numPlayers;
    mutable TreeSet m_trees;

    void iterate(TreeList &trees, const Game<Move> &state) const
    {
        const auto iterations = this->iterationCount();
        if (iterations > 0) {
            for (std::size_t i {0}; i < iterations; ++i)
                search(trees, state);
        } else {
            auto duration = this->iterationTime();
            while (duration.count() > 0) {
                using namespace std::chrono;
                const auto start = high_resolution_clock::now();
                search(trees, state);
                duration -= high_resolution_clock::now() - start;
            }
        }
    }

    /// Traverse a single sequence of moves
    void search(TreeList &trees, const Game<Move> &rootState) const
    {
        NodePtrList roots(trees.size());
        std::transform(trees.begin(), trees.end(), roots.begin(), [](Node<Move> &n){ return &n; });
        auto randomState = rootState.cloneAndRandomise(rootState.currentPlayer());
        select(roots, *randomState);
        expand(roots, *randomState);
        this->simulate(*randomState);
        backPropagate(roots, *randomState);
    }

    /**
     * Selection stage
     *
     * Descend all trees until a node is reached that has unexplored moves, or
     * is a terminal node (no more moves available).
     */
    void select(NodePtrList &nodes, Game<Move> &state) const
    {
        const auto validMoves = state.validMoves();
        const auto player = state.currentPlayer();
        const auto &targetNode = nodes[player];
        if (!MOSolver::selectNode(targetNode, validMoves)) {
            const auto selection = targetNode->select(validMoves, this->m_treePolicy);
            for (auto &node : nodes)
                node = node->findOrAddChild(selection->move(), player);
            state.doMove(selection->move());
            select(nodes, state);
        }
    }

    /**
     * Expansion stage
     *
     * Choose a random unexplored move, add it to the children of all current
     * nodes and select these new nodes.
     */
    void expand(NodePtrList &nodes, Game<Move> &state) const
    {
        const auto player = state.currentPlayer();
        const auto untriedMoves = nodes[player]->untriedMoves(state.validMoves());
        if (!untriedMoves.empty()) {
            const auto move = this->randomMove(untriedMoves);
            for (auto &node : nodes)
                node = node->findOrAddChild(move, player);
            state.doMove(move);
        }
    }

    static void backPropagate(NodePtrList &nodes, const Game<Move> &state)
    {
        for (auto node : nodes)
            SolverBase<Move,TreePolicy>::backPropagate(node, state);
    }
};

} // ISMCTS

#endif // ISMCTS_MOSOLVER_H
