cmake_minimum_required(VERSION 3.17)

project(GravityUnityPlugin)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
if (NOT GRAVITY_ROOT)
    message(FATAL_ERROR "Please define GRAVITY_ROOT to the Gravity install location")
endif()

message(STATUS "GravityRoot: ${GRAVITY_ROOT}")
get_filename_component(ABS_GRAVITY_ROOT "${GRAVITY_ROOT}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")

list(APPEND CMAKE_PREFIX_PATH "${ABS_GRAVITY_ROOT}/deps/protobuf/cmake" "${ABS_GRAVITY_ROOT}/deps/protobuf/lib/cmake")
list(APPEND CMAKE_PREFIX_PATH "${ABS_GRAVITY_ROOT}/deps/libzmq/CMake" "${ABS_GRAVITY_ROOT}/deps/libzmq/share/cmake")
list(APPEND CMAKE_PREFIX_PATH "${ABS_GRAVITY_ROOT}/deps/spdlog/lib/cmake" "${ABS_GRAVITY_ROOT}/deps/spdlog/lib/cmake/spdlog")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")
message(STATUS "Installing to: ${CMAKE_INSTALL_PREFIX}")

list(APPEND CMAKE_MODULE_PATH "${ABS_GRAVITY_ROOT}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${ABS_GRAVITY_ROOT}/cmake")
set(CMAKE_DEBUG_POSTFIX _d)
add_subdirectory(src)


