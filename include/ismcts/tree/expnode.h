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

    void setProbability(double p) { m_probability = p; }
    double score() const { return m_score; }

    void updateData(const Game<Move> &terminalState) override
    {
        if (this->m_parent)
            m_score += terminalState.getResult(this->m_playerJustMoved) / m_probability;
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
};

} // ISMCTS

#endif // ISMCTS_EXP3NODE_H
