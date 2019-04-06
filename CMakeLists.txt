# Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
# This file is subject to the terms of the MIT License; see the LICENSE file in
# the root directory of this distribution.

project(ismcsolver)

cmake_minimum_required(VERSION 3.1.0)

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
    set(DOXYGEN_HTML_OUTPUT ".")
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs)
    file(READ README.md README)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/README.in "@mainpage\n ${README}")
    configure_file(${CMAKE_CURRENT_BINARY_DIR}/README.in README.md COPYONLY)
    doxygen_add_docs(docs ${headers} ${CMAKE_CURRENT_BINARY_DIR}/README.md ALL)
endif()
