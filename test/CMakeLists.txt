# Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
# This file is subject to the terms of the MIT License; see the LICENSE file in
# the root directory of this distribution.

add_executable(ismctest main.cpp knockoutwhist.cpp)
target_link_libraries(ismctest ismcsolver)
target_include_directories(ismctest PRIVATE ismcsolver)
