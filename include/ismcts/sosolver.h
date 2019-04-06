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
#include <map>
#include <chrono>

namespace ISMCTS
{
/// Common base class for single observer solvers
template<class Move>
class SOSolverBase : public SolverBase<Move>
{
public:
    using SolverBase<Move>::SolverBase;

protected:
    void iterate(Node<Move> &root, const Game<Move> &state) const
    {
        if (this->m_iterCount > 0) {
            for (std::size_t i {0}; i < this->m_iterCount; ++i)
                search(&root, state);
        } else {
            auto duration = this->m_iterTime;
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
        auto statePtr = randomState.get();
        select(rootNode, statePtr);
        expand(rootNode, statePtr);
        SolverBase<Move>::simulate(statePtr);
        SolverBase<Move>::backPropagate(rootNode, statePtr);
    }

    /**
     * Selection stage
     *
     * Descend the tree until a node is reached that has unexplored moves, or
     * is a terminal node (no more moves available).
     */
    void select(Node<Move> *&node, Game<Move> *state) const
    {
        const auto validMoves = state->validMoves();
        if (!SolverBase<Move>::selectNode(node, validMoves)) {
            node = node->ucbSelectChild(validMoves, this->m_exploration);
            state->doMove(node->move());
            select(node, state);
        }
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
            const auto move = SOSolverBase::randMove(untriedMoves);
            node = node->addChild(move, state->currentPlayer());
            state->doMove(move);
        }
    }
};

// Partial specialisations for each execution policy
template<class Move, class ExecutionPolicy = Sequential>
class SOSolver {};

/// Sequential single observer solver
template<class Move>
class SOSolver<Move, Sequential> : public SOSolverBase<Move>
{
public:
    using SOSolverBase<Move>::SOSolverBase;

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        Node<Move> root;
        this->iterate(root, rootState);
        return SOSolver::bestMove(root);
    }
};

/// Single observer solver with root parallelisation
template<class Move>
class SOSolver<Move, RootParallel> : public SOSolverBase<Move>
{
public:
    using SOSolverBase<Move>::SOSolverBase;

    explicit SOSolver(std::size_t iterationCount = 1000, double exploration = 0.7)
        : SOSolverBase<Move>{iterationCount / std::thread::hardware_concurrency(), exploration}
    {}

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        const auto numThreads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads(numThreads);
        std::vector<Node<Move>> trees(numThreads);

        for (std::size_t t = 0; t < numThreads; ++t)
            threads[t] = std::thread(&SOSolver::iterate, this, std::ref(trees[t]), std::ref(rootState));
        for (auto &t : threads)
            t.join();

        const auto results = compileVisitCounts(trees);
        using pair = typename VisitMap::value_type;
        const auto &mostVisited = *max_element(results.begin(), results.end(), [](const pair &a, const pair &b){
            return a.second < b.second;
        });
        return mostVisited.first;
    }

private:
    using VisitMap = std::map<Move, unsigned int>;

    // Map each unique move to its total number of visits
    static VisitMap compileVisitCounts(const std::vector<Node<Move>> &trees)
    {
        VisitMap results;
        for (auto &node : trees.front().children())
            results.emplace(node->move(), node->visits());
        for (auto t = trees.begin() + 1; t < trees.end(); ++t) {
            for (auto &node : t->children()) {
                const auto result = results.emplace(node->move(), node->visits());
                if (!result.second)
                    (*result.first).second += node->visits();
            }
        }
        return results;
    }
};

} // ISMCTS

#endif // ISMCTS_SOSOLVER_H
