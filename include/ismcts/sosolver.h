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

namespace ISMCTS
{

template<class Move, class _ExecutionPolicy = Sequential>
class SOSolver : public SolverBase<Move, _ExecutionPolicy>
{
public:
    using SolverBase<Move,_ExecutionPolicy>::SolverBase;
    using typename SolverBase<Move,_ExecutionPolicy>::RootPtr;
    using TreeList = typename _ExecutionPolicy::template TreeList<Move>;

    Move operator()(Game<Move> const &rootState) const
    {
        auto treeGenerator = [&rootState]{ return SOSolver::newRoot(rootState); };
        auto treeSearch = [this](RootPtr &root, Game<Move> const &state){ search(root.get(), state); };
        m_trees = SOSolver::execute(treeSearch, treeGenerator, rootState);
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
            auto const &move = randomElement(untriedMoves);
            node = node->addChild(SOSolver::newChild(move, state));
            state.doMove(move);
        }
    }

private:
    mutable TreeList m_trees;
};

} // ISMCTS

#endif // ISMCTS_SOSOLVER_H
