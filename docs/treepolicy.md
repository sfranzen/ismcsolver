{% include mathjax.html %}
# Tree policies
The tree policy is the part of the algorithm concerned with evaluating and selecting child nodes at fully expanded nodes, in a way that maximises the expected rewards. This is also known as a *multi-armed bandit* (MAB) problem, from the analogy with a gambler searching for an optimal strategy for playing multiple "one-armed bandit" (slot) machines. Several approaches exist, some of which are provided with the library, but you can also write your own by defining a policy class template and possibly a custom Node.

A custom policy should have the following features:
* It has a single template parameter representing the type of move;
* It declares a public member `Node` representing the type of node it uses, e.g. `using Node = UCBNode<Move>;`;
* It has a public `operator()` that returns a pointer to this node type from a `std::vector` of such pointers, e.g. `Node *operator()(std::vector<Node*> const &nodes);`.

The policy class is free to modify these nodes; this is done for example by the UCB1 policy to mark the given nodes as having been available for selection.

Two tree policies can be specified for a solver, one for game states with sequential moves and one for states with simultaneous moves, as indicated by the game implementation. Following the suggestion of the authors of ISMCTS, the [UCB1](#UCB1) policy is the default for sequential moves, and [EXP3](#EXP3) for simultaneous moves. The default choice for the default policy selects moves uniformly at random.

Note that, although the policies listed below are defined in separate headers (along with their node types), they can also be included together using `#include <ismcts/tree/policies.h>`.

## UCB1
Defined in `<ismcts/tree/ucb1.h>`
```cpp
template<class Move> class UCB1;
```
UCB, meaning *Upper Confidence Bound*, chooses an action *a* from available actions *A* as follows:

$$
a = \mathrm{argmax}_{a \in A}\ \overline x(a) + c \sqrt\frac{\ln t}{n(a)},
$$

where $$\overline x(a)$$ is the expected reward from selecting a particular action, *t* is the number of times this action was available and $$n(a)$$ the number of times it was selected. The latter part of this expression represents the upper confidence bound, or how optimistic the virtual gambler is in the face of uncertain rewards. It has a free parameter *c* (greater than zero) that can be used to influence the bias towards infrequently exploited actions.

### Constructor
```cpp
explicit UCB1(double exploration = 0.7);
```
Default constructor. The `exploration` parameter (minimum value 0) sets the value of the exploration constant *c* of the UCB function.

## D_UCB
Defined in `<ismcts/tree/d_ucb.h>`
```cpp
template<class Move> class D_UCB;
```
D_UCB stands for *Discounted* UCB, which gradually discounts (lessens the influence of) previous rewards. This can be advantageous if the reward structure of the game changes over time, where UCB1 might dwell on choices that may no longer be good. It does this by reducing the apparent number of visits to the node as well as the average score using a factor $$\gamma^{t - s}$$, where *t* is the number of the current trial and *s* the trial numbers of past visits to the node.

### Constructor
```cpp
explicit D_UCB(double exploration = 0.7, double gamma = 0.8);
```
Default constructor. The `exploration` parameter is the same as in UCB1; the discount factor `gamma` (0, 1] determines the strength of the discount effect. Lower means stronger and 1 means no discount, identical to the UCB1 policy.

## SW_UCB
Defined in `<ismcts/tree/sw_ucb.h>`
```cpp
template<class Move> class SW_UCB;
```
SW_UCB stands for *Sliding Window* UCB and is similar to D_UCB, but takes a more radical approach. Reward values are processed as in UCB1, but only if they were obtained within a given number of trials (the window) in the past; older results are disregarded.

### Constructor
```cpp
explicit SW_UCB(double exploration = 0.7, std::size_t window = 500);
```
Default constructor. The `exploration` parameter is as in UCB1; the `window` is the number of past trials for which rewards are to be considered.

## EXP3
Defined in `<ismcts/tree/exp3.h>`
```cpp
template<class Move> class EXP3;
```
EXP3 stands for *Exponential weight algorithm for Exploitation and Exploration*. It is aimed at the more general *adversarial bandit* problem, where the gambler has an adversary capable of influencing the rewards of the available actions. The approach is to choose actions randomly according to a non-uniform probability distribution, obtained by assigning a probability to each action *a* as follows:

$$
p(a) = \frac{\gamma}{K} + \frac{1 - \gamma}{\sum_{b \in A} \exp\left[ \eta\big( s(b) - s(a) \big)\right]}.
$$

In this equation, $$0 \leq \gamma \leq 1$$ and $$0 \leq \eta < \frac1K$$ are factors determining the level of exploration, $$K = \vert A\vert$$ is the number of available actions and $$s(a)$$ denotes a (weighted) sum of the rewards from selecting *a* on previous trials *t*:

$$
s(a) = \sum_t \frac{x_t(a)}{p_t(a)}.
$$

The factors $$\gamma$$ and $$\eta$$ are set in accordance with the literature as

$$
\begin{aligned}
    \gamma(a) &= \min\left(1, \sqrt\frac{K \log K}{(\mathrm e - 1) n(a)} \right),\\
    \eta(a)   &= \frac{\gamma(a)}{K}.
\end{aligned}
$$

### Constructor
```cpp
EXP3();
```
Default constructor. The policy parameters cannot be set directly as they are automatically tuned by the algorithm.
