/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_UTILITY_H
#define ISMCTS_UTILITY_H

#include <atomic>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

namespace ISMCTS
{

// Decrement the count by either the given chunk or count itself, whichever is
// lower, returning the decrement.
template<typename T>
T decrement(std::atomic<T> &count, T chunk)
{
    T old {count};
    do {
        chunk = std::min(old, chunk);
    } while(old > 0 && !count.compare_exchange_weak(old, old - chunk));
    return chunk;
}

double inline operator+=(std::atomic<double> &d, double other)
{
    double old {d};
    double newValue;
    do {
        newValue = old + other;
    } while (!d.compare_exchange_weak(old, newValue));
    return newValue;
}

template<class Callable, class... Args>
void executeFor(std::atomic_size_t &count, std::size_t chunk, Callable&& f, Args&&... args)
{
    while (count > 0)
        for (auto i = decrement(count, chunk); i > 0; --i)
            f(std::forward<Args>(args)...);
}

template<class Callable, class... Args>
void executeFor(std::chrono::duration<double> time, Callable&& f, Args&&... args)
{
    using clock = std::chrono::high_resolution_clock;
    while (time.count() > 0) {
        auto const start = clock::now();
        f(std::forward<Args>(args)...);
        time -= clock::now() - start;
    }
}

template<class RNG = std::mt19937>
RNG &prng()
{
    std::mt19937 thread_local static prng {std::random_device{}()};
    return prng;
}

template<class T>
T const &randomElement(std::vector<T> const &v)
{
    std::uniform_int_distribution<std::size_t> randIdx {0, v.size() - 1};
    return v[randIdx(prng())];
}

template<class T>
struct RandomElement
{
    T const &operator()(std::vector<T> const &v) const
    {
        return randomElement(v);
    }
};

// Sum the results of operator op applied to each element of container c.
template<class C, class Op>
auto sum(C const &c, Op op)
{
    using T = typename C::value_type;
    using Ret = decltype(op(T{}));
    return std::accumulate(std::begin(c), std::end(c), Ret{0}, [=](Ret sum, T const &t){
        return sum + op(t);
    });
}

double inline ucb(double X, double C, double n, double N)
{
    return X + C * std::sqrt(std::log(n) / N);
}

} // ISMCTS

#endif // ISMCTS_UTILITY_H
