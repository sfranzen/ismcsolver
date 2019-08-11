/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_UCB1_H
#define ISMCTS_UCB1_H

#include "node.h"
#include "../game.h"
#include "../utility.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>

namespace ISMCTS
{

template<class Move>
class UCBNode : public Node<Move>
{
public:
    using Node<Move>::Node;

    unsigned int available() const { return m_available; }

    void markAvailable()
    {
        ++m_available;
    }

    double ucbScore(double exploration) const
    {
        return ucb(m_score / this->visits(), exploration, m_available, this->visits());
    }

    operator std::string() const override
    {
        std::ostringstream oss;
        oss << "[M:" << this->move() << " by " << this->player() << ", V/S/A: ";
        oss << std::fixed << std::setprecision(1) << this->visits() << "/" << m_score << "/" << m_available << "]";
        return oss.str();
    }

private:
    std::atomic<double> m_score {0};
    std::atomic_uint m_available {1};

    void updateData(Game<Move> const &terminalState) override
    {
        m_score += terminalState.getResult(this->player());
    }
};

template<class Move>
class UCB1
{
public:
    using Node = UCBNode<Move>;

    explicit UCB1(double exploration = 0.7)
        : m_exploration{std::max(0., exploration)}
    {}

    Node *operator()(std::vector<Node*> const &nodes) const
    {
        for (auto &node : nodes)
            node->markAvailable();
        return *std::max_element(nodes.begin(), nodes.end(), [&](Node const *a, Node const *b){
            return a->ucbScore(m_exploration) < b->ucbScore(m_exploration);
        });
    }

    double explorationConstant() const { return m_exploration; }

private:
    double m_exploration;
};

} // ISMCTS

#endif // ISMCTS_UCB1_H
