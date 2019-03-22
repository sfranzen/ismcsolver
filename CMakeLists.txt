project(ismcsolver)

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(ECMInstallIcons)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_library(ismcsolver INTERFACE)
target_include_directories(ismcsolver INTERFACE include/)

add_subdirectory(test)
