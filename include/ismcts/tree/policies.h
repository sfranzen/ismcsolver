/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_TREEPOLICIES_H
#define ISMCTS_TREEPOLICIES_H

#include "policy.h"
#include "nodetypes.h"
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>

namespace ISMCTS
{

template<class Move>
struct TreePolicy<EXPNode<Move>> : public ITreePolicy<EXPNode<Move>>
{
    using Node = EXPNode<Move>;

    Node *operator()(const std::vector<Node*> &nodes) const override
    {
        static thread_local std::mt19937 prng {std::random_device{}()};
        const auto weights = Node::probabilities(nodes);
        std::discrete_distribution<std::size_t> dist {weights.begin(), weights.end()};
        return nodes[dist(prng)];
    }
};

template<class Move>
struct TreePolicy<UCBNode<Move>> : public ITreePolicy<UCBNode<Move>>
{
    using Node = UCBNode<Move>;

    explicit TreePolicy(double exploration = 0.7)
    : m_exploration{std::max(0., exploration)}
    {}

    Node *operator()(const std::vector<Node*> &nodes) const override
    {
        for (auto &node : nodes)
            node->markAvailable();
        return *std::max_element(nodes.begin(), nodes.end(), [&](const Node *a, const Node *b){
            return a->ucbScore(m_exploration) < b->ucbScore(m_exploration);
        });
    };

private:
    const double m_exploration;
};

} // ISMCTS

#endif // ISMCTS_TREEPOLICIES_H
