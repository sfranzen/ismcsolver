/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_EXECUTION_H
#define ISMCTS_EXECUTION_H

#include "config.h"
#include "tree/node.h"
#include "utility.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <future>
#include <memory>
#include <utility>
#include <vector>
#include <map>

namespace ISMCTS
{

class ExecutionPolicy
{
public:
    using Duration = std::chrono::duration<double>;

    template<class Move>
    using TreeList = typename Config<Move>::TreeList;

    ExecutionPolicy(ExecutionPolicy const &) = delete;
    ExecutionPolicy &operator=(ExecutionPolicy const &) = delete;

    std::size_t iterationCount() const
    {
        return m_iterCount;
    }

    void setIterationCount(std::size_t count)
    {
        m_iterCount = count;
        m_iterTime = Duration::zero();
        if (m_numThreads == 1)
            m_chunkSize = count;
        else
            // Increase chunk size with number of threads and iterations to
            // reduce contention on m_counter
            m_chunkSize = std::max(std::size_t(1), count * m_numThreads / 1000);
    }

    Duration iterationTime() const
    {
        return m_iterTime;
    }

    void setIterationTime(Duration time)
    {
        m_iterTime = time;
        m_iterCount = 0;
    }

    unsigned int numThreads() const
    {
        return m_numThreads;
    }

protected:
    unsigned int static hwThreadCount() { return std::thread::hardware_concurrency(); }

    ExecutionPolicy(unsigned int numThreads, unsigned int numTrees)
        : m_numThreads{validateCount(numThreads)}
        , m_numTrees{validateCount(numTrees)}
    {}

    ExecutionPolicy(std::size_t iterationCount, unsigned int numThreads, unsigned int numTrees)
        : ExecutionPolicy{numThreads, numTrees}
    {
        setIterationCount(iterationCount);
    }

    ExecutionPolicy(Duration iterationTime, unsigned int numThreads, unsigned int numTrees)
        : ExecutionPolicy{numThreads, numTrees}
    {
        setIterationTime(iterationTime);
    }

    template<class Generator>
    auto makeTrees(Generator &&g) const
    {
        std::vector<decltype(g())> trees(m_numTrees);
        std::generate(trees.begin(), trees.end(), g);
        return trees;
    }

    // Works for both Sequential and TreeParallel
    template<class SearchOp, class TreeGenerator, class Game>
    auto execute(SearchOp &&search, TreeGenerator &&g, Game const &rootState)
    {
        auto trees = makeTrees(g);
        std::vector<std::future<void>> futures(m_numThreads);
        std::generate(futures.begin(), futures.end(), [&]{
            return launch([&]{ search(trees[0], rootState); });
        });
        for (auto &f : futures)
            f.get();
        return trees;
    }

    template<class Callable>
    std::future<void> launch(Callable &&f)
    {
        if (m_iterCount > 0) {
            setCounter();
            return std::async(std::launch::async, [=]{
                executeFor(m_counter, m_chunkSize, f);
                m_isCounterSet = false;
            });
        } else {
            return std::async(std::launch::async, [=]{ executeFor(m_iterTime, f); });
        }
    }

    template<class Move>
    Move static const &bestMove(TreeList<Move> const &trees)
    {
        // Sequential/TreeParallel solvers use a vector with only one tree
        auto const &children = trees.front()->children();
        auto const &mostVisited = *std::max_element(children.begin(), children.end(), [](auto const &a, auto const &b){
            return a->visits() < b->visits();
        });
        return mostVisited->move();
    }

private:
    std::size_t m_iterCount;
    Duration m_iterTime;
    unsigned int const m_numThreads;
    unsigned int const m_numTrees;
    unsigned int m_chunkSize {1};
    std::atomic_size_t m_counter;
    std::atomic_bool m_isCounterSet {false};

    unsigned int static validateCount(unsigned int count) { return std::max(count, 1u); }

    void setCounter()
    {
        if (m_isCounterSet)
            return;
        m_isCounterSet = true;
        m_counter = m_iterCount;
    }
};

class Sequential : public ExecutionPolicy
{
public:
    explicit Sequential(std::size_t iterationCount = 1000)
        : ExecutionPolicy{iterationCount, 1, 1}
    {}

    explicit Sequential(Duration iterationTime)
        : ExecutionPolicy{iterationTime, 1, 1}
    {}
};

class TreeParallel : public ExecutionPolicy
{
public:
    explicit TreeParallel(std::size_t iterationCount = 1000, unsigned int numThreads = hwThreadCount())
        : ExecutionPolicy{iterationCount, numThreads, 1}
    {}

    explicit TreeParallel(Duration iterationTime, unsigned int numThreads = hwThreadCount())
        : ExecutionPolicy{iterationTime, numThreads, 1}
    {}
};

class RootParallel : public ExecutionPolicy
{
public:
    explicit RootParallel(std::size_t iterationCount = 1000, unsigned int numThreads = hwThreadCount())
        : ExecutionPolicy{iterationCount, numThreads, numThreads}
    {}

    explicit RootParallel(Duration iterationTime, unsigned int numThreads = hwThreadCount())
        : ExecutionPolicy{iterationTime, numThreads, numThreads}
    {}

protected:
    template<class SearchOp, class TreeGenerator, class Game>
    auto execute(SearchOp &&search, TreeGenerator &&g, Game const &rootState)
    {
        auto trees = makeTrees(g);
        std::vector<std::future<void>> futures(numThreads());
        std::transform(trees.begin(), trees.end(), futures.begin(), [&](auto &tree){
            return launch([&]{ search(tree, rootState); });
        });
        for (auto &f : futures)
            f.get();
        return trees;
    }

    // Return best move from a number of trees holding results for the same
    // player
    template<class Move>
    Move static bestMove(TreeList<Move> const &trees)
    {
        auto const results = compileVisitCounts<Move>(trees);
        auto const &mostVisited = *std::max_element(results.begin(), results.end(), [](auto const &a, auto const &b){
            return a.second < b.second;
        });
        return mostVisited.first;
    }

private:
    template<class Move>
    auto static compileVisitCounts(TreeList<Move> const &trees)
    {
        std::map<Move, unsigned int> results;
        for (auto &tree : trees) {
            for (auto &node : tree->children()) {
                auto const result = results.emplace(node->move(), node->visits());
                if (!result.second)
                    result.first->second += node->visits();
            }
        }
        return results;
    }
};

} // ISMCTS

#endif // ISMCTS_EXECUTION_H
