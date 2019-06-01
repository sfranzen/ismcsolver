/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef ISMCTS_UTILITY_H
#define ISMCTS_UTILITY_H

#include <cstdint>
#include <cmath>
#include <atomic>
#include <utility>
#include <chrono>
#include <vector>
#include <random>

namespace ISMCTS
{

// Decrement the count by either the given chunk or count itself, whichever is
// lower, returning the decrement.
template<typename T>
inline T decrement(std::atomic<T> &count, T chunk)
{
    T old {count};
    do {
        chunk = std::min(old, chunk);
    } while(old > 0 && !count.compare_exchange_weak(old, old - chunk));
    return chunk;
}

inline double operator+=(std::atomic<double> &d, double other)
{
    double old {d};
    double newValue;
    do {
        newValue = old + other;
    } while (!d.compare_exchange_weak(old, newValue));
    return newValue;
}

template<class Callable, class... Args>
inline void executeFor(std::atomic_size_t &count, std::size_t chunk, Callable&& f, Args&&... args)
{
    while (count > 0)
        for (auto i = decrement(count, chunk); i > 0; --i)
            f(std::forward<Args>(args)...);
}

template<class Callable, class... Args>
inline void executeFor(std::chrono::duration<double> time, Callable&& f, Args&&... args)
{
    using clock = std::chrono::high_resolution_clock;
    while (time.count() > 0) {
        auto const start = clock::now();
        f(std::forward<Args>(args)...);
        time -= clock::now() - start;
    }
}

template<class T>
inline const T &randomElement(std::vector<T> const &v)
{
    std::mt19937 thread_local static prng {std::random_device{}()};
    std::uniform_int_distribution<std::size_t> randIdx {0, v.size() - 1};
    return v[randIdx(prng)];
}

} // ISMCTS

#endif // ISMCTS_UTILITY_H
