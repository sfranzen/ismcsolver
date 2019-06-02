/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_UCBNODE_H
#define ISMCTS_UCBNODE_H

#include "node.h"
#include "../game.h"
#include "../utility.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>

namespace ISMCTS
{

template<class Move>
class UCBNode : public Node<Move>
{
public:
    using Node<Move>::Node;

    void markAvailable()
    {
        ++m_available;
    }

    double ucbScore(double exploration) const
    {
        return m_score / this->visits() + exploration * std::sqrt(std::log(m_available.load()) / this->visits());
    }

    operator std::string() const override
    {
        std::ostringstream oss;
        oss << "[M:" << this->move() << " by " << this->player() << ", V/S/A: ";
        oss << std::fixed << std::setprecision(1) << this->visits() << "/" << m_score << "/" << m_available << "]";
        return oss.str();
    }

private:
    std::atomic<double> m_score {0};
    std::atomic_uint m_available {1};

    void updateData(Game<Move> const &terminalState) override
    {
        if (this->parent())
            m_score += terminalState.getResult(this->player());
    }
};

} // ISMCTS

#endif // ISMCTS_UCBNODE_H
