{% include mathjax.html %}
# Tree policies
The tree policy is the part of the algorithm concerned with evaluating and selecting child nodes at fully expanded nodes, in a way that maximises the expected rewards. This is also known as a *multi-armed bandit* (MAB) problem, from the analogy with a gambler searching for an optimal strategy for playing multiple "one-armed bandit" (slot) machines. Several approaches exist, two of which are employed by this library. Following the suggestion of the authors of ISMCTS, the [UCB1](#UCB1) policy is used for game states with sequential moves, and the [EXP3](#EXP3) policy for simultaneous moves, as indicated by the game implementation.

## UCB1
Defined in `<ismcts/tree/policies.h>`
```cpp
template<class Move>
struct UCB1;
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
Default constructor. The `exploration` parameter sets the value of the exploration constant *c* of the UCB function.

## EXP3
Defined in `<ismcts/tree/policies.h>`
```cpp
template<class Move>
struct EXP3;
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
