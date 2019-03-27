/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_EXECUTION_H
#define ISMCTS_EXECUTION_H

namespace ISMCTS
{
// Unique classes representing parallelisation policies

/// All iterations are executed by a single thread on a single tree.
class Sequential {};

/// Each system thread executes a portion of the iterations on its own tree;
/// results of its root nodes are combined afterwards.
class RootParallel {};
}

#endif // ISMCTS_EXECUTION_H
