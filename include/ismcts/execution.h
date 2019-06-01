/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_EXECUTION_H
#define ISMCTS_EXECUTION_H

#include "tree/node.h"
#include "utility.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <future>
#include <memory>
#include <type_traits>
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
    using NodePtr = typename Node<Move>::Ptr;

    explicit ExecutionPolicy(std::size_t iterationCount = 1000, unsigned int numThreads = 1)
    {
        setThreadCount(numThreads);
        setIterationCount(iterationCount);
    }

    explicit ExecutionPolicy(Duration iterationTime, unsigned int numThreads = 1)
    {
        setThreadCount(numThreads);
        setIterationTime(iterationTime);
    }

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
    template<class Generator>
    auto static makeTrees(Generator &&g, unsigned int count)
    {
        std::vector<std::result_of_t<Generator()>> trees(count);
        std::generate(trees.begin(), trees.end(), g);
        return trees;
    }

    template<class Callable>
    std::future<void> launch(Callable &&f) const
    {
        if (m_iterCount > 0) {
            setCounter();
            return std::async(std::launch::async, [=]{
                executeFor(m_counter, m_chunkSize, std::move(f));
                m_isCounterSet = false;
            });
        } else {
            return std::async(std::launch::async, [=]{ executeFor(m_iterTime, std::move(f)); });
        }
    }

    template<class Move>
    Move static const &bestMove(std::vector<NodePtr<Move>> const &trees)
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
    unsigned int m_numThreads;
    unsigned int m_chunkSize {1};
    mutable std::atomic_size_t m_counter;
    mutable std::atomic_bool m_isCounterSet {false};

    void setThreadCount(unsigned int count)
    {
        m_numThreads = std::max(count, 1u);
    }

    void setCounter() const
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
        : ExecutionPolicy{iterationCount}
    {}

    explicit Sequential(Duration iterationTime)
        : ExecutionPolicy{iterationTime}
    {}

protected:
    template<class SearchOp, class TreeGenerator, class Game>
    auto execute(SearchOp &&search, TreeGenerator &&g, Game const &rootState) const
    {
        auto trees = makeTrees(g, 1);
        launch([&]{ search(trees[0], rootState); }).get();
        return trees;
    }
};

class TreeParallel : public ExecutionPolicy
{
public:
    explicit TreeParallel(std::size_t iterationCount = 1000)
        : ExecutionPolicy{iterationCount, std::thread::hardware_concurrency()}
    {}

    explicit TreeParallel(Duration iterationTime)
        : ExecutionPolicy{iterationTime, std::thread::hardware_concurrency()}
    {}

protected:
    template<class SearchOp, class TreeGenerator, class Game>
    auto execute(SearchOp &&search, TreeGenerator &&g, Game const &rootState) const
    {
        auto trees = makeTrees(g, 1);
        std::vector<std::future<void>> futures(numThreads());
        std::generate(futures.begin(), futures.end(), [&]{
            return launch([&]{ search(trees[0], rootState); });
        });
        for (auto &f : futures)
            f.get();
        return trees;
    }
};

class RootParallel : public TreeParallel
{
public:
    using TreeParallel::TreeParallel;

protected:
    template<class SearchOp, class TreeGenerator, class Game>
    auto execute(SearchOp &&search, TreeGenerator &&g, Game const &rootState) const
    {
        auto trees = makeTrees(g, numThreads());
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
    Move static bestMove(std::vector<NodePtr<Move>> const &trees)
    {
        auto const results = compileVisitCounts<Move>(trees);
        auto const &mostVisited = *std::max_element(results.begin(), results.end(), [](auto const &a, auto const &b){
            return a.second < b.second;
        });
        return mostVisited.first;
    }

private:
    template<class Move>
    using VisitMap = std::map<Move, unsigned int>;

    template<class Move>
    VisitMap<Move> static compileVisitCounts(std::vector<NodePtr<Move>> const &trees)
    {
        VisitMap<Move> results;
        for (auto &node : trees.front()->children())
            results.emplace(node->move(), node->visits());
        for (auto t = trees.begin() + 1; t < trees.end(); ++t) {
            for (auto &node : (*t)->children()) {
                auto const result = results.emplace(node->move(), node->visits());
                if (!result.second)
                    (*result.first).second += node->visits();
            }
        }
        return results;
    }
};

} // ISMCTS

#endif // ISMCTS_EXECUTION_H
