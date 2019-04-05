/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_MOSOLVER_H
#define ISMCTS_MOSOLVER_H

#include "solverbase.h"
#include "game.h"
#include "node.h"
#include "execution.h"

#include <memory>
#include <vector>
#include <chrono>

namespace ISMCTS
{
/// Common base class for multiple observer solvers
template<class Move>
class MOSolverBase : public SolverBase<Move>
{
public:
    explicit MOSolverBase(std::size_t numPlayers, std::size_t iterationCount = 1000, double exploration = 0.7)
        : SolverBase<Move>{iterationCount, exploration}
        , m_numPlayers{numPlayers}
    {}

    explicit MOSolverBase(std::size_t numPlayers, std::chrono::duration<double> time, double exploration = 0.7)
        : SolverBase<Move>{time, exploration}
        , m_numPlayers{numPlayers}
    {}

protected:
    using RootList = std::vector<Node<Move>>;
    using NodePtrList = std::vector<Node<Move>*>;

    std::size_t m_numPlayers;

    void iterate(RootList &trees, const Game<Move> &state) const
    {
        if (this->m_iterCount > 0) {
            for (std::size_t i {0}; i < this->m_iterCount; ++i)
                search(trees, state);
        } else {
            auto duration = this->m_iterTime;
            while (duration.count() > 0) {
                using namespace std::chrono;
                const auto start = high_resolution_clock::now();
                search(trees, state);
                duration -= high_resolution_clock::now() - start;
            }
        }
    }

    /// Traverse a single sequence of moves
    void search(RootList &trees, const Game<Move> &rootState) const
    {
        NodePtrList roots(trees.size());
        std::transform(trees.begin(), trees.end(), roots.begin(), [](Node<Move> &n){ return &n; });
        auto randomState = rootState.cloneAndRandomise(rootState.currentPlayer());
        auto statePtr = randomState.get();
        select(roots, statePtr);
        expand(roots, statePtr);
        SolverBase<Move>::simulate(statePtr);
        backPropagate(roots, statePtr);
    }

    /**
     * Selection stage
     *
     * Descend all trees until a node is reached that has unexplored moves, or
     * is a terminal node (no more moves available).
     */
    void select(NodePtrList &nodes, Game<Move> *state) const
    {
        const auto validMoves = state->validMoves();
        const auto player = state->currentPlayer();
        const auto &targetNode = nodes[player];
        if (!SolverBase<Move>::selectNode(targetNode, validMoves)) {
            const auto selection = targetNode->ucbSelectChild(validMoves, this->m_exploration);
            for (auto &node : nodes)
                node = node->findOrAddChild(selection->move(), player);
            state->doMove(selection->move());
            select(nodes, state);
        }
    }

    /**
     * Expansion stage
     *
     * Choose a random unexplored move, add it to the children of all current
     * nodes and select these new nodes.
     */
    static void expand(NodePtrList &nodes, Game<Move> *state)
    {
        const auto player = state->currentPlayer();
        const auto untriedMoves = nodes[player]->untriedMoves(state->validMoves());
        if (!untriedMoves.empty()) {
            const auto move = SolverBase<Move>::randMove(untriedMoves);
            for (auto &node : nodes)
                node = node->findOrAddChild(move, player);
            state->doMove(move);
        }
    }

    static void backPropagate(NodePtrList &nodes, const Game<Move> *state)
    {
        for (auto node : nodes)
            SolverBase<Move>::backPropagate(node, state);
    }
};

// Partial specialisations for each execution policy
template<class Move, class ExecutionPolicy = Sequential>
class MOSolver {};

/// Sequential multiple observer solver
template<class Move>
class MOSolver<Move, Sequential> : public MOSolverBase<Move>
{
public:
    using MOSolverBase<Move>::MOSolverBase;

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        std::vector<Node<Move>> trees(this->m_numPlayers);

        this->iterate(trees, rootState);

        const auto &rootList = trees.at(rootState.currentPlayer()).children();
        using value = typename Node<Move>::Ptr;
        const auto &mostVisited = *std::max_element(rootList.begin(), rootList.end(), [](const value &a, const value &b){
            return a->visits() < b->visits();
        });
        return mostVisited->move();
    }
};

/// Multiple observer solver with root parallelisation
template<class Move>
class MOSolver<Move, RootParallel> : public MOSolverBase<Move>
{
public:
    using MOSolverBase<Move>::MOSolverBase;

    explicit MOSolver(std::size_t numPlayers, std::size_t iterMax = 1000, double exploration = 0.7)
        : MOSolverBase<Move>{numPlayers, iterMax / std::thread::hardware_concurrency(), exploration}
    {}

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        const auto numThreads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads(numThreads);
        std::vector<RootList> treeSets(numThreads);
        for (auto &set : treeSets)
            set = RootList(this->m_numPlayers);

        for (std::size_t t = 0; t < numThreads; ++t)
            threads[t] = std::thread(&MOSolver::iterate, this, std::ref(treeSets[t]), std::ref(rootState));
        for (auto &t : threads)
            t.join();

        // Gather results for the current player from each thread
        NodePtrList currentPlayerTrees(numThreads);
        std::transform(treeSets.begin(), treeSets.end(), currentPlayerTrees.begin(), [&](RootList &set){
            return &set.at(rootState.currentPlayer());
        });
        const auto results = compileVisitCounts(currentPlayerTrees);
        using pair = typename VisitMap::value_type;
        const auto &mostVisited = *max_element(results.begin(), results.end(), [](const pair &a, const pair &b){
            return a.second < b.second;
        });
        return mostVisited.first;
    }

protected:
    using typename MOSolverBase<Move>::RootList;
    using typename MOSolverBase<Move>::NodePtrList;

private:
    using VisitMap = std::map<Move, unsigned int>;

    // Map each unique move to its total number of visits
    static VisitMap compileVisitCounts(const NodePtrList &trees)
    {
        VisitMap results;
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

}

#endif // ISMCTS_MOSOLVER_H