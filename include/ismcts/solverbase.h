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

#include <vector>
#include <functional>

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
    using DefaultPolicy = std::function<const Move &(const std::vector<Move> &)>;

    virtual ~SolverBase() = default;

    virtual Move operator()(const Game<Move> &rootState) const = 0;

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
        const auto moves = state.validMoves();
        if (!moves.empty()) {
            state.doMove(m_defaultPolicy(moves));
            simulate(state);
        }
    }

    static void backPropagate(Node<Move> *node, const Game<Move> &state)
    {
        while (node) {
            node->update(state);
            node = node->parent();
        }
    }

    static bool selectNode(const Node<Move> *node, const std::vector<Move> &moves)
    {
        return moves.empty() || !node->untriedMoves(moves).empty();
    }

    static Node<Move> *addChild(Node<Move> *node, const Game<Move> &state, const Move &move)
    {
        using ChildPtr = typename Node<Move>::ChildPtr;
        ChildPtr child;
        if (state.currentMoveSimultaneous())
            child = std::make_unique<EXPNode<Move>>(move, state.currentPlayer());
        else
            child = std::make_unique<UCBNode<Move>>(move, state.currentPlayer());
        return node->addChild(std::move(child));
    }

    Node<Move> *selectChild(const Node<Move> *node, const Game<Move> &state, const std::vector<Move> &moves) const
    {
        if (state.currentMoveSimultaneous()) {
            const auto children = legalChildren<EXPNode<Move>>(node, moves);
            return m_EXP3(children);
        } else {
            const auto children = legalChildren<UCBNode<Move>>(node, moves);
            return m_UCB1(children);
        }
    }

    static NodePtr newNode(const Game<Move> &state)
    {
        if (state.currentMoveSimultaneous())
            return std::make_shared<EXPNode<Move>>();
        else
            return std::make_shared<UCBNode<Move>>();
    }

private:
    EXP3 m_EXP3 {};
    UCB1 m_UCB1 {};
    DefaultPolicy m_defaultPolicy {randomElement<Move>};

    template<class Type>
    static std::vector<Type*> legalChildren(const Node<Move> *node, const std::vector<Move> &legalMoves)
    {
        std::vector<Type*> legalChildren;
        legalChildren.reserve(legalMoves.size());
        for(auto &c : node->children()) {
            if (std::any_of(legalMoves.begin(), legalMoves.end(), [&](const auto &move){ return c->move() == move; }))
                legalChildren.emplace_back(static_cast<Type*>(c.get()));
        }
        return legalChildren;
    }
};

} // ISMCTS

#endif // ISMCTS_SOLVERBASE_H
