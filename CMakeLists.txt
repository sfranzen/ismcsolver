# Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
# This file is subject to the terms of the MIT License; see the LICENSE file in
# the root directory of this distribution.

project(ismcsolver)

cmake_minimum_required(VERSION 3.1.0)

find_package(Threads REQUIRED)

set(headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/game.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/node.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/solverbase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sosolver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/execution.h
)

add_subdirectory(test)

add_library(ismcsolver INTERFACE)
target_link_libraries(ismcsolver INTERFACE Threads::Threads)
target_include_directories(ismcsolver INTERFACE include)
target_sources(ismcsolver INTERFACE ${headers})
install(FILES ${headers} DESTINATION include)
