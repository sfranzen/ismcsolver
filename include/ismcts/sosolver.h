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

template<class Move, class _ExecutionPolicy = Sequential>
class SOSolver : public SolverBase<Move>, public _ExecutionPolicy
{
public:
    using _ExecutionPolicy::_ExecutionPolicy;
    using _ExecutionPolicy::numThreads;
    using NodePtr = typename Node<Move>::Ptr;
    using TreeList = std::vector<NodePtr>;

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

    void search(Node<Move> *rootNode, const Game<Move> &rootState) const
    {
        auto randomState = rootState.cloneAndRandomise(rootState.currentPlayer());
        select(rootNode, *randomState);
        expand(rootNode, *randomState);
        this->simulate(*randomState);
        SOSolver::backPropagate(rootNode, *randomState);
    }

    void select(Node<Move> *&node, Game<Move> &state) const
    {
        const auto validMoves = state.validMoves();
        if (!SOSolver::selectNode(node, validMoves)) {
            node = this->selectChild(node, state, validMoves);
            state.doMove(node->move());
            select(node, state);
        }
    }

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
