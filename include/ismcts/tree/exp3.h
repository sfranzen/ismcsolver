/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_EXP3_H
#define ISMCTS_EXP3_H

#include "node.h"
#include "../game.h"
#include "../utility.h"

#include <atomic>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <vector>
#include <iostream>

namespace ISMCTS
{

template<class Move>
class EXPNode : public Node<Move>
{
public:
    using Node<Move>::Node;

    void setProbability(double p) { m_probability = p; }
    double score() const { return m_score; }

    operator std::string() const override
    {
        std::ostringstream oss;
        oss << "[M:" << this->move() << " by " << this->player() << ", V/S/P: " << std::fixed << std::setprecision(1);
        oss << this->visits() << "/" << m_score << "/" << std::setprecision(2) << m_probability << "]";
        return oss.str();
    }

private:
    std::atomic<double> m_probability {1};
    std::atomic<double> m_score {0};

    void updateData(Game<Move> const &terminalState) override
    {
        m_score += terminalState.getResult(this->player()) / m_probability;
    }
};

template<class Move>
class EXP3
{
public:
    using Node = EXPNode<Move>;

    Node *operator()(std::vector<Node*> const &nodes) const
    {
        auto const weights = probabilities(nodes);
        std::discrete_distribution<std::size_t> dist {weights.begin(), weights.end()};
        return nodes[dist(prng())];
    }

private:
    /*
     * The probability calculation used here is a modified version of the
     * original, given as Algorithm 1 in "Evaluation and Analysis of the
     * Performance of the EXP3 Algorithm in Stochastic Environments" by Seldin
     * et al. (2012). This function uses the combined number of visits of the
     * given nodes as the trial counter variable t, because the number of trials
     * varies with the set of nodes.
     */
    std::vector<double> static probabilities(std::vector<Node*> const &nodes)
    {
        auto const K = nodes.size();
        auto const t = sum(nodes, [](Node const *node){ return node->visits(); });
        auto const e_t = epsilon(K, t);
        auto const e_tm1 = epsilon(K, t - 1);
        auto const expSum = sum(nodes, [=](Node const *node){ return std::exp(e_tm1 * node->score()); });

        std::vector<double> probabilities(K);
        std::transform(nodes.begin(), nodes.end(), probabilities.begin(), [=](Node *node){
            auto const p = e_t + (1 - K * e_t) * std::exp(e_tm1 * node->score()) / expSum;
            node->setProbability(p);
            return p;
        });

        return probabilities;
    }

    // The epsilon factor or exploration rate
    double static epsilon(std::size_t K, std::size_t t)
    {
        return std::min(1./K, std::sqrt(std::log(K) / K / t));
    }
};

} // ISMCTS

#endif // ISMCTS_EXP3_H
