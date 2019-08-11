# Configuration helper
The `Config` template creates the objects representing tree and default policies used by the solvers. Additionally it defines several type aliases used by other parts of the library.

## ISMCTS::Config
Defined in `<ismcts/config.h>`
```cpp
template<
    class Move,
    template<class> class SeqTree = UCB1,
    template<class> class SimTree = EXP3,
    template<class> class Default = RandomElement
> struct Config;
```
`SeqTree` and `SimTree` are the tree policies used for sequential and simultaneous moves, respectively. These should be functors with the signature `Node* (std::vector<Node*> const&)`, where `Node` is the node class associated with the policy; this is further described in [Tree Policies](treepolicy.md). `Default` specifies the default policy, which is used to simulate a non-terminal game state to completion upon creation of new nodes by the tree policy. This should be a functor with signature `Move const& (std::vector<Move> const&)`, returning one of the given moves.

Note: policy arguments need not be const or reference, but the return values must come from the vectors provided. Policy operators are never called with empty vectors by the solver.

### Constructor
```cpp
Config(SeqTree<Move> seq = SeqTree<Move>{},
       SimTree<Move> sim = SimTree<Move>{},
       Default<Move> d = Default<Move>{}
);
```
Default constructor.
