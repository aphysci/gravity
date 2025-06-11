cmake_minimum_required(VERSION 3.17)
project(ConfigFile)
#
# see CMakeLists.txt one level up on how to set up CMAKE_PREFIX_PATH and CMAKE_MODULE_PATH
#


find_package(Gravity QUIET)

# since we use spdlog directly
find_package(spdlog REQUIRED)

set(CMAKE_DEBUG_POSTFIX _d)
add_executable(ConfigFileExample ConfigFileExample.cpp)
set_target_properties(ConfigFileExample PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(ConfigFileExample gravity spdlog::spdlog_header_only)
configure_file(run.sh.in ${ConfigFile_SOURCE_DIR}/run.sh)
