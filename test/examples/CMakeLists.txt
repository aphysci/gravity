cmake_minimum_required(VERSION 3.11)

project(GravityExamples)

if (NOT GRAVITY_ROOT)
    message(FATAL_ERROR "Please define GRAVITY_ROOT to the Gravity install location")
endif()

message(STATUS "GravityRoot: ${GRAVITY_ROOT}")
get_filename_component(ABS_GRAVITY_ROOT "${GRAVITY_ROOT}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")
set(CMAKE_INSTALL_PREFIX "${ABS_GRAVITY_ROOT}")
message(STATUS "Installing to: ${CMAKE_INSTALL_PREFIX}")

list(APPEND CMAKE_MODULE_PATH "${ABS_GRAVITY_ROOT}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${ABS_GRAVITY_ROOT}/cmake")

if (EXISTS "${CMAKE_INSTALL_PREFIX}/deps/boost/bootstrap.bat")
    set(BOOST_ROOT "${CMAKE_INSTALL_PREFIX}/deps/boost")
endif()

add_subdirectory("1-BasicDataProduct")
add_subdirectory("2-ProtobufDataProduct")

find_package(GravityJava)
if (NOT Java_FOUND)
    set(SKIP_JAVA ON)
elseif(NOT SKIP_JAVA)
    set(SKIP_JAVA OFF)
endif()

if (NOT SKIP_JAVA)
    add_subdirectory("6-JavaProtobufDataProduct")
endif()
