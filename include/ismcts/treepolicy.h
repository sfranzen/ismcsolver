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

template<class Move>
class Node;

/**
 * Tree selection policy class template
 *
 * The tree policy is used at the selection stage on fully expanded nodes to
 * determine which of the previously visited and currently legal child nodes
 * should be chosen for this iteration.
 */
template<class Move>
struct TreePolicy
{
    /// Select one of the given legalChildren pointers
    virtual Node<Move> *operator()(const std::vector<Node<Move>*> &legalChildren) const = 0;
};

} // ISMCTS

#endif // TREEPOLICY_H
