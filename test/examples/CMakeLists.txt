cmake_minimum_required(VERSION 3.11)

project(GravityExamples)

if (NOT GRAVITY_ROOT)
    message(FATAL_ERROR "Please define GRAVITY_ROOT to the Gravity install location")
endif()

get_filename_component(ABS_GRAVITY_ROOT "${GRAVITY_ROOT}" ABSOLUTE)
set(CMAKE_INSTALL_PREFIX "${ABS_GRAVITY_ROOT}")

add_subdirectory("1-BasicDataProduct")