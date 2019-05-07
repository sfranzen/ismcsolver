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

#include <random>
#include <vector>

namespace ISMCTS
{
/// Interface shared by all algorithms
template<class Move>
class SolverBase
{
public:
    using NodePtr = typename Node<Move>::Ptr;
    using EXP3 = TreePolicy<EXPNode<Move>>;
    using UCB1 = TreePolicy<UCBNode<Move>>;

    explicit SolverBase(EXP3 &&expPolicy = EXP3{}, UCB1 &&ucbPolicy = UCB1{})
        : m_EXP3{expPolicy}
        , m_UCB1{ucbPolicy}
    {}

    virtual ~SolverBase() = default;

    /**
     * Search operator.
     *
     * @return The most promising move from the given state.
     */
    virtual Move operator()(const Game<Move> &rootState) const = 0;

    void setEXPPolicy(EXP3 &&policy)
    {
        m_EXP3 = policy;
    }

    void setUCBPolicy(UCB1 &&policy)
    {
        m_UCB1 = policy;
    }

protected:
    /**
     * Simulation stage
     *
     * Continue performing random available moves from this state until the end
     * of the game.
     */
    void simulate(Game<Move> &state) const
    {
        const auto moves = state.validMoves();
        if (!moves.empty()) {
            state.doMove(this->randomMove(moves));
            simulate(state);
        }
    }

    /**
     * Backpropagation stage
     *
     * Traverse back to the root from the given node, updating the statistics of
     * all visited nodes with the result of the finished game state.
     */
    static void backPropagate(Node<Move> *node, const Game<Move> &state)
    {
        while (node) {
            node->update(state);
            node = node->parent();
        }
    }

    /// Whether to select this node given available moves
    static bool selectNode(const Node<Move> *node, const std::vector<Move> &moves)
    {
        return moves.empty() || !node->untriedMoves(moves).empty();
    }

    static Node<Move> *addChild(Node<Move> *node, const Game<Move> &state, const Move &move)
    {
        if (state.currentMoveSimultaneous())
            return node->addChild(new EXPNode<Move>{move, state.currentPlayer()});
        else
            return node->addChild(new UCBNode<Move>{move, state.currentPlayer()});
    }

    Node<Move> *selectChild(const Node<Move> *node, const Game<Move> &state, const std::vector<Move> &moves) const
    {
        if (state.currentMoveSimultaneous()) {
            const auto children = legalChildren<EXPNode<Move>>(node, moves);
            return this->m_EXP3(children);
        } else {
            const auto children = legalChildren<UCBNode<Move>>(node, moves);
            return this->m_UCB1(children);
        }
    }

    const Move &randomMove(const std::vector<Move> &moves) const
    {
        std::uniform_int_distribution<std::size_t> dist {0, moves.size() - 1};
        return moves[dist(prng())];
    }

    virtual std::mt19937 &prng() const
    {
        thread_local static std::mt19937 prng {std::random_device{}()};
        return prng;
    }

    static NodePtr newNode(const Game<Move> &state)
    {
        if (state.currentMoveSimultaneous())
            return std::make_shared<EXPNode<Move>>();
        else
            return std::make_shared<UCBNode<Move>>();
    }

private:
    EXP3 m_EXP3;
    UCB1 m_UCB1;

    template<class Type>
    static std::vector<Type*> legalChildren(const Node<Move> *node, const std::vector<Move> &legalMoves)
    {
        std::vector<Type*> legalChildren;
        legalChildren.reserve(legalMoves.size());
        for(auto &c : node->children()) {
            if (std::any_of(legalMoves.begin(), legalMoves.end(), [&](const Move &move){ return c->move() == move; }))
                legalChildren.emplace_back(static_cast<Type*>(c.get()));
        }
        return legalChildren;
    }
};

} // ISMCTS

#endif // ISMCTS_SOLVERBASE_H
