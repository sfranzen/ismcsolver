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

add_library(ismcsolver INTERFACE)
target_compile_features(ismcsolver INTERFACE cxx_std_11)
target_link_libraries(ismcsolver INTERFACE Threads::Threads)
target_include_directories(ismcsolver INTERFACE include)
target_sources(ismcsolver INTERFACE ${headers})
install(FILES ${headers} DESTINATION include)

# Tests
include(CTest)
set(BUILD_TESTING CACHE PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)

# Code coverage
option(CODE_COVERAGE "Enable coverage reporting" OFF)
add_library(coverage_config INTERFACE)

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # Set debug + coverage
  target_compile_options(coverage_config INTERFACE -O0 -g --coverage)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(coverage_config INTERFACE --coverage)
  else()
    target_link_libraries(coverage_config INTERFACE --coverage)
  endif()
endif()

if (BUILD_TESTING OR CODE_COVERAGE)
    enable_testing()
    add_subdirectory(test)
endif()

# Documentation
if(DOXYGEN_FOUND)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_BRIEF_MEMBER_DESC NO)
    set(DOXYGEN_ALWAYS_DETAILED_SEC YES)
    set(DOXYGEN_SORT_MEMBER_DOCS NO)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET docs/extra.css)
    set(DOXYGEN_STRIP_FROM_INC_PATH "include")
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs)
    doxygen_add_docs(docs ${headers} docs/main.md)
endif()
