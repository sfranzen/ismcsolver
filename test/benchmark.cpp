/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */

#include <ismcts/sosolver.h>
#include <ismcts/mosolver.h>
#include "common/catch.hpp"
#include "common/knockoutwhist.h"
#include "common/card.h"
#include "common/phantommnkgame.h"
#include "common/goofspiel.h"
#include "common/utility.h"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <array>
#include <map>
#include <vector>

namespace
{

using namespace ISMCTS;
unsigned int constexpr numGames {100};
unsigned int constexpr iterationCount {1000};

// Test one "move generator" against another in a given game
class SolverTester
{
public:
    explicit SolverTester(unsigned games = 100)
        : m_numGames{games}
    {}

    template<class Game, class... Args>
    std::string run(Game const &game, Args &&... args)
    {
        m_p0Scores.resize(m_numGames);
        m_numCalls.fill(0);
        m_times.fill(Duration::zero());
        std::generate(m_p0Scores.begin(), m_p0Scores.end(), [&]{
            return playGame(game, std::forward<Args>(args)...);
        });
        return report();
    }

private:
    using Clock = std::chrono::high_resolution_clock;
    using Duration = ExecutionPolicy::Duration;

    unsigned m_numGames;
    std::vector<double> m_p0Scores;
    std::array<unsigned, 2> m_numCalls;
    std::array<Duration, 2> m_times;

    template<class Game, class G1>
    auto static getMove(Game const &game, G1 &&g1)
    {
        return g1(game);
    }

    template<class Game, class G1, class G2>
    auto static getMove(Game const &game, G1 &&g1, G2 &&g2)
    {
        return game.currentPlayer() == 0 ? g1(game) : g2(game);
    }

    template<class Game, class... Generators>
    double playGame(Game game, Generators &&... gs)
    {
        while (!game.validMoves().empty()) {
            auto const player = game.currentPlayer();
            if (player > 1) {
                doValidMove(game);
                continue;
            }
            auto const t0 = Clock::now();
            auto const move = getMove(game, std::forward<Generators>(gs)...);
            m_times[player] += Clock::now() - t0;
            ++m_numCalls[player];
            game.doMove(move);
        }
        return game.getResult(0);
    }

    std::string report() const
    {
        using namespace std;
        using namespace std::chrono;

        auto static const separator = std::string(77, '-') + "\n";
        std::ostringstream oss;

        std::map<double,unsigned> scoreCounts;
        for (auto score : m_p0Scores) {
            auto ret = scoreCounts.emplace(score, 1);
            if (!ret.second)
                ++ret.first->second;
        }

        auto const countWidth = int(std::floor(1 + std::log10(m_numGames)));

        oss << separator << "First player score stats after " << m_numGames << " games:\n";
        for (auto &pair : scoreCounts)
            oss << setw(3) << setprecision(2) << pair.first << ": " << setw(countWidth) << pair.second
                << " times (" << setprecision(3) << pair.second * 100. / m_numGames << "%)\n";

        for (unsigned p : {0, 1}) {
            double const time_ms = duration_cast<microseconds>(m_times[p]).count() / 1000.;
            oss << "Player " << p << " selected " << m_numCalls[p] << " moves in " << setprecision(4)
                << time_ms << " ms, average " << time_ms / m_numCalls[p] << " ms per move.\n";
        }
        oss << separator;
        return oss.str();
    }
};

template<class... Args>
void singleTest(SolverTester &tester, Args &&... args)
{
    WARN(tester.run(std::forward<Args>(args)...));
}

template<SOLVER_SIG class Solver, template<class> class... Opponent, class Game>
void testAllPolicies(Game &&game, unsigned games)
{
    using Move = MoveType<Game>;
    SolverTester tester {games};
    auto &&gameRef = std::forward<Game>(game);

    SECTION("Sequential")
        singleTest(tester, gameRef, Solver<Move, Sequential>{iterationCount}, Opponent<Move>{}...);
    SECTION("RootParallel")
        singleTest(tester, gameRef, Solver<Move, RootParallel>{iterationCount}, Opponent<Move>{}...);
    SECTION("TreeParallel")
        singleTest(tester, gameRef, Solver<Move, TreeParallel>{iterationCount}, Opponent<Move>{}...);
}

template<SOLVER_SIG class Solver>
void vsRandom(unsigned games)
{
    SECTION("Knockout Whist")
        testAllPolicies<Solver, RandomPlayer>(KnockoutWhist{2}, games);
    SECTION("Goofspiel")
        testAllPolicies<Solver, RandomPlayer>(Goofspiel{}, games);
}

template<SOLVER_SIG class Solver>
void vsSelf(unsigned games)
{
    SECTION("Knockout Whist")
        testAllPolicies<Solver>(KnockoutWhist{2}, games);
    SECTION("Goofspiel")
        testAllPolicies<Solver>(Goofspiel{}, games);
}

} // namespace

TEST_CASE("Versus random player", "[SOSolver][MOSolver]")
{
    SECTION("SOSolver")
        vsRandom<SOSolver>(numGames);
    SECTION("MOSolver")
        vsRandom<MOSolver>(numGames);
}

TEST_CASE("Versus self", "[SOSolver][MOSolver]")
{
    SECTION("SOSolver")
        vsSelf<SOSolver>(numGames);
    SECTION("MOSolver")
        vsSelf<MOSolver>(numGames);
}
