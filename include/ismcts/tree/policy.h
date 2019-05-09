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

template<class Node>
struct ITreePolicy
{
    virtual Node *operator()(const std::vector<Node*> &nodes) const = 0;
};

template<class Node>
struct TreePolicy : public ITreePolicy<Node>
{};

} // ISMCTS

#endif // ISMCTS_TREEPOLICY_H
