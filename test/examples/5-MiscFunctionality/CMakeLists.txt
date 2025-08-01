cmake_minimum_required(VERSION 3.17)
project(MiscFunctionality)
#
# see CMakeLists.txt one level up on how to set up CMAKE_PREFIX_PATH and CMAKE_MODULE_PATH
#


find_package(Gravity QUIET)

# since we use spdlog directly
find_package(spdlog REQUIRED)

set(CMAKE_DEBUG_POSTFIX _d)
add_executable(MiscComponent1 MiscComponent1.cpp)
set_target_properties(MiscComponent1 PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(MiscComponent1 PRIVATE gravity spdlog::spdlog_header_only)


add_executable(MiscComponent2 MiscComponent2.cpp)
set_target_properties(MiscComponent2 PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(MiscComponent2 PRIVATE gravity spdlog::spdlog_header_only)

configure_file(run.sh.in ${MiscFunctionality_SOURCE_DIR}/run.sh)
configure_file(RunMiscFunctionalityDebug.bat.in ${MiscFunctionality_SOURCE_DIR}/RunMiscFunctionalityDebug.bat)
configure_file(RunMiscFunctionalityRelease.bat.in ${MiscFunctionality_SOURCE_DIR}/RunMiscFunctionalityRelease.bat)

