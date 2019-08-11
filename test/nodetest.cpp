/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */

#include <ismcts/tree/policies.h>
#include "common/catch.hpp"
#include "common/knockoutwhist.h"
#include "common/card.h"

#include <vector>
#include <numeric>

namespace
{
using namespace ISMCTS;
Card constexpr testMove {Card::Ace, Card::Spades};
unsigned int constexpr testPlayer {1};
}

TEMPLATE_TEST_CASE("Nodes are constructed properly", "[node]", UCBNode<Card>, EXPNode<Card>)
{
    TestType node;

    CHECK(node.parent() == nullptr);
    CHECK(node.children().empty());
    CHECK(node.move() == Card{});
    CHECK(node.player() == 0);
    CHECK(node.visits() == 0);
    CHECK(node.depth() == 0);
    CHECK(node.height() == 0);
    REQUIRE(std::string(node) != "");
}

TEMPLATE_TEST_CASE("Node::addChild works", "[node]", UCBNode<Card>, EXPNode<Card>)
{
    TestType root;
    Node<Card>* child {nullptr};

    CHECK_NOTHROW([&]{ child = root.addChild(std::make_unique<TestType>(testMove, testPlayer)); }());
    CHECK(child->parent() == &root);
    CHECK(child->move() == testMove);
    CHECK(child->player() == testPlayer);
    CHECK(child->depth() == 1);
    CHECK(root.height() == 1);
    REQUIRE(root.children().size() == 1);
    REQUIRE(child == root.children().front().get());
}

TEMPLATE_TEST_CASE("Node::update works", "[node]", UCBNode<Card>, EXPNode<Card>)
{
    TestType root;
    root.addChild(std::make_unique<TestType>(testMove, testPlayer));
    auto &node = root.children().front();
    KnockoutWhist game;

    CHECK_NOTHROW(node->update(game));
    REQUIRE(node->visits() == 1);
}

TEMPLATE_TEST_CASE("Node::untriedMoves returns expected values", "[node]", UCBNode<int>, EXPNode<int>)
{
    TestType root;
    std::vector<int> legalMoves(10);
    std::iota(legalMoves.begin(), legalMoves.end(), 0);

    SECTION("At a leaf node")
        REQUIRE(root.untriedMoves(legalMoves) == legalMoves);

    SECTION("After expanding available moves") {
        while (legalMoves.size() > 0) {
            auto move = --legalMoves.end();
            root.addChild(std::make_unique<TestType>(*move, testPlayer));
            legalMoves.erase(move);
            REQUIRE(root.untriedMoves(legalMoves) == legalMoves);
        }
    }

}
