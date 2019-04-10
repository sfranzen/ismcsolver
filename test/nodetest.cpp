/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#include "catch.hpp"
#include <ismcts/node.h>
#include "knockoutwhist.h"
#include "card.h"

namespace
{
using namespace ISMCTS;
const Card testMove {Card::Ace, Card::Spades};
const int testPlayer {0};
}

TEST_CASE("Node instantiation", "[node]")
{
    SECTION("Default constructor") {
        Node<Card> node;

        CHECK(node.parent() == nullptr);
        CHECK(node.move() == Card{});
        CHECK(node.visits() == 0);
        REQUIRE(node.children().empty());
    }
    SECTION("Non-default constructor") {
        Node<Card> parent;
        Node<Card> node {&parent, testMove, testPlayer};

        CHECK(node.parent() == &parent);
        CHECK(node.move() == testMove);
        CHECK(node.visits() == 0);
        REQUIRE(node.children().empty());
    }
}

TEST_CASE("Adding children", "[node]")
{
    Node<Card> root;
    Node<Card>* child {nullptr};
    auto findOrAdd = [&](){ child = root.findOrAddChild(testMove, testPlayer); };

    SECTION("Using addChild()")
        CHECK_NOTHROW([&](){ child = root.addChild(testMove, testPlayer); }());

    SECTION("Using findOrAddChild()")
        CHECK_NOTHROW(findOrAdd());

    SECTION("Repeating findOrAddChild() with same input") {
        CHECK_NOTHROW(findOrAdd());
        CHECK_NOTHROW(findOrAdd());
    }

    REQUIRE(root.children().size() == 1);
    CHECK(child->move() == testMove);
    CHECK(child == root.children().front().get());
    REQUIRE(child->parent() == &root);
}

TEST_CASE("Updating node statistics", "[node]")
{
    Node<Card> node {nullptr, testMove, testPlayer};
    KnockoutWhist game;

    CHECK_NOTHROW(node.update(game));
    REQUIRE(node.visits() == 1);
}

TEST_CASE("Finding untried moves", "[node]")
{
    Node<int> root;
    std::vector<int> legalMoves(10);
    std::iota(legalMoves.begin(), legalMoves.end(), 0);

    SECTION("At a leaf node")
        REQUIRE(root.untriedMoves(legalMoves) == legalMoves);

    SECTION("After expanding available moves") {
        while (legalMoves.size() > 0) {
            auto move = --legalMoves.end();
            root.addChild(std::move(*move), testPlayer);
            legalMoves.erase(move);
            REQUIRE(root.untriedMoves(legalMoves) == legalMoves);
        }
    }

}
