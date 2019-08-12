/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_NODE_H
#define ISMCTS_NODE_H

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <vector>

namespace ISMCTS
{

template<class Move>
struct Game;

template<class Move>
class Node
{
public:
    using ChildPtr = std::unique_ptr<Node>;

    explicit Node(Move const &move = {}, unsigned int player = 0)
        : m_move{move}
        , m_playerJustMoved{player}
    {}

    virtual ~Node() = default;

    Node *parent() const { return m_parent; }
    std::vector<ChildPtr> const &children() const { return m_children; }
    Move const &move() const { return m_move; }
    unsigned int player() const { return m_playerJustMoved; }
    unsigned int visits() const { return m_visits; }
    std::size_t depth() const { return depth(0); }
    std::size_t height() const { return height(0); }

    Node *addChild(ChildPtr child)
    {
        Lock lock {m_mutex};
        return addChildLocked(std::move(child));
    }

    template<class Generator>
    Node *findOrAddChild(Move const &move, Generator &&g)
    {
        Lock lock {m_mutex};
        auto const pos = std::find_if(m_children.begin(), m_children.end(), [&](auto const &c){ return c->m_move == move; });
        return pos < m_children.end() ? pos->get() : addChildLocked(g());
    }

    template<class Policy>
    Node *selectChild(std::vector<Move> const &legalMoves, Policy &policy) const
    {
        using Type = typename Policy::Node;
        std::vector<Type*> legalChildren;
        legalChildren.reserve(legalMoves.size());
        {
            Lock lock {m_mutex};
            for (auto &c : m_children) {
                if (std::any_of(legalMoves.begin(), legalMoves.end(), [&](Move const &move){ return c->m_move == move; }))
                    legalChildren.emplace_back(static_cast<Type*>(c.get()));
            }
        }
        return policy(legalChildren);
    }

    virtual void update(Game<Move> const &terminalState) final
    {
        if (this->parent()) {
            ++m_visits;
            updateData(terminalState);
        }
    }

    std::vector<Move> untriedMoves(std::vector<Move> const &legalMoves) const
    {
        std::vector<Move> untried;
        untried.reserve(legalMoves.size());
        Lock lock {m_mutex};
        std::copy_if(legalMoves.begin(), legalMoves.end(), std::back_inserter(untried), [&](auto const &m){
            return std::none_of(m_children.begin(), m_children.end(), [&](auto const &c){ return c->m_move == m; });
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

    friend std::ostream &operator<<(std::ostream &out, Node const &node)
    {
        return out << std::string(node);
    }

protected:
    using Lock = std::lock_guard<std::mutex>;
    std::mutex &mutex() const { return m_mutex; }

private:
    Node *m_parent = nullptr;
    std::mutex mutable m_mutex;
    std::vector<ChildPtr> m_children;
    Move const m_move;
    unsigned int const m_playerJustMoved;
    std::atomic_uint m_visits {0};

    // m_mutex assumed locked
    Node *addChildLocked(ChildPtr child)
    {
        child->m_parent = this;
        m_children.emplace_back(std::move(child));
        return m_children.back().get();
    }

    virtual void updateData(Game<Move> const &terminalState) = 0;

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
