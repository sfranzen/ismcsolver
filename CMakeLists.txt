# Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
# This file is subject to the terms of the MIT License; see the LICENSE file in
# the root directory of this distribution.

project(ismcsolver)

cmake_minimum_required(VERSION 3.12.0)
set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)
add_library(ismcsolver INTERFACE)
target_compile_features(ismcsolver INTERFACE cxx_std_11)
target_link_libraries(ismcsolver INTERFACE Threads::Threads)
target_include_directories(ismcsolver INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(GNUInstallDirs)
set(export_dir ${CMAKE_INSTALL_LIBDIR}/cmake/ismcsolver)
install(DIRECTORY include/ismcts DESTINATION include)
install(TARGETS ismcsolver EXPORT ismcsolver DESTINATION include)
install(EXPORT ismcsolver DESTINATION ${export_dir})

include(CMakePackageConfigHelpers)
set(config_file ${CMAKE_CURRENT_BINARY_DIR}/ismcsolver-config.cmake)
configure_package_config_file(
    cmake/ismcsolver-config.cmake.in
    ${config_file}
    INSTALL_DESTINATION ${export_dir}
)
install(FILES ${config_file} DESTINATION ${export_dir})

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
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_BRIEF_MEMBER_DESC NO)
    set(DOXYGEN_ALWAYS_DETAILED_SEC YES)
    set(DOXYGEN_SORT_MEMBER_DOCS NO)
    set(DOXYGEN_STRIP_FROM_INC_PATH "include")
    set(DOXYGEN_HTML_EXTRA_STYLESHEET docs/extra.css)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs)
    doxygen_add_docs(docs include docs/main.md)
endif()
