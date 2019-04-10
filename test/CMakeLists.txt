# Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
# This file is subject to the terms of the MIT License; see the LICENSE file in
# the root directory of this distribution.

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(Catch)

# Precompile Catch2 to limit compilation times
add_library(testmain OBJECT main.cpp)

set(game_src knockoutwhist.cpp)
set(testlibs testmain ismcsolver)

add_executable(benchmark benchmark.cpp ${game_src})
target_link_libraries(benchmark ${testlibs})

add_executable(gametest gametest.cpp ${game_src})
target_link_libraries(gametest ${testlibs})
catch_discover_tests(gametest)

add_executable(nodetest nodetest.cpp ${game_src})
target_link_libraries(nodetest ${testlibs})
catch_discover_tests(nodetest)

add_executable(solvertest solvertest.cpp ${game_src})
target_link_libraries(solvertest ${testlibs})
catch_discover_tests(solvertest)
