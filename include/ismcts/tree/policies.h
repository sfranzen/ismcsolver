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

namespace ISMCTS
{

/**
 * EXP3: Exponential weight algorithm for Exploitation and Exploration
 *
 * The EXP3 method uses and updates a non-uniform distribution of probabilities
 * over the nodes to select one, skewing this distribution over time in favour
 * of nodes with better expected rewards.
 */
template<class Move>
struct TreePolicy<EXPNode<Move>> : public ITreePolicy<EXPNode<Move>>
{
    using Node = EXPNode<Move>;

    Node *operator()(const std::vector<Node*> &nodes) const override
    {
        static thread_local std::mt19937 prng {std::random_device{}()};
        const auto weights = probabilities(nodes);
        std::discrete_distribution<std::size_t> dist {weights.begin(), weights.end()};
        return nodes[dist(prng)];
    }

private:
    /// Update, and return as a vector, all the selection probabilities of the
    /// given nodes.
    static std::vector<double> probabilities(const std::vector<Node*> &nodes)
    {
        const auto K = nodes.size();
        std::vector<double> probabilities(K);
        std::transform(nodes.begin(), nodes.end(), probabilities.begin(), [&](Node *node){
            const auto gamma = std::min(1., coefficient(K, node->visits()));
            const auto eta = gamma / K;
            const auto p = eta + (1 - gamma) / sumDifferences(nodes, node->score(), eta);
            node->setProbability(p);
            return p;
        });
        return probabilities;
    }

    /// Return the coefficient used in determining "gamma" for the selection
    /// probability.
    /// @param K The number of available choices.
    /// @param maxReward The maximum reward that could have been obtained from
    ///     selecting this node every time.
    static double coefficient(std::size_t K, double maxReward)
    {
        static const double factor { 1 / (std::exp(1) - 1) };
        return std::sqrt(K * std::log(K) * factor / maxReward);
    }

    /// Sum the exponential differences of the scores of the nodes with the
    /// given score, using the factor eta.
    static double sumDifferences(const std::vector<Node*> &nodes, double score, double eta)
    {
        return std::accumulate(nodes.begin(), nodes.end(), 0, [=](double sum, const Node *node){
            return sum + std::exp(eta * (node->score() - score));
        });
    }
};

/**
 * UCB: Upper Confidence Bound tree policy algorithm
 *
 * UCB uses a combination of the node statistics to select a node, using the
 * configurable exploration parameter to set the balance of exploitation of
 * known robust moves against exploration of infrequently visited nodes.
 */
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
    }

private:
    const double m_exploration;
};

} // ISMCTS

#endif // ISMCTS_TREEPOLICIES_H
