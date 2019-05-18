/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_EXECUTION_H
#define ISMCTS_EXECUTION_H

#include "tree/node.h"
#include "utility.h"

#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <thread>
#include <atomic>
#include <cmath>

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

    ExecutionPolicy(const ExecutionPolicy &) = delete;
    ExecutionPolicy &operator=(const ExecutionPolicy &) = delete;

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
            m_chunkSize = std::max(std::size_t(5), count * m_numThreads / 800);
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
    template<class Callable>
    std::thread launch(Callable &&f) const
    {
        if (m_iterCount > 0) {
            setCounter();
            return std::thread{[&,f]{
                executeFor(m_counter, m_chunkSize, f);
                m_isCounterSet = false;
            }};
        } else {
            return std::thread{[&,f]{ executeFor(m_iterTime, f); }};
        }
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
        m_counter = m_iterCount;
        m_isCounterSet = true;
    }
};

class Sequential : public ExecutionPolicy {
public:
    using ExecutionPolicy::ExecutionPolicy;

    template<class Move>
    static const Move &bestMove(const std::vector<NodePtr<Move>> &trees)
    {
        // Sequential solvers use a vector with only one tree
        const auto &children = trees.front()->children();
        const auto &mostVisited = *std::max_element(children.begin(), children.end(), [](const auto &a, const auto &b){
            return a->visits() < b->visits();
        });
        return mostVisited->move();
    }
};

class RootParallel : public ExecutionPolicy {
public:
    explicit RootParallel(std::size_t iterationCount = 1000)
        : ExecutionPolicy{iterationCount, std::thread::hardware_concurrency()}
    {}

    explicit RootParallel(Duration iterationTime)
        : ExecutionPolicy{iterationTime, std::thread::hardware_concurrency()}
    {}

    // Return best move from a number of trees holding results for the same
    // player
    template<class Move>
    static const Move &bestMove(const std::vector<NodePtr<Move>> &trees)
    {
        const auto results = compileVisitCounts<Move>(trees);
        const auto &mostVisited = *std::max_element(results.begin(), results.end(), [](const auto &a, const auto &b){
            return a.second < b.second;
        });
        return mostVisited.first;
    }

private:
    template<class Move>
    using VisitMap = std::map<Move, unsigned int>;

    template<class Move>
    static VisitMap<Move> compileVisitCounts(const std::vector<NodePtr<Move>> &trees)
    {
        VisitMap<Move> results;
        for (auto &node : trees.front()->children())
            results.emplace(node->move(), node->visits());
        for (auto t = trees.begin() + 1; t < trees.end(); ++t) {
            for (auto &node : (*t)->children()) {
                const auto result = results.emplace(node->move(), node->visits());
                if (!result.second)
                    (*result.first).second += node->visits();
            }
        }
        return results;
    }
};

} // ISMCTS

#endif // ISMCTS_EXECUTION_H
