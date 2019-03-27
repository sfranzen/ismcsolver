/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_NODE_H
#define ISMCTS_NODE_H

#include "game.h"

#include <memory>
#include <vector>
#include <algorithm>
#include <math.h>

namespace ISMCTS
{

template<class Move> class Node
{
public:
    using Ptr = std::unique_ptr<Node>;
    using List = std::vector<Ptr>;

    explicit Node(Node *parent = nullptr, const Move move = Move(), int playerJustMoved = -1)
        : m_parent{parent}
        , m_move{move}
        , m_playerJustMoved{playerJustMoved}
        , m_score{0}
        , m_visits{0}
        , m_available{1}
    {}

    const Move &move() const { return m_move; }
    Node *parent() const { return m_parent; }
    unsigned int visits() const { return m_visits; }
    const List &children() const { return m_children; }

    Node *addChild(Move move, int player)
    {
        m_children.emplace_back(Ptr(new Node(this, move, player)));
        return m_children.back().get();
    }

    void update(const Game<Move> *terminalState)
    {
        ++m_visits;
        if (m_playerJustMoved != -1)
            m_score += terminalState->getResult(m_playerJustMoved);
    }

    std::vector<Move> untriedMoves(const std::vector<Move> &legalMoves) const
    {
        std::vector<Move> tried(m_children.size()), untried;
        std::transform(m_children.begin(), m_children.end(), tried.begin(), [](const Ptr &a){ return a->m_move; });
        untried.reserve(legalMoves.size());
        std::copy_if(legalMoves.begin(), legalMoves.end(), std::back_inserter(untried), [&](const Move &m){
            return std::find(tried.begin(), tried.end(), m) == tried.end();
        });
        return untried;
    }

    Node *ucbSelectChild(const std::vector<Move> &legalMoves, double exploration) const
    {
        std::vector<Node*> legalChildren;
        legalChildren.reserve(m_children.size());
        for (const auto &node : m_children) {
            if (std::find(legalMoves.begin(), legalMoves.end(), node->m_move) != legalMoves.end()) {
                legalChildren.emplace_back(node.get());
                ++(node->m_available);
            }
        }
        static const auto compareUCB = [=](const Node *a, const Node *b){
            return a->ucbScore(exploration) < b->ucbScore(exploration);
        };
        return *std::max_element(legalChildren.begin(), legalChildren.end(), compareUCB);
    }

    Node *selectChild(const Move &move) const
    {
        const auto child = std::find_if(m_children.begin(), m_children.end(), [&](const Ptr &c){ return c->m_move == move; });
        return child < m_children.end() ? child->get() : nullptr;
    }

private:
    Node *m_parent;
    List m_children;
    Move m_move;
    int m_playerJustMoved;
    double m_score;
    unsigned int m_visits;
    mutable unsigned int m_available;

    double ucbScore(double exploration) const
    {
        return m_score / double(m_visits) + exploration * std::sqrt(std::log(m_available) / m_visits);
    }
};

} // ISMCTS

#endif // ISMCTS_NODE_H
