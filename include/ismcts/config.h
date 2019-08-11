/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_CONFIG_H
#define ISMCTS_CONFIG_H

#include "tree/node.h"
#include "tree/exp3.h"
#include "tree/ucb1.h"
#include "utility.h"

#include <memory>
#include <vector>

namespace ISMCTS
{

template<
    class Move,
    template<class> class SeqTree = UCB1,
    template<class> class SimTree = EXP3,
    template<class> class Default = RandomElement
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

    Config(SeqTreePolicy seq = SeqTreePolicy{},
           SimTreePolicy sim = SimTreePolicy{},
           DefaultPolicy d = DefaultPolicy{}
    )
        : defaultPolicy{d}
        , seqTreePolicy{seq}
        , simTreePolicy{sim}
    {}
};

} // ISMCTS

#endif // ISMCTS_CONFIG_H
