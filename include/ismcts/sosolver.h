/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_SOSOLVER_H
#define ISMCTS_SOSOLVER_H

#include "solverbase.h"
#include "game.h"
#include "tree/nodetypes.h"
#include "tree/policies.h"
#include "execution.h"

#include <memory>
#include <vector>
#include <thread>
#include <chrono>

namespace ISMCTS
{
/**
 * Single observer solver class template.
 *
 * The single observer solvers implement the SO-ISMCTS algorithm, which searches
 * a single tree corresponding to the information sets of the current player in
 * the root game state. They should be used for games where the players can see
 * each other's moves, because the algorithm treats opponent moves as fully
 * observable.
 */
template<class Move, class _ExecutionPolicy = Sequential>
class SOSolver : public SolverBase<Move>, public _ExecutionPolicy
{
public:
    using _ExecutionPolicy::numThreads;
    using NodePtr = typename Node<Move>::Ptr;

    /// The search trees for the current player, one per thread
    using TreeList = std::vector<NodePtr>;

    /**
     * Constructs a solver that will iterate the given number of times per
     * search operation.
     *
     * @copydetails SolverBase::SolverBase
     */
    explicit SOSolver(std::size_t iterationCount = 1000)
        : SolverBase<Move>{}
        , _ExecutionPolicy{iterationCount}
    {}

    /**
     * Constructs a solver that will iterate for the given duration per search
     * operation.
     *
     * @copydetails SolverBase::SolverBase
     */
    explicit SOSolver(std::chrono::duration<double> iterationTime)
        : SolverBase<Move>{}
        , _ExecutionPolicy{iterationTime}
    {}

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        std::vector<std::thread> threads(numThreads());
        m_trees = TreeList(numThreads());
        for (auto &t : m_trees)
            t = SOSolver::newNode(rootState);

        for (std::size_t t = 0; t < numThreads(); ++t)
            threads[t] = std::thread(&SOSolver::iterate, this, std::ref(*m_trees[t]), std::ref(rootState));
        for (auto &t : threads)
            t.join();

        return SOSolver::template bestMove<Move>(m_trees);
    }

    /// Return the decision tree(s) resulting from the most recent call to
    /// operator(). The resulting vector contains one root node for each thread
    /// used for the search.
    TreeList &currentTrees() const
    {
        return m_trees;
    }

protected:
    mutable TreeList m_trees;

    void iterate(Node<Move> &root, const Game<Move> &state) const
    {
        const auto iterations = this->iterationCount();
        if (iterations > 0) {
            for (std::size_t i {0}; i < iterations; ++i)
                search(&root, state);
        } else {
            auto duration = this->iterationTime();
            while (duration.count() > 0) {
                using namespace std::chrono;
                const auto start = high_resolution_clock::now();
                search(&root, state);
                duration -= high_resolution_clock::now() - start;
            }
        }
    }

    /// Traverse a single sequence of moves
    void search(Node<Move> *rootNode, const Game<Move> &rootState) const
    {
        auto randomState = rootState.cloneAndRandomise(rootState.currentPlayer());
        select(rootNode, *randomState);
        expand(rootNode, *randomState);
        this->simulate(*randomState);
        SOSolver::backPropagate(rootNode, *randomState);
    }

    /**
     * Selection stage
     *
     * Descend the tree until a node is reached that has unexplored moves, or
     * is a terminal node (no more moves available).
     */
    void select(Node<Move> *&node, Game<Move> &state) const
    {
        const auto validMoves = state.validMoves();
        if (!SOSolver::selectNode(node, validMoves)) {
            node = this->selectChild(node, state, validMoves);
            state.doMove(node->move());
            select(node, state);
        }
    }

    /**
     * Expansion stage
     *
     * Choose a random unexplored move, add it to the children of the current
     * node and select this new node.
     */
    void expand(Node<Move> *&node, Game<Move> &state) const
    {
        const auto untriedMoves = node->untriedMoves(state.validMoves());
        if (!untriedMoves.empty()) {
            const auto move = this->randomMove(untriedMoves);
            node = SOSolver::addChild(node, state, move);
            state.doMove(move);
        }
    }
};

} // ISMCTS

#endif // ISMCTS_SOSOLVER_H
