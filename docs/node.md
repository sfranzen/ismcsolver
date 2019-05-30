# Nodes and trees
Although nodes are not primarily intended for use outside the algorithm, the `currentTrees` function provided by the solvers returns the `std::shared_ptr<Node<Move>>` instances holding the root nodes of the generated information trees. These may be used to query information about the tree using the functions below.

The structure of the tree is simple, with each node holding a raw pointer to its parent and a vector of `std::unique_ptr<Node>` storing zero or more children.

### Member types

| Type      | Definition                |
|:----------|:--------------------------|
|`Ptr`      |`std::shared_ptr<Node>`    |
|`ChildPtr` |`std::unique_ptr<Node>`    |

### Member functions
```cpp
Node *parent() const;
```
Returns a pointer to the parent node or `nullptr` if this node is a root.

---
```cpp
std::vector<ChildPtr> const &children() const;
```
Returns the children of this node.

---
```cpp
std::size_t depth() const;
```
Returns the depth of the node, which is the number of edges (steps) to the root node.

---
```cpp
std::size_t height() const;
```
Returns the height of the node, which is the length of the path to its most distant descendant.

---
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
std::ostream &operator<<(std::ostream &out, Node const &node);
```
Writes the string representation of a node to the given output stream.
