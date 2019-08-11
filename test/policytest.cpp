/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */

#include "common/catch.hpp"
#include <ismcts/tree/policies.h>
#include <ismcts/game.h>
#include <ismcts/utility.h>

#include <algorithm>
#include <memory>
#include <vector>

namespace
{

using namespace ISMCTS;

// A mock game that constructs in a finished state with the specified reward
class TestGame : public Game<int>
{
public:
    explicit TestGame(double result = 0)
        : m_result{result}
    {}

    Clone cloneAndRandomise(unsigned int) const override
    {
        return std::make_unique<TestGame>();
    }

    unsigned int currentPlayer() const override
    {
        return 0;
    }

    void doMove(const int) override
    {}

    std::vector<int> validMoves() const override
    {
        return {};
    }

    double getResult(unsigned int) const override
    {
        return m_result;
    }

private:
    double m_result;
};

TestGame const win {1};
TestGame const loss {0};

} // namespace

TEMPLATE_PRODUCT_TEST_CASE("Common tree policy tests", "[UCB1][EXP3][D_UCB][SW_UCB]", (UCB1, EXP3, D_UCB, SW_UCB), int)
{
    using Node = typename TestType::Node;

    Node root;
    for (int i = 0; i < 10; ++i)
        root.addChild(std::make_unique<Node>());

    auto const &nodes = root.children();
    auto const firstNode = nodes.front().get();
    firstNode->update(win);
    std::for_each(nodes.begin() + 1, nodes.end(), [](auto &n){ n->update(loss); });

    TestType policy;
    auto const selection = root.selectChild({0}, policy);
    auto const expNode = dynamic_cast<EXPNode<int>*>(firstNode);
    if (expNode) {
        // EXP3 is non-deterministic; only check that the policy updates the
        // probability as expected. Initial probability equals 1, so the first
        // node has a score of 1 / 1 == 1
        REQUIRE(expNode->score() == 1);
        // The policy gives all nodes a new probability of exactly 0.1 in this
        // case, so another update to the first node will add 10 to its score
        expNode->update(win);
        REQUIRE(expNode->score() == 11);
    } else {
        // Other policies will select the first node because of its reward
        REQUIRE(selection == firstNode);
    }
}
