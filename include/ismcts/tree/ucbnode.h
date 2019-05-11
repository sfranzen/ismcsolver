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

template<class Move>
class UCBNode : public Node<Move>
{
public:
    using Node<Move>::Node;

    void updateData(const Game<Move> &terminalState) override
    {
        if (this->parent())
            m_score += terminalState.getResult(this->player());
    }

    void markAvailable()
    {
        ++m_available;
    }

    double ucbScore(double exploration) const
    {
        return m_score / this->visits() + exploration * std::sqrt(std::log(m_available) / this->visits());
    }

    operator std::string() const override
    {
        std::ostringstream oss;
        oss << "[M:" << this->move() << " by " << this->player() << ", S/V/A: ";
        oss << std::fixed << std::setprecision(1) << this->visits() << "/" << m_score << "/" << m_available << "]";
        return oss.str();
    }

private:
    double m_score {0};
    unsigned int m_available {1};
};

} // ISMCTS

#endif // ISMCTS_UCBNODE_H
