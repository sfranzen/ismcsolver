/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_SW_UCB_H
#define ISMCTS_SW_UCB_H

#include "d_ucb.h"
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

        Lock lock {m_mutex};
        auto s = std::find_if(m_trials.begin(), m_trials.end(), [=](unsigned t){ return t >= min; });
        for (auto r = m_results.begin() + (s - m_trials.begin()); r != m_results.end(); ++s, ++r) {
            N += *s;
            X += *r;
        }
        return {N,X};
    }

private:
    using typename UCBNode<Move>::Lock;

    std::mutex mutable m_mutex;
    std::vector<double> m_results;
    std::vector<unsigned> m_trials;

    void updateData(Game<Move> const &terminalState) override
    {
        Lock lock {m_mutex};
        m_trials.emplace_back(this->available());
        m_results.emplace_back(terminalState.getResult(this->player()));
    }
};

template<class Move>
struct SW_UCB
{
    using Node = SW_UCBNode<Move>;

    explicit SW_UCB(unsigned window = 1000, double exploration = 0.7)
    : m_window{window}
    , m_exploration{exploration}
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
            return X / N + m_exploration * std::sqrt(std::log(n) / N);
        });

        auto const max = std::max_element(ucbScores.begin(), ucbScores.end()) - ucbScores.begin();
        return nodes[max];
    }

private:
    unsigned m_window;
    double m_exploration;
};

} // ISMCTS

#endif // ISMCTS_SW_UCB_H

