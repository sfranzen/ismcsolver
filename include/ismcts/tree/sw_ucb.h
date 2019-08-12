/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_SW_UCB_H
#define ISMCTS_SW_UCB_H

#include "ucb1.h"
#include "../utility.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <tuple>
#include <vector>

namespace ISMCTS
{

// Sliding Window UCB
template<class Move>
class SW_UCBNode : public UCBNode<Move>
{
public:
    using UCBNode<Move>::UCBNode;
    using ResultType = std::tuple<unsigned, double>;

    ResultType rewardSums(unsigned window) const
    {
        unsigned N {0};
        double X {0};
        auto const min = window > this->available() ? 0 : this->available() - window + 1;

        Lock lock {this->mutex()};
        auto s = std::find_if(m_trials.begin(), m_trials.end(), [=](unsigned t){ return t >= min; });
        for (auto r = m_results.begin() + (s - m_trials.begin()); r != m_results.end(); ++s, ++r) {
            N += *s;
            X += *r;
        }
        return std::make_tuple(N, X);
    }

private:
    using typename UCBNode<Move>::Lock;

    std::vector<double> m_results;
    std::vector<unsigned> m_trials;

    void updateData(Game<Move> const &terminalState) override
    {
        Lock lock {this->mutex()};
        m_trials.emplace_back(this->available());
        m_results.emplace_back(terminalState.getResult(this->player()));
    }
};

template<class Move>
class SW_UCB : public UCB1<Move>
{
public:
    using Node = SW_UCBNode<Move>;

    explicit SW_UCB(double exploration = 0.7, unsigned window = 500)
        : UCB1<Move>{exploration}
        , m_window{window}
    {}

    Node *operator()(std::vector<Node*> const &nodes) const
    {
        using Result = typename Node::ResultType;

        for (auto &node : nodes)
            node->markAvailable();

        std::vector<Result> results(nodes.size());
        std::transform(nodes.begin(), nodes.end(), results.begin(), [=](Node const *node){ return node->rewardSums(m_window); });

        auto const n = std::min(m_window, sum(results, [](Result const &r){ return std::get<0>(r); }));
        std::vector<double> ucbScores(nodes.size());
        std::transform(results.begin(), results.end(), ucbScores.begin(), [=](Result const &r){
            unsigned N;
            double X;
            std::tie(N, X) = r;
            return ucb(X / N, 2 * this->explorationConstant(), n, N);
        });

        auto const max = std::max_element(ucbScores.begin(), ucbScores.end()) - ucbScores.begin();
        return nodes[max];
    }

private:
    unsigned m_window;
};

} // ISMCTS

#endif // ISMCTS_SW_UCB_H
