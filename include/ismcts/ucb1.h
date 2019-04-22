/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef UCB1_H
#define UCB1_H

#include "treepolicy.h"
#include "node.h"
#include <algorithm>
#include <cmath>

namespace ISMCTS
{

template<class Move>
struct UCB1 : public TreePolicy<Move>
{
    explicit UCB1(double exploration = 0.7)
        : m_exploration{std::max(exploration, 0.0)}
    {}

    virtual Node<Move> *operator()(const std::vector<Node<Move>*> &legalChildren) const override
    {
        return *std::max_element(legalChildren.begin(), legalChildren.end(), [&](const Node<Move>* a, const Node<Move>* b){
            return ucbScore(a) < ucbScore(b);
        });
    }

private:
    double m_exploration;

    double ucbScore(const Node<Move> *node) const
    {
        return node->score() / node->visits() + m_exploration * std::sqrt(std::log(node->available()) / node->visits());
    }
};

} // ISMCTS

#endif // UCB1_H
