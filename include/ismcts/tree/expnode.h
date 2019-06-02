/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_EXP3NODE_H
#define ISMCTS_EXP3NODE_H

#include "node.h"
#include "../game.h"
#include "../utility.h"

#include <atomic>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <sstream>

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
        if (this->parent())
            m_score += terminalState.getResult(this->player()) / m_probability;
    }
};

} // ISMCTS

#endif // ISMCTS_EXP3NODE_H
