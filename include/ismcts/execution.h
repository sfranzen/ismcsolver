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
#include <chrono>
#include <thread>

namespace ISMCTS
{
/**
 * Execution policy base class
 *
 * Manages the information for the solver that is related to its execution. It
 * stores the number of threads the algorithm should be executed on and the
 * length of the search, either as a number of iterations or a length of time.
 */
class ExecutionPolicy
{
public:
    using Duration = std::chrono::duration<double>;

    explicit ExecutionPolicy(std::size_t iterationCount = 1000, unsigned int numThreads = 1)
        : m_numThreads{numThreads}
    {
        setIterationCount(iterationCount);
    }

    explicit ExecutionPolicy(Duration iterationTime, unsigned int numThreads = 1)
        : m_numThreads{numThreads}
    {
        setIterationTime(iterationTime);
    }

    virtual ~ExecutionPolicy() = default;

    /// Return number of iterations to perform per search
    std::size_t iterationCount() const
    {
        return m_iterCount;
    }

    /// Set policy to a fixed number of iterations
    void setIterationCount(std::size_t count)
    {
        m_iterCount = count / m_numThreads;
        m_iterTime = Duration::zero();
    }

    /// Return the time each search should take
    Duration iterationTime() const
    {
        return m_iterTime;
    }

    /// Set policy to a fixed length of time
    void setIterationTime(Duration time)
    {
        m_iterTime = time;
        m_iterCount = 0;
    }

    /// Return number of execution threads
    unsigned int numThreads() const
    {
        return m_numThreads;
    }

private:
    std::size_t m_iterCount;
    Duration m_iterTime;
    unsigned int m_numThreads;
};

/// All iterations are executed by a single thread on a single tree.
class Sequential : public ExecutionPolicy {
public:
    using ExecutionPolicy::ExecutionPolicy;

    template<class Move>
    static const Move &bestMove(const std::vector<Node<Move>> &trees)
    {
        // Sequential solvers use a vector with only one tree
        const auto &children = trees.front().children();
        using value = typename Node<Move>::Ptr;
        const auto &mostVisited = *std::max_element(children.begin(), children.end(), [](const value &a, const value &b){
            return a->visits() < b->visits();
        });
        return mostVisited->move();
    }
};

/// Each system thread executes a portion of the iterations on its own tree;
/// results of its root nodes are combined afterwards.
class RootParallel : public ExecutionPolicy {
public:
    RootParallel(std::size_t iterationCount)
        : ExecutionPolicy{iterationCount, std::thread::hardware_concurrency()}
    {}

    RootParallel(Duration iterationTime)
        : ExecutionPolicy{iterationTime, std::thread::hardware_concurrency()}
    {}

    /// Return best move from a number of trees holding results for the same
    /// player
    template<class Move>
    static const Move &bestMove(const std::vector<Node<Move>> &trees)
    {
        const auto results = compileVisitCounts(trees);
        using value = typename VisitMap<Move>::value_type;
        const auto &mostVisited = *std::max_element(results.begin(), results.end(), [](const value &a, const value &b){
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

} // ISMCTS

#endif // ISMCTS_EXECUTION_H
