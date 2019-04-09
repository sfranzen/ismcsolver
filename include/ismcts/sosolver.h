/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_SOSOLVER_H
#define ISMCTS_SOSOLVER_H

#include "solverbase.h"
#include "game.h"
#include "node.h"
#include "execution.h"

#include <memory>
#include <vector>
#include <thread>
#include <chrono>

namespace ISMCTS
{
/// Common base class for single observer solvers
template<class Move, class ExecutionPolicy>
class SOSolverBase : public SolverBase<Move>, public ExecutionPolicy
{
public:
    /**
     * Constructs a solver that will iterate the given number of times per
     * search operation.
     *
     * @param exploration Sets the algorithm's bias towards unexplored moves.
     *      It must be positive; the authors of the algorithm suggest 0.7 for
     *      a game that reports result values on the interval [0,1].
     */
    explicit SOSolverBase(std::size_t iterationCount = 1000, double exploration = 0.7)
        : SolverBase<Move>{exploration}
        , ExecutionPolicy{iterationCount}
    {}

    /**
     * Constructs a solver that will iterate for the given duration per search
     * operation.
     *
     * @param exploration Sets the algorithm's bias towards unexplored moves.
     *      It must be positive; the authors of the algorithm suggest 0.7 for
     *      a game that reports result values on the interval [0,1].
     */
    explicit SOSolverBase(std::chrono::duration<double> iterationTime, double exploration = 0.7)
        : SolverBase<Move>{exploration}
        , ExecutionPolicy{iterationTime}
    {}

protected:
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
        SOSolverBase::backPropagate(rootNode, *randomState);
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
        if (!SolverBase<Move>::selectNode(node, validMoves)) {
            node = node->ucbSelectChild(validMoves, this->m_exploration);
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
            node = node->addChild(move, state.currentPlayer());
            state.doMove(move);
        }
    }
};

// Partial specialisations for each execution policy
template<class Move, class ExecutionPolicy = Sequential>
class SOSolver {};

/// Sequential single observer solver
template<class Move>
class SOSolver<Move,Sequential> : public SOSolverBase<Move,Sequential>
{
public:
    using SOSolverBase<Move,Sequential>::SOSolverBase;

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        Node<Move> root;
        this->iterate(root, rootState);
        return this->bestMove(root);
    }
};

/// Single observer solver with root parallelisation
template<class Move>
class SOSolver<Move,RootParallel> : public SOSolverBase<Move,RootParallel>
{
public:
    using SOSolverBase<Move,RootParallel>::SOSolverBase;
    using RootParallel::numThreads;

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        std::vector<std::thread> threads(numThreads());
        std::vector<Node<Move>> trees(numThreads());

        for (std::size_t t = 0; t < numThreads(); ++t)
            threads[t] = std::thread(&SOSolver::iterate, this, std::ref(trees[t]), std::ref(rootState));
        for (auto &t : threads)
            t.join();

        return this->bestMove(trees);
    }
};

} // ISMCTS

#endif // ISMCTS_SOSOLVER_H
