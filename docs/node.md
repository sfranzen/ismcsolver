# Nodes and trees
Nodes are the building blocks of the solvers' information trees. They are intimately linked to the tree policies, which generally need specific statistics and calculations derived from the state of the game. Therefore each tree policy specifies the type of node that is to be instantiated for it, which should ultimately be derived from the `Node` class template described below.

Although nodes are not primarily intended for use outside the algorithm, the `currentTrees` function provided by the solvers returns `std::shared_ptr<Node>` instances holding the root nodes of the generated information trees. These may be used to query information about the tree using the functions below. The structure of the tree is simple, with each node holding a raw pointer to its parent and a vector of `std::unique_ptr<Node>` storing zero or more children.

As an example, consider the perfect information [m-n-k game], for which this algorithm is certainly not the best approach, although it does work. A game state with a winning move available to player 0 may look like this:
```
1  0  0
1  0 -1
-1  1 -1
```
where the moves are numbered 0 through 8 and -1 represents available moves. A tree search of 1000 iterations from this state might produce the following `treeToString` result:
```
[M:0 by -1, V/S/A: 1000/0.0/1]
| [M:6 by 0, V/S/A: 988/988.0/998]
| [M:8 by 0, V/S/A: 5/1.0/998]
| | [M:6 by 1, V/S/A: 3/3.0/3]
| | [M:5 by 1, V/S/A: 1/0.0/3]
| [M:5 by 0, V/S/A: 7/2.0/998]
| | [M:8 by 1, V/S/A: 1/0.0/5]
| | [M:6 by 1, V/S/A: 5/5.0/5]
```
indicating that move 6 was the most visited and would therefore be selected.

[m-n-k game]: https://github.com/sfranzen/ismcsolver/tree/master/test/common/mnkgame.cpp

## ISMCTS::Node
Defined in `<ismcts/tree/node.h>`
```cpp
template<class Move> class Node;
```
The `Node` is abstract and manages only the basic information required by the solver. In particular, a concrete node class template must implement its virtual abstract `updateData` method, which is responsible for updating the state of the node if it was selected during an iteration. The node must either inherit the constructor (`using Node<Move>::Node;`) or provide one with the same arguments that delegates to it. Furthermore, all of its methods must be thread-safe if it is to be used with TreeParallel solvers. C++ provides standard atomic operations for integral and pointer types with `std::atomic`, otherwise mutexes with lock guards are the recommended technique to keep data consistent under multi-threaded access.

### Member types

| Type      | Definition                |
|:----------|:--------------------------|
|`ChildPtr` |`std::unique_ptr<Node>`    |

### Public member functions
## Constructor
```cpp
explicit Node(Move const &move = {}, unsigned int player = 0);
```
Default constructor. Its default arguments are used only to create root nodes, whose move and player data are never used.

## Observers
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
Move const &move() const;
```
Returns the move stored at this node.

---
```cpp
unsigned int player() const;
```
Returns the number associated with the player of this move.

---
```cpp
unsigned int visits() const;
```
Returns the number of times this node was selected.

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

### Private member functions
```cpp
virtual void updateData(Game<Move> const &terminalState) = 0;
```
Update the data associated with this node using the result of the given game state, which can be retrieved by calling `terminalSate.getResult(this->player())`.

### Non-member functions
```cpp
std::ostream &operator<<(std::ostream &out, Node const &node);
```
Writes the string representation of a node to the given output stream.
