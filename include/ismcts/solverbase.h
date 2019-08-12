/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_SOLVERBASE_H
#define ISMCTS_SOLVERBASE_H

#include "config.h"
#include "game.h"
#include "tree/node.h"

#include <memory>
#include <vector>

namespace ISMCTS
{

template<class Move, template<class> class... Ps>
class SolverBase
{
public:
    void setConfig(Ps<Move>... policies)
    {
        m_config = Config(policies...);
    }

protected:
    using Base = SolverBase;
    using Config = ISMCTS::Config<Move, Ps...>;
    using RootNode = std::shared_ptr<Node<Move>>;
    using ChildNode = typename Node<Move>::ChildPtr;
    using SeqNode = typename Config::SeqTreePolicy::Node;
    using SimNode = typename Config::SimTreePolicy::Node;

    void simulate(Game<Move> &state) const
    {
        auto const moves = state.validMoves();
        if (!moves.empty()) {
            state.doMove(m_config.defaultPolicy(moves));
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
            return node->selectChild(moves, m_config.simTreePolicy);
        else
            return node->selectChild(moves, m_config.seqTreePolicy);
    }

    RootNode static newRoot(Game<Move> const &state)
    {
        if (state.currentMoveSimultaneous())
            return std::make_shared<SimNode>();
        else
            return std::make_shared<SeqNode>();
    }

    ChildNode static newChild(Move const &move, Game<Move> const &state)
    {
        if (state.currentMoveSimultaneous())
            return std::make_unique<SimNode>(move, state.currentPlayer());
        else
            return std::make_unique<SeqNode>(move, state.currentPlayer());
    }

private:
    Config m_config;
};

} // ISMCTS

#endif // ISMCTS_SOLVERBASE_H
