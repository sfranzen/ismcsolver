@mainpage

This documentation contains a detailed reference of the ismcsolver library,
automatically generated from the source code and included comments. Below is a
brief outline of the most important classes.

* ISMCTS::Game: Most importantly, the solver classes require a game to inherit
from this class;

* Each of the solver templates needs an ISMCTS::ExecutionPolicy as its second
parameter, two of which are currently available:
    * ISMCTS::Sequential (the default, can be omitted) for single-threaded
    solvers;
    * ISMCTS::RootParallel for multi-threaded solvers that search a separate
    tree on each thread.

* ISMCTS::SOSolver: class template for Single Observer solvers;

* ISMCTS::MOSolver: class template for Multiple Observer solvers.
