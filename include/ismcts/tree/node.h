/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_NODE_H
#define ISMCTS_NODE_H

#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include <ostream>

namespace ISMCTS
{

template<class Move>
struct Game;

template<class Move>
class Node
{
public:
    using Ptr = std::shared_ptr<Node>;
    using ChildPtr = std::unique_ptr<Node>;

    explicit Node(const Move &move = {}, unsigned int player = 0)
        : m_move{move}
        , m_playerJustMoved{player}
    {}

    virtual ~Node() = default;

    Node *parent() const { return m_parent; }
    const std::vector<ChildPtr> &children() const { return m_children; }
    const Move &move() const { return m_move; }
    unsigned int player() const { return m_playerJustMoved; }
    unsigned int visits() const { return m_visits; }
    std::size_t depth() const { return depth(0); }
    std::size_t height() const { return height(0); }

    Node *addChild(ChildPtr child)
    {
        child->m_parent = this;
        m_children.emplace_back(std::move(child));
        return m_children.back().get();
    }

    virtual void update(const Game<Move> &terminalState) final
    {
        ++m_visits;
        updateData(terminalState);
    }

    std::vector<Move> untriedMoves(const std::vector<Move> &legalMoves) const
    {
        std::vector<Move> untried;
        untried.reserve(legalMoves.size());
        std::copy_if(legalMoves.begin(), legalMoves.end(), std::back_inserter(untried), [&](const auto &m){
            return std::none_of(m_children.begin(), m_children.end(), [&](const auto &c){ return c->m_move == m; });
        });
        return untried;
    }

    virtual operator std::string() const { return ""; }

    std::string treeToString(unsigned int indent = 0) const
    {
        std::string s {indentSelf(indent)};
        for (auto &c : m_children)
            s += c->treeToString(indent + 1);
        return s;
    }

    friend std::ostream &operator<<(std::ostream &out, const Node &node)
    {
        return out << std::string(node);
    }

private:
    Node *m_parent = nullptr;
    std::vector<ChildPtr> m_children;
    const Move m_move;
    const unsigned int m_playerJustMoved;
    unsigned int m_visits {0};

    virtual void updateData(const Game<Move> &terminalState) = 0;

    std::string indentSelf(unsigned int indent) const
    {
        std::string s;
        for (unsigned int i = 0; i < indent; ++i)
            s += "| ";
        return s + std::string(*this) + "\n";
    }

    std::size_t depth(std::size_t start) const
    {
        return m_parent ? m_parent->depth(start + 1) : start;
    }

    std::size_t height(std::size_t start) const
    {
        if (m_children.empty())
            return start;
        std::vector<std::size_t> heights(m_children.size());
        std::transform(m_children.begin(), m_children.end(), heights.begin(), [=](auto &c){ return c->height(start + 1); });
        return *std::max_element(heights.begin(), heights.end());
    }
};

} // ISMCTS

#endif // ISMCTS_NODE_H
