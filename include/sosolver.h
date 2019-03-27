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

namespace ISMCTS
{
/// Common base class for single observer solvers
template<class Move>
class SOSolverBase : public SolverBase<Move>
{
public:
    using SolverBase<Move>::SolverBase;

protected:
    void iterate(std::size_t number, Node<Move> &root, const Game<Move> &state) const
    {
        for (std::size_t i {0}; i < number; ++i)
            search(&root, state);
    }

    /// Traverse a single sequence of moves
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
            node = node->ucbSelectChild(validMoves, this->m_exploration);
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
            const auto move = SOSolverBase::randMove(untriedMoves);
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
            state->doMove(SOSolverBase::randMove(moves));
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
        this->iterate(this->m_iterMax, root, rootState);
        const auto &rootList = root.children();
        const auto &mostVisited = *std::max_element(rootList.begin(), rootList.end());
        return mostVisited->move();
    }
};

/// Single observer solver with root parallelisation
template<class Move>
class SOSolver<Move, RootParallel> : public SOSolverBase<Move>
{
public:
    using SOSolverBase<Move>::SOSolverBase;

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        const auto numThreads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads(numThreads);
        std::vector<Node<Move>> trees(numThreads);

        for (std::size_t t = 0; t < numThreads; ++t)
            threads[t] = std::thread(&SOSolver::iterate, this, this->m_iterMax / numThreads, std::ref(trees[t]), std::ref(rootState));
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
        for (auto &root : trees) {
            if (root == trees.front())
                for (auto &node : root.children())
                    results.emplace(node->move(), node->visits());
            else
                for (auto &node : root.children()) {
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
