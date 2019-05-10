/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include <ismcts/tree/nodetypes.h>
#include "common/catch.hpp"
#include "common/knockoutwhist.h"
#include "common/card.h"
#include <numeric>

namespace
{
using namespace ISMCTS;
const Card testMove {Card::Ace, Card::Spades};
const unsigned int testPlayer {0};
}

TEMPLATE_TEST_CASE("Node instantiation", "[node]", UCBNode<Card>, EXPNode<Card>)
{
    SECTION("Default constructor") {
        TestType node;

        CHECK(node.parent() == nullptr);
        CHECK(node.move() == Card{});
        CHECK(node.visits() == 0);
        REQUIRE(node.children().empty());
    }
    SECTION("Non-default constructor") {
        TestType parent;
        auto node = parent.addChild(std::make_unique<TestType>(testMove, testPlayer));

        CHECK(node->parent() == &parent);
        CHECK(node->move() == testMove);
        CHECK(node->visits() == 0);
        REQUIRE(node->children().empty());
    }
}

TEMPLATE_TEST_CASE("Adding children", "[node]", UCBNode<Card>, EXPNode<Card>)
{
    TestType root;
    Node<Card>* child {nullptr};

    CHECK_NOTHROW([&]{ child = root.addChild(std::make_unique<TestType>(testMove, testPlayer)); }());
    REQUIRE(root.children().size() == 1);
    CHECK(child->move() == testMove);
    CHECK(child == root.children().front().get());
    REQUIRE(child->parent() == &root);
}

TEMPLATE_TEST_CASE("Updating node statistics", "[node]", UCBNode<Card>, EXPNode<Card>)
{
    TestType node {testMove, testPlayer};
    KnockoutWhist game;

    CHECK_NOTHROW(node.update(game));
    REQUIRE(node.visits() == 1);
}

TEMPLATE_TEST_CASE("Finding untried moves", "[node]", UCBNode<int>, EXPNode<int>)
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
