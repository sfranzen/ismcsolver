/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_UCBNODE_H
#define ISMCTS_UCBNODE_H

#include "node.h"
#include "../game.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>

namespace ISMCTS
{

/// Node for the UCB1 tree policy
template<class Move>
class UCBNode : public Node<Move>
{
public:
    using Node<Move>::Node;

    void updateData(const Game<Move> &terminalState) override
    {
        if (this->m_parent)
            m_score += terminalState.getResult(this->m_playerJustMoved);
    }

    /// Mark this node as having been available for selection
    void markAvailable()
    {
        ++m_available;
    }

    /// The UCB score is used to rank children for selection; the exploration
    /// parameter increases the bias towards infrequently visited nodes.
    double ucbScore(double exploration) const
    {
        return m_score / this->m_visits + exploration * std::sqrt(std::log(m_available) / this->m_visits);
    }

    /// Returns a string containing information about this node, in the format
    /// "[M:(move) by (player), S/V/A: (score)/(visits)/(available)]".
    operator std::string() const override
    {
        std::ostringstream oss;
        oss << "[M:" << this->m_move << " by " << this->m_playerJustMoved << ", S/V/A: ";
        oss << std::fixed << std::setprecision(1) << m_score << "/" << this->m_visits << "/" << m_available << "]";
        return oss.str();
    }

private:
    double m_score {0};
    unsigned int m_available {1};
};

} // ISMCTS

#endif // ISMCTS_UCBNODE_H
