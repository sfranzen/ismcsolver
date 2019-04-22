/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef TREEPOLICY_H
#define TREEPOLICY_H

namespace std
{
template<class T, class A>
class vector;
}

namespace ISMCTS
{

/**
 * Tree selection policy
 *
 * The tree policy is used at the selection stage on fully expanded nodes to
 * determine which of the previously visited and currently legal child nodes
 * should be chosen for this iteration.
 *
 * A custom selection policy could be provided by inheriting this class and
 * implementing the selection operator. The template parameter is the type of
 * node, which can be the current library node type, as is done in the
 * implementation of the default policy UCB1.
 */
template<class Node>
struct TreePolicy
{
    /// Selects one of the given pointers to legal child nodes.
    virtual Node *operator()(const std::vector<Node*> &legalChildren) const = 0;
};

} // ISMCTS

#endif // TREEPOLICY_H
