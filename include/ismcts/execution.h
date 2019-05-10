/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_EXECUTION_H
#define ISMCTS_EXECUTION_H

#include "tree/node.h"

#include <vector>
#include <map>
#include <chrono>
#include <thread>

namespace ISMCTS
{

class ExecutionPolicy
{
public:
    using Duration = std::chrono::duration<double>;
    template<class Move>
    using NodePtr = typename Node<Move>::Ptr;

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

    std::size_t iterationCount() const
    {
        return m_iterCount;
    }

    void setIterationCount(std::size_t count)
    {
        m_iterCount = count / m_numThreads;
        m_iterTime = Duration::zero();
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

private:
    std::size_t m_iterCount;
    Duration m_iterTime;
    unsigned int m_numThreads;
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
    RootParallel(std::size_t iterationCount)
        : ExecutionPolicy{iterationCount, std::thread::hardware_concurrency()}
    {}

    RootParallel(Duration iterationTime)
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
