

# Solver class templates
The class templates listed below form the main point of interaction with this library. All that is generally needed to use the algorithms is a solver instance and a compatible game. The tree search algorithm is then invoked on any game state by calling the solver's `operator()`. Both solvers share the same interface, which is described further below.

## SOSolver
Defined in `<ismcts/sosolver.h>`
```cpp
template<
    class Move,
    class ExecutionPolicy = Sequential
> class SOSolver;
```
Single observer solvers implement the SO-ISMCTS algorithm, which searches a single tree corresponding to the information sets of the current player in the root game state. They should be used for games where the players can see each other's moves, because the algorithm treats opponent moves as fully observable.

## MOSolver
Defined in `<ismcts/mosolver.h>`
```cpp
template<
    class Move,
    class ExecutionPolicy = Sequential
> class MOSolver;
```
The multiple observer solvers implement the MO-ISMCTS algorithm, which builds a separate tree for each player and searches these simultaneously. This makes it applicable to games with partially observable moves, i.e. where players cannot always fully observe the other players' or teams' moves.

## Member types

| Type      | Definition                            |
|:----------|:--------------------------------------|
|`Duration` | `std::chrono::duration<double>`       |
|`EXP3`     | `ISMCTS::TreePolicy<EXPNode<Move>>`   |
|`UCB1`     | `ISMCTS::TreePolicy<UCBNode<Move>>`   |
|`NodePtr`  | `std::shared_ptr<Node<Move>>`         |
|*SOSolver:*|                                       |
|`TreeList` | `std::vector<NodePtr> `               |
|*MOsolver:*|                                       |
|`TreeMap`  | `std::map<unsigned int, NodePtr>`     |
|`TreeList` | `std::vector<TreeMap>`                |

## Member functions
### Constructors
In this section, the name `SolverType` refers to either of the two solvers.

```cpp
explicit SolverType(std::size_t iterationCount = 1000);
```
Constructs a solver that will iterate the given number of times per search operation.

---
```cpp
explicit SolverType(Duration iterationTime);
```
Constructs a solver that will iterate for the given duration per search operation.

### Search
```cpp
Move operator()(const Game<Move> &rootState) const;
```
Returns the most promising move from the given game state.

### Modifiers
```cpp
void setIterationCount(std::size_t count);
```
Sets the execution policy to use a fixed number of iterations in future searches.

---
```cpp
void setIterationTime(Duration time);
```
Sets the execution policy to use a fixed length of time in future searches.

---
```cpp
void setEXPPolicy(EXP3 &&policy);
```
Sets a new EXP3 tree policy, which is applied at nodes where the game has simultaneous moves.

---
```cpp
void setUCBPolicy(UCB1 &&policy);
```
Sets a new UCB1 tree policy, which is applied at nodes where the game uses sequential moves.

### Observers
```cpp
std::size_t iterationCount() const;
```
Returns the current iteration count, which is 0 if the time policy is used.

---
```cpp
Duration iterationTime() const;
```
Returns the current iteration time length, which is `Duration::zero()` if the iteration count policy is used.

---
```cpp
unsigned int numThreads() const;
```
Returns the number of threads used for execution of the algorithm, which is 1 for the `Sequential` policy and equal to `std::thread::hardware_concurrency()` for `RootParallel`.

---
```cpp
TreeList &currentTrees() const;
```
Returns the decision tree(s) resulting from the most recent call to operator(). The result is always a vector with one element per execution thread, but each solver uses a different element type as described under [Member types](#member-types).
