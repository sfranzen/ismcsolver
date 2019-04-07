# Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
# This file is subject to the terms of the MIT License; see the LICENSE file in
# the root directory of this distribution.

# Precompile Catch2 to limit compilation times
add_library(testmain OBJECT main.cpp)

add_executable(benchmark benchmark.cpp knockoutwhist.cpp)
target_link_libraries(benchmark ismcsolver testmain)
