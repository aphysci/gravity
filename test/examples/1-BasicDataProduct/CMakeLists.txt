cmake_minimum_required(VERSION 3.17)
project(BasicDataProduct)
#
# see CMakeLists.txt one level up on how to set up CMAKE_PREFIX_PATH and CMAKE_MODULE_PATH
#


find_package(Gravity QUIET)

# since we use spdlog directly
find_package(spdlog REQUIRED)

set(CMAKE_DEBUG_POSTFIX _d)
add_executable(BasicDataProductPublisher BasicDataProductPublisher.cpp)
set_target_properties(BasicDataProductPublisher PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(BasicDataProductPublisher PRIVATE gravity spdlog::spdlog_header_only)


add_executable(BasicDataProductSubscriber BasicDataProductSubscriber.cpp)
set_target_properties(BasicDataProductSubscriber PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(BasicDataProductSubscriber PRIVATE gravity spdlog::spdlog_header_only)

configure_file(run.sh.in ${BasicDataProduct_SOURCE_DIR}/run.sh)
configure_file(RunBasicDataProductDebug.bat.in ${BasicDataProduct_SOURCE_DIR}/RunBasicDataProductDebug.bat)
configure_file(RunBasicDataProductRelease.bat.in ${BasicDataProduct_SOURCE_DIR}/RunBasicDataProductRelease.bat)
