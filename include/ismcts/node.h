/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_NODE_H
#define ISMCTS_NODE_H

#include "game.h"
#include "treepolicy.h"

#include <memory>
#include <vector>
#include <algorithm>
#include <ostream>
#include <sstream>
#include <iomanip>

namespace ISMCTS
{

template<class Move>
class Node
{
public:
    using ChildPtr = std::shared_ptr<Node>;

    explicit Node(Node *parent = nullptr, const Move move = Move(), int playerJustMoved = -1)
        : m_parent{parent}
        , m_move{move}
        , m_playerJustMoved{playerJustMoved}
        , m_score{0}
        , m_visits{0}
        , m_available{1}
    {}

    Node *parent() const { return m_parent; }
    const std::vector<ChildPtr> &children() const { return m_children; }
    const Move &move() const { return m_move; }
    double score() const { return m_score; }
    unsigned int visits() const { return this->m_visits; }
    unsigned int available() const { return m_available; }

    Node *addChild(const Move &move, int player)
    {
        m_children.emplace_back(ChildPtr(new Node(this, move, player)));
        return m_children.back().get();
    }

    void update(const Game<Move> &terminalState)
    {
        ++m_visits;
        if (m_playerJustMoved != -1)
            m_score += terminalState.getResult(m_playerJustMoved);
    }

    std::vector<Move> untriedMoves(const std::vector<Move> &legalMoves) const
    {
        std::vector<Move> tried(m_children.size()), untried;
        std::transform(m_children.begin(), m_children.end(), tried.begin(), [](const ChildPtr &a){ return a->m_move; });
        untried.reserve(legalMoves.size());
        std::copy_if(legalMoves.begin(), legalMoves.end(), std::back_inserter(untried), [&](const Move &m){
            return std::find(tried.begin(), tried.end(), m) == tried.end();
        });
        return untried;
    }

    template<class TreePolicy>
    Node *select(const std::vector<Move> &legalMoves, const TreePolicy &policy) const
    {
        std::vector<Node*> legalChildren;
        legalChildren.reserve(m_children.size());
        for (const auto &node : m_children) {
            if (std::find(legalMoves.begin(), legalMoves.end(), node->m_move) != legalMoves.end()) {
                legalChildren.emplace_back(node.get());
                ++(node->m_available);
            }
        }
        return policy(legalChildren);
    }

    Node *findOrAddChild(const Move &move, int player)
    {
        const auto childPos = std::find_if(m_children.begin(), m_children.end(), [&](const ChildPtr &node){ return node->m_move == move; });
        return childPos < m_children.end() ? childPos->get() : addChild(move, player);
    }

    /// Returns a string containing information about this node, in the format
    /// "[M:(move) by (player), S/V/A: (score)/(visits)/(available)]".
    operator std::string() const
    {
        std::ostringstream oss;
        oss << "[M:" << m_move << " by " << m_playerJustMoved << ", S/V/A: ";
        oss << std::fixed << std::setprecision(1) << m_score << "/" << m_visits << "/" << m_available << "]";
        return oss.str();
    }

    /// Writes the structure and node statistics of the (sub)tree starting at
    /// the given node to the given output stream.
    friend std::ostream &operator<<(std::ostream &out, const Node &node)
    {
        return out << std::string(node);
    }

    std::string treeToString(unsigned int indent = 0) const
    {
        std::string s {indentSelf(indent)};
        for (auto &c : m_children)
            s += c->treeToString(indent + 1);
        return s;
    }

private:
    Node *m_parent;
    std::vector<ChildPtr> m_children;
    Move m_move;
    int m_playerJustMoved;
    double m_score;
    unsigned int m_visits;
    mutable unsigned int m_available;

    std::string indentSelf(unsigned int indent) const
    {
        std::string s;
        for (unsigned int i = 0; i < indent; ++i)
            s += "| ";
        return s + std::string(*this) + "\n";
    }
};

} // ISMCTS

#endif // ISMCTS_NODE_H
