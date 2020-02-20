cmake_minimum_required(VERSION 3.11)

include(GravitySupport)

project(gravity-swig NONE)

include(FetchContent)
include(GravityExternalUrls)

find_package(ZeroMQ REQUIRED PATHS "${CMAKE_INSTALL_PREFIX}/deps/libzmq/CMake" "${CMAKE_INSTALL_PREFIX}/deps/libzmq/share/cmake")

if (WIN32)
    get_target_property(ZeroMQ_LIBRARY_RELEASE libzmq LOCATION_RELEASE)
    get_filename_component(LIBZMQ_LIBRARY_FILENAME "${ZeroMQ_LIBRARY_RELEASE}" NAME_WE)
else()
    get_target_property(ZeroMQ_LIBRARY_${CMAKE_BUILD_TYPE} libzmq LOCATION_${CMAKE_BUILD_TYPE})
    get_filename_component(LIBZMQ_LIBRARY_FILENAME_FULL "${ZeroMQ_LIBRARY_${CMAKE_BUILD_TYPE}}" NAME_WE)
    string(SUBSTRING "${LIBZMQ_LIBRARY_FILENAME_FULL}" 3 -1 LIBZMQ_LIBRARY_FILENAME)
endif()


configure_file("${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i.in" "${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i" @ONLY)

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

message(STATUS ${SWIG_EXECUTABLE} -c++ -o "${CMAKE_CURRENT_LIST_DIR}/../cpp/gravity_wrap.cpp" -outdir "${CMAKE_CURRENT_LIST_DIR}/../java/com/aphysci/gravity/swig" -package com.aphysci.gravity.swig -java "${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i")
add_custom_target(${PROJECT_NAME} ALL COMMAND ${SWIG_EXECUTABLE} -c++ -o "${CMAKE_CURRENT_LIST_DIR}/../cpp/gravity_wrap.cpp" -outdir "${CMAKE_CURRENT_LIST_DIR}/../java/com/aphysci/gravity/swig" -package com.aphysci.gravity.swig -java "${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i"
                    DEPENDS "${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i" 
                    BYPRODUCTS "${CMAKE_CURRENT_LIST_DIR}/../cpp/gravity_wrap.cpp"
                    SOURCES "${CMAKE_CURRENT_LIST_DIR}/cmake_gravity.i")

gravity_add_dependency(${PROJECT_NAME})
													                 
