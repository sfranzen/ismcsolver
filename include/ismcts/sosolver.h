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
#include "utility.h"

#include <memory>
#include <vector>
#include <future>

namespace ISMCTS
{

template<class Move, class _ExecutionPolicy = Sequential>
class SOSolver : public SolverBase<Move, _ExecutionPolicy>
{
public:
    using SolverBase<Move,_ExecutionPolicy>::SolverBase;
    using typename SolverBase<Move,_ExecutionPolicy>::NodePtr;
    using _ExecutionPolicy::numThreads;
    using TreeList = std::vector<NodePtr>;

    virtual Move operator()(Game<Move> const &rootState) const override
    {
        std::vector<std::future<void>> futures(numThreads());
        m_trees.resize(numThreads());
        std::generate(m_trees.begin(), m_trees.end(), [&]{ return SOSolver::newNode(rootState); });

        std::transform(m_trees.begin(), m_trees.end(), futures.begin(), [&](auto &node){
            return this->launch([&]{ search(node.get(), rootState); });
        });
        for (auto &f : futures)
            f.get();

        return SOSolver::template bestMove<Move>(m_trees);
    }

    TreeList currentTrees() const
    {
        return m_trees;
    }

protected:
    void search(Node<Move> *rootNode, Game<Move> const &rootState) const
    {
        auto randomState = rootState.cloneAndRandomise(rootState.currentPlayer());
        select(rootNode, *randomState);
        expand(rootNode, *randomState);
        this->simulate(*randomState);
        SOSolver::backPropagate(rootNode, *randomState);
    }

    void select(Node<Move> *&node, Game<Move> &state) const
    {
        auto const validMoves = state.validMoves();
        if (!SOSolver::selectNode(node, validMoves)) {
            node = this->selectChild(node, state, validMoves);
            state.doMove(node->move());
            select(node, state);
        }
    }

    void expand(Node<Move> *&node, Game<Move> &state) const
    {
        auto const untriedMoves = node->untriedMoves(state.validMoves());
        if (!untriedMoves.empty()) {
            auto const move = randomElement(untriedMoves);
            node = SOSolver::addChild(node, state, move);
            state.doMove(move);
        }
    }

private:
    mutable TreeList m_trees;
};

} // ISMCTS

#endif // ISMCTS_SOSOLVER_H
