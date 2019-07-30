/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_CONFIG_H
#define ISMCTS_CONFIG_H

#include <memory>
#include <vector>

namespace ISMCTS
{

template<class> class Node;
template<class> struct EXP3;
template<class> struct UCB1;
template<class> struct RandomElement;

template<
    class Move,
    template<class> class SeqTree = UCB1, // Tree policies are functors of the form
    template<class> class SimTree = EXP3, // Node*(std::vector<Node*> const&)
    template<class> class Default = RandomElement // A functor of the form T&(std::vector<T> const&)
>
struct Config
{
    using DefaultPolicy = Default<Move>;
    using SeqTreePolicy = SeqTree<Move>;
    using SimTreePolicy = SimTree<Move>;

    using RootNode = std::shared_ptr<Node<Move>>;
    using ChildNode = typename Node<Move>::ChildPtr;
    using TreeList = std::vector<RootNode>;

    DefaultPolicy defaultPolicy;
    SeqTreePolicy seqTreePolicy;
    SimTreePolicy simTreePolicy;
};

} // ISMCTS

#endif // ISMCTS_CONFIG_H
