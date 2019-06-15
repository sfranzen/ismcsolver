/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_SOLVERBASE_H
#define ISMCTS_SOLVERBASE_H

#include "game.h"
#include "tree/nodetypes.h"
#include "tree/policies.h"
#include "utility.h"

#include <functional>
#include <memory>
#include <vector>

namespace ISMCTS
{

template<class Move, class _ExecutionPolicy>
class SolverBase : public _ExecutionPolicy
{
public:
    using _ExecutionPolicy::_ExecutionPolicy;
    using RootPtr = typename Node<Move>::RootPtr;
    using DefaultPolicy = std::function<Move const &(std::vector<Move> const &)>;

    void setUCBPolicy(UCB1<Move> &&policy)
    {
        m_UCB1 = policy;
    }

    void setDefaultPolicy(DefaultPolicy &&policy)
    {
        m_defaultPolicy = policy;
    }

protected:
    void simulate(Game<Move> &state) const
    {
        auto const moves = state.validMoves();
        if (!moves.empty()) {
            state.doMove(m_defaultPolicy(moves));
            simulate(state);
        }
    }

    void static backPropagate(Node<Move> *node, Game<Move> const &state)
    {
        while (node) {
            node->update(state);
            node = node->parent();
        }
    }

    bool static selectNode(Node<Move> const *node, std::vector<Move> const &moves)
    {
        return moves.empty() || !node->untriedMoves(moves).empty();
    }

    Node<Move> *selectChild(Node<Move> const *node, Game<Move> const &state, std::vector<Move> const &moves) const
    {
        if (state.currentMoveSimultaneous())
            return node->selectChild(moves, m_EXP3);
        else
            return node->selectChild(moves, m_UCB1);
    }

    RootPtr static newRoot(Game<Move> const &state)
    {
        if (state.currentMoveSimultaneous())
            return std::make_shared<EXPNode<Move>>();
        else
            return std::make_shared<UCBNode<Move>>();
    }

    std::unique_ptr<Node<Move>> static newChild(Move const &move, Game<Move> const &state)
    {
        if (state.currentMoveSimultaneous())
            return std::make_unique<EXPNode<Move>>(move, state.currentPlayer());
        else
            return std::make_unique<UCBNode<Move>>(move, state.currentPlayer());
    }

private:
    EXP3<Move> m_EXP3 {};
    UCB1<Move> m_UCB1 {};
    DefaultPolicy m_defaultPolicy {randomElement<Move>};
};

} // ISMCTS

#endif // ISMCTS_SOLVERBASE_H
