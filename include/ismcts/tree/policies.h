/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_TREEPOLICIES_H
#define ISMCTS_TREEPOLICIES_H

#include "nodetypes.h"
#include "../utility.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace ISMCTS
{

template<class Move>
struct EXP3
{
    using Node = EXPNode<Move>;

    Node *operator()(std::vector<Node*> const &nodes) const
    {
        auto const weights = probabilities(nodes);
        std::discrete_distribution<std::size_t> dist {weights.begin(), weights.end()};
        return nodes[dist(prng())];
    }

private:
    std::vector<double> static probabilities(std::vector<Node*> const &nodes)
    {
        auto const K = nodes.size();
        std::vector<double> probabilities(K);
        std::transform(nodes.begin(), nodes.end(), probabilities.begin(), [&](Node *node){
            auto const gamma = std::min(1., coefficient(K, node->visits()));
            auto const eta = gamma / K;
            auto const p = eta + (1 - gamma) / sumDifferences(nodes, node->score(), eta);
            node->setProbability(p);
            return p;
        });
        return probabilities;
    }

    double static coefficient(std::size_t K, double maxReward)
    {
        auto static const factor = std::expm1(1);
        return std::sqrt(K * std::log(K) / factor / maxReward);
    }

    double static sumDifferences(std::vector<Node*> const &nodes, double score, double eta)
    {
        return std::accumulate(nodes.begin(), nodes.end(), 0., [=](double sum, Node const *node){
            return sum + std::exp(eta * (node->score() - score));
        });
    }
};

template<class Move>
struct UCB1
{
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

private:
    double const m_exploration;
};

} // ISMCTS

#endif // ISMCTS_TREEPOLICIES_H
