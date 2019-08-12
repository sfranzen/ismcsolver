/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_D_UCB_H
#define ISMCTS_D_UCB_H

#include "ucb1.h"
#include "../utility.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <tuple>
#include <vector>

namespace ISMCTS
{

// Helper class that precomputes positive integer powers of a given base
template<typename T>
class PositiveIntegerPowers
{
public:
    explicit PositiveIntegerPowers(std::size_t max)
        : m_powers(max + 1, T{1})
    {}

    T operator()(T base, std::size_t power)
    {
        if (base != m_base)
            fill(base);

        if (power >= m_powers.size())
            resize(power);

        return m_powers[power];
    }

private:
    std::vector<T> m_powers;
    T m_base {0};

    void fill(T base, std::size_t start = 0)
    {
        m_base = base;
        for (std::size_t i = start; i < m_powers.size(); ++i)
            m_powers[i] = std::pow(base, i);
    }

    void resize(std::size_t power)
    {
        auto const oldSize = m_powers.size();
        m_powers.resize(power * 2);
        fill(m_base, oldSize);
    }
};

// Discounted UCB
template<class Move>
class D_UCBNode : public UCBNode<Move>
{
public:
    using UCBNode<Move>::UCBNode;
    using ResultType = std::tuple<double,double>;

    ResultType discountSums(double gamma) const
    {
        double N {0}, X {0};
        Lock lock {this->mutex()};
        auto const t = this->available();
        auto r = m_results.begin();
        for (auto s = m_trials.begin(); s != m_trials.end(); ++s, ++r) {
            auto const discount = s_powers(gamma, t - *s);
            N += discount;
            X += discount * *r;
        }
        return std::make_tuple(N, X);
    }

private:
    using typename UCBNode<Move>::Lock;

    PositiveIntegerPowers<double> static s_powers;
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
PositiveIntegerPowers<double> D_UCBNode<Move>::s_powers {2000};

template<class Move>
struct D_UCB : public UCB1<Move>
{
    using Node = D_UCBNode<Move>;

    explicit D_UCB(double exploration = 0.7, double gamma = 0.8)
        : UCB1<Move>{exploration}
        , m_gamma{gamma}
    {}

    Node *operator()(std::vector<Node*> const &nodes) const
    {
        using Result = typename Node::ResultType;

        for (auto &node : nodes)
            node->markAvailable();

        std::vector<Result> results(nodes.size());
        std::transform(nodes.begin(), nodes.end(), results.begin(), [=](Node const *node){ return node->discountSums(m_gamma); });

        auto const n = sum(results, [](Result const &r){ return std::get<0>(r); });
        std::vector<double> ucbScores(nodes.size());
        std::transform(results.begin(), results.end(), ucbScores.begin(), [=](Result const &r){
            double N, X;
            std::tie(N, X) = r;
            return ucb(X / N, 2 * this->explorationConstant(), n, N);
        });

        auto const max = std::max_element(ucbScores.begin(), ucbScores.end()) - ucbScores.begin();
        return nodes[max];
    }

private:
    double m_gamma;
};

} // ISMCTS

#endif // ISMCTS_D_UCB_H
