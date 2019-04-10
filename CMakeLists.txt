# Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
# This file is subject to the terms of the MIT License; see the LICENSE file in
# the root directory of this distribution.

project(ismcsolver)

cmake_minimum_required(VERSION 3.12.0)
set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)
find_package(Doxygen)

set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include/ismcts)
set(headers
    ${include_dir}/game.h
    ${include_dir}/node.h
    ${include_dir}/solverbase.h
    ${include_dir}/sosolver.h
    ${include_dir}/mosolver.h
    ${include_dir}/execution.h
)

add_subdirectory(test)

add_library(ismcsolver INTERFACE)
target_link_libraries(ismcsolver INTERFACE Threads::Threads)
target_include_directories(ismcsolver INTERFACE include)
target_sources(ismcsolver INTERFACE ${headers})
install(FILES ${headers} DESTINATION include)

if(DOXYGEN_FOUND)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs)
    doxygen_add_docs(docs ${headers} docs/main.md)
endif()
