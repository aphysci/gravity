cmake_minimum_required(VERSION 3.17)

include(GravitySupport)

project(gravity-swig NONE)

include(FetchContent)
include(GravityExternalUrls)

gravity_find_zeromq(ON)

if (WIN32)
    get_target_property(ZeroMQ_LIBRARY_RELEASE libzmq LOCATION_RELEASE)
    get_filename_component(LIBZMQ_LIBRARY_FILENAME_RELEASE "${ZeroMQ_LIBRARY_RELEASE}" NAME_WE)
    get_target_property(ZeroMQ_LIBRARY_DEBUG libzmq LOCATION_DEBUG)
    if (NOT ZeroMQ_LIBRARY_DEBUG)
        set(ZeroMQ_LIBRARY_DEBUG "${ZeroMQ_LIBRARY_RELEASE}")
    endif()
    get_filename_component(LIBZMQ_LIBRARY_FILENAME_DEBUG "${ZeroMQ_LIBRARY_DEBUG}" NAME_WE)
else()
    get_target_property(ZeroMQ_LIBRARY_${CMAKE_BUILD_TYPE} libzmq LOCATION_${CMAKE_BUILD_TYPE})
    get_filename_component(LIBZMQ_LIBRARY_FILENAME_FULL "${ZeroMQ_LIBRARY_${CMAKE_BUILD_TYPE}}" NAME_WE)
    string(SUBSTRING "${LIBZMQ_LIBRARY_FILENAME_FULL}" 3 -1 LIBZMQ_LIBRARY_FILENAME_RELEASE)
    set(LIBZMQ_LIBRARY_FILENAME_DEBUG ${LIBZMQ_LIBRARY_FILENAME_RELEASE})
endif()

set(POSTFIX)
set(LIBZMQ_LIBRARY_FILENAME ${LIBZMQ_LIBRARY_FILENAME_RELEASE})
configure_file("${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i.in" "${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i" @ONLY)

set(POSTFIX _d)
set(LIBZMQ_LIBRARY_FILENAME ${LIBZMQ_LIBRARY_FILENAME_DEBUG})
configure_file("${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i.in" "${CMAKE_CURRENT_LIST_DIR}/cmake_gravity_d.i" @ONLY)

if (WIN32)
    if (NOT Swig_DIR)
        FetchContent_Populate(FetchSwig URL ${swigwin_url} SOURCE_DIR swig)
        set(Swig_DIR "${CMAKE_CURRENT_BINARY_DIR}/swig")
    endif()
    list(APPEND CMAKE_PROGRAM_PATH "${Swig_DIR}")
    find_package(SWIG REQUIRED MODULE)
else()
    find_package(SWIG REQUIRED MODULE) 
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/cpp/swig/java" "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig")

add_custom_target(${PROJECT_NAME} ALL COMMAND ${SWIG_EXECUTABLE} -c++ -o 
                    "$<$<NOT:$<CONFIG:Debug>>:${CMAKE_BINARY_DIR}/cpp/swig/java/gravity_wrap.cpp>$<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/cpp/swig/java/gravity_wrap_d.cpp>" -outdir 
                    "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig" -package com.aphysci.gravity.swig -java 
                    "$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i>$<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/cmake_gravity_d.i>"
                    DEPENDS "${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i" 
                    BYPRODUCTS "${CMAKE_BINARY_DIR}/cpp/swig/java/gravity_wrap.cpp" "${CMAKE_BINARY_DIR}/cpp/swig/java/gravity_wrap_d.cpp" "${CMAKE_BINARY_DIR}/cpp/swig/java/gravity_wrap.h" "${CMAKE_BINARY_DIR}/cpp/swig/java/gravity_wrap_d.h"
                    SOURCES "$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i>$<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/cmake_gravity_d.i>")

gravity_add_dependency(${PROJECT_NAME})
													                 
