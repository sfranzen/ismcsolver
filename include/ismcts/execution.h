/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_EXECUTION_H
#define ISMCTS_EXECUTION_H

#include "node.h"

#include <vector>
#include <map>

namespace ISMCTS
{
// Unique classes representing parallelisation policies

/// All iterations are executed by a single thread on a single tree.
class Sequential {
public:
    /// Return best move from a single tree
    template<class Move>
    static const Move &bestMove(const Node<Move> &tree)
    {
        const auto &children = tree.children();
        using value = typename Node<Move>::Ptr;
        const auto &mostVisited = *std::max_element(children.begin(), children.end(), [](const value &a, const value &b){
            return a->visits() < b->visits();
        });
        return mostVisited->move();
    }

};

/// Each system thread executes a portion of the iterations on its own tree;
/// results of its root nodes are combined afterwards.
class RootParallel {
public:
    /// Return best move from a number of trees holding results for the same
    /// player
    template<class Move>
    static const Move &bestMove(const std::vector<Node<Move>> &trees)
    {
        const auto results = compileVisitCounts(trees);
        using value = typename VisitMap<Move>::value_type;
        const auto &mostVisited = *max_element(results.begin(), results.end(), [](const value &a, const value &b){
            return a.second < b.second;
        });
        return mostVisited.first;
    }

private:
    template<class Move>
    using VisitMap = std::map<Move, unsigned int>;

    /// Map each unique move to its total number of visits
    template<class Move>
    static VisitMap<Move> compileVisitCounts(const std::vector<Node<Move>> &trees)
    {
        VisitMap<Move> results;
        for (auto &node : trees.front().children())
            results.emplace(node->move(), node->visits());
        for (auto t = trees.begin() + 1; t < trees.end(); ++t) {
            for (auto &node : t->children()) {
                const auto result = results.emplace(node->move(), node->visits());
                if (!result.second)
                    (*result.first).second += node->visits();
            }
        }
        return results;
    }
};
}

#endif // ISMCTS_EXECUTION_H
