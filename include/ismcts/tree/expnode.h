/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_EXP3NODE_H
#define ISMCTS_EXP3NODE_H

#include "node.h"
#include "../game.h"
#include <numeric>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace ISMCTS
{

/// Node for the EXP3 tree policy
template<class Move>
class EXPNode : public Node<Move>
{
public:
    using Node<Move>::Node;

    void updateData(const Game<Move> &terminalState) override
    {
        if (this->m_parent != nullptr)
            m_score += terminalState.getResult(this->m_playerJustMoved) / m_probability;
    }

    /// Update, and return as a vector, all the selection probabilities of the
    /// given nodes.
    static std::vector<double> probabilities(const std::vector<EXPNode*> &nodes)
    {
        const auto K = nodes.size();
        std::vector<double> probabilities(K);

        std::transform(nodes.begin(), nodes.end(), probabilities.begin(), [&](EXPNode *node){
            const auto gamma = std::min(1., coefficient(K, node->m_visits));
            const auto eta = gamma / K;
            const auto p = eta + (1 - gamma) / sumDifferences(nodes, node->m_score, eta);
            node->m_probability = p;
            return p;
        });
        return probabilities;
    }

    /// Returns a string containing information about this node, in the format
    /// "[M:(move) by (player), V/S/P: (visits)/(score)/(probability)]".
    operator std::string() const override
    {
        std::ostringstream oss;
        oss << "[M:" << this->m_move << " by " << this->m_playerJustMoved << ", V/S/P: " << std::fixed << std::setprecision(1);
        oss << this->m_visits << "/" << m_score << "/" << std::setprecision(2) << m_probability << "]";
        return oss.str();
    }

private:
    double m_probability {1};
    double m_score {0};

    /// Return the coefficient used in determining "gamma" for the selection
    /// probability.
    /// @param K The number of available choices.
    /// @param maxReward The maximum reward that could have been obtained from
    ///     selecting this node every time.
    static double coefficient(std::size_t K, double maxReward)
    {
        static const double factor { 1 / (std::exp(1) - 1) };
        return std::sqrt(K * std::log(K) * factor / maxReward );
    }

    /// Sum the exponential differences of the scores of the nodes with the
    /// given score, using the factor eta.
    static double sumDifferences(const std::vector<EXPNode*> &nodes, double score, double eta)
    {
        return std::accumulate(nodes.begin(), nodes.end(), 0, [=](double sum, const EXPNode *node){
            return sum + std::exp(eta * (node->m_score - score));
        });
    }
};

} // ISMCTS

#endif // ISMCTS_EXP3NODE_H
