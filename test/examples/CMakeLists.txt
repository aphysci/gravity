cmake_minimum_required(VERSION 3.17)

project(GravityExamples)

set(CMAKE_CXX_STANDARD 11)

if (NOT GRAVITY_ROOT)
    message(FATAL_ERROR "Please define GRAVITY_ROOT to the Gravity install location")
endif()

message(STATUS "GravityRoot: ${GRAVITY_ROOT}")
get_filename_component(ABS_GRAVITY_ROOT "${GRAVITY_ROOT}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")

# for GravitySupport.cmake
list(APPEND CMAKE_MODULE_PATH "${ABS_GRAVITY_ROOT}/lib/cmake/gravity")
# for *Config.cmake and the like. this will look in lots of subdirectories therein for the right files.
list(APPEND CMAKE_PREFIX_PATH "${ABS_GRAVITY_ROOT}")

set(CMAKE_DEBUG_POSTFIX _d)

include(CTest)

include_directories("${CMAKE_BINARY_DIR}")
add_subdirectory("1-BasicDataProduct")
add_subdirectory("2-ProtobufDataProduct")
#add_subdirectory("8-ConfigFile")

find_package(GravityJava)
if (NOT Java_FOUND)
    set(SKIP_JAVA ON)
elseif(NOT SKIP_JAVA)
    set(SKIP_JAVA OFF)
endif()

if (NOT SKIP_JAVA)
    add_subdirectory("6-JavaProtobufDataProduct")
endif()

if (NOT SKIP_PYTHON)
    add_subdirectory("11-PythonPubSub")
    add_subdirectory("12-PythonBasicService")
endif()
