/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_TREEPOLICY_H
#define ISMCTS_TREEPOLICY_H

#include <vector>

namespace ISMCTS
{

/**
 * Tree policy interface class
 *
 * ITreePolicy represents the abstract operation of a tree policy: choosing one
 * of the of the compatible child nodes.
 */
template<class Node>
struct ITreePolicy
{
    virtual Node *operator()(const std::vector<Node*> &nodes) const = 0;
};

template<class Node>
struct TreePolicy {};

} // ISMCTS

#endif // ISMCTS_TREEPOLICY_H
