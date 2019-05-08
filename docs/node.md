---
layout: post
title: Node classes
---

# Node classes
Although nodes are not primarily intended for use outside the algorithm, the `currentTrees` function provided by the solvers returns the `std::shared_ptr<Node<Move>>` instances holding the root nodes of the generated information trees. These may be used to query information about the node using the functions below.

### Member functions
```cpp
virtual operator std::string() const;
```
Returns a string representation of the node. The output depends on the actual node type and is as follows:
* UCBNode: `[M:(move) by (player), V/S/A: (visits)/(score)/(available)]`;
* EXPNode: `[M:(move) by (player), V/S/P: (visits)/(score)/(probability)]`.

---
```cpp
std::string treeToString(unsigned int indent = 0) const;
```
Returns a string representation of the entire (sub)tree starting at the given node.

### Non-member functions
```cpp
std::ostream &operator<<(std::ostream &out, const Node &node);
```
Writes the string representation of a node to the given output stream.
