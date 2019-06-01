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
    using NodePtr = typename Node<Move>::Ptr;
    using EXP3 = TreePolicy<EXPNode<Move>>;
    using UCB1 = TreePolicy<UCBNode<Move>>;
    using DefaultPolicy = std::function<Move const &(std::vector<Move> const &)>;

    virtual ~SolverBase() = default;

    virtual Move operator()(Game<Move> const &rootState) const = 0;

    void setEXPPolicy(EXP3 &&policy)
    {
        m_EXP3 = policy;
    }

    void setUCBPolicy(UCB1 &&policy)
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
        if (state.currentMoveSimultaneous()) {
            auto const children = node->template legalChildren<EXPNode<Move>>(moves);
            return m_EXP3(children);
        } else {
            auto const children = node->template legalChildren<UCBNode<Move>>(moves);
            return m_UCB1(children);
        }
    }

    NodePtr static newRoot(Game<Move> const &state)
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
    EXP3 m_EXP3 {};
    UCB1 m_UCB1 {};
    DefaultPolicy m_defaultPolicy {randomElement<Move>};
};

} // ISMCTS

#endif // ISMCTS_SOLVERBASE_H
