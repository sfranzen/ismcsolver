/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_MOSOLVER_H
#define ISMCTS_MOSOLVER_H

#include "solverbase.h"
#include "game.h"
#include "tree/nodetypes.h"
#include "tree/policies.h"
#include "execution.h"

#include <memory>
#include <vector>
#include <map>
#include <thread>
#include <chrono>

namespace ISMCTS
{

template<class Move, class _ExecutionPolicy = Sequential>
class MOSolver : public SolverBase<Move>, public _ExecutionPolicy
{
public:
    using _ExecutionPolicy::numThreads;
    using NodePtr = typename Node<Move>::Ptr;

    // The search trees for the current observer, one per player
    using TreeMap = std::map<unsigned int, NodePtr>;

    // The set of tree maps, one for each thread
    using TreeList = std::vector<TreeMap>;

explicit MOSolver(std::size_t iterationCount = 1000)
        : SolverBase<Move>{}
        , _ExecutionPolicy(iterationCount)
        , m_trees{numThreads()}
    {}

explicit MOSolver(std::chrono::duration<double> iterationTime)
        : SolverBase<Move>{}
        , _ExecutionPolicy(iterationTime)
        , m_trees{numThreads()}
    {}

    virtual Move operator()(const Game<Move> &rootState) const override
    {
        std::vector<std::thread> threads(numThreads());
        setupTrees(rootState);

        for (std::size_t t = 0; t < numThreads(); ++t)
            threads[t] = std::thread(&MOSolver::iterate, this, std::ref(m_trees[t]), std::ref(rootState));
        for (auto &t : threads)
            t.join();

        std::vector<NodePtr> currentPlayerTrees(numThreads());
        std::transform(m_trees.begin(), m_trees.end(), currentPlayerTrees.begin(), [&](TreeMap &set){
            return set[rootState.currentPlayer()];
        });
        return MOSolver::template bestMove<Move>(currentPlayerTrees);
    }

    TreeList &currentTrees() const
    {
        return m_trees;
    }

protected:
    using NodePtrMap = std::map<unsigned int, Node<Move>*>;

    mutable TreeList m_trees;

    void iterate(TreeMap &trees, const Game<Move> &state) const
    {
        const auto iterations = this->iterationCount();
        if (iterations > 0) {
            for (std::size_t i {0}; i < iterations; ++i)
                search(trees, state);
        } else {
            auto duration = this->iterationTime();
            while (duration.count() > 0) {
                using namespace std::chrono;
                const auto start = high_resolution_clock::now();
                search(trees, state);
                duration -= high_resolution_clock::now() - start;
            }
        }
    }

    void search(TreeMap &trees, const Game<Move> &rootState) const
    {
        NodePtrMap roots;
        for (auto &pair : trees)
            roots.emplace(pair.first, pair.second.get());
        auto randomState = rootState.cloneAndRandomise(rootState.currentPlayer());
        select(roots, *randomState);
        expand(roots, *randomState);
        this->simulate(*randomState);
        backPropagate(roots, *randomState);
    }

    void select(NodePtrMap &nodes, Game<Move> &state) const
    {
        const auto validMoves = state.validMoves();
        const auto player = state.currentPlayer();
        const auto &targetNode = nodes[player];
        if (!MOSolver::selectNode(targetNode, validMoves)) {
            const auto selection = this->selectChild(targetNode, state, validMoves);
            for (auto &node : nodes)
                node.second = MOSolver::findOrAddChild(node.second, state, selection->move());
            state.doMove(selection->move());
            select(nodes, state);
        }
    }

    static Node<Move> *findOrAddChild(Node<Move> *node, Game<Move> &state, const Move &move)
    {
        using Child = typename Node<Move>::ChildPtr;
        const auto &children = node->children();
        const auto pos = std::find_if(children.begin(), children.end(), [&](const Child &c){ return c->move() == move; });
        return pos < children.end() ? pos->get() : MOSolver::addChild(node, state, move);
    }

    void expand(NodePtrMap &nodes, Game<Move> &state) const
    {
        const auto player = state.currentPlayer();
        const auto untriedMoves = nodes[player]->untriedMoves(state.validMoves());
        if (!untriedMoves.empty()) {
            const auto move = this->randomMove(untriedMoves);
            for (auto &node : nodes)
                node.second = MOSolver::findOrAddChild(node.second, state, move);
            state.doMove(move);
        }
    }

    static void backPropagate(NodePtrMap &nodes, const Game<Move> &state)
    {
        for (auto node : nodes)
            SolverBase<Move>::backPropagate(node.second, state);
    }

    void setupTrees(const Game<Move> &rootState) const
    {
        const auto &state = dynamic_cast<const POMGame<Move>&>(rootState);

        for (auto &tree : m_trees) {
            tree.clear();
            for (auto player : state.players())
                tree.emplace(player, MOSolver::newNode(rootState));
        }
    }
};

} // ISMCTS

#endif // ISMCTS_MOSOLVER_H
