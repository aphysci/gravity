cmake_minimum_required(VERSION 3.17)

project(gravity)
include(GNUInstallDirs)
set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

set(GRAVITY_USING_EXTERNAL_PROTOBUF ${GRAVITY_USE_EXTERNAL_PROTOBUF})
set(GRAVITY_USING_EXTERNAL_ZEROMQ ${GRAVITY_USE_EXTERNAL_ZEROMQ})
set(GRAVITY_USING_EXTERNAL_SPDLOG ${GRAVITY_USE_EXTERNAL_SPDLOG})

if (NOT SKIP_JAVA)
    find_package(GravityJava REQUIRED)

    if (NOT SKIP_JAVA AND NOT JAVA_HOME)
	get_filename_component(REAL_JAVA_EXEC "${Java_JAVA_EXECUTABLE}" REALPATH)
	get_filename_component(JAVA_BIN_DIR "${REAL_JAVA_EXEC}" DIRECTORY)
        get_filename_component(JAVA_HOME "${JAVA_BIN_DIR}/../" ABSOLUTE)
        set(ENV{JAVA_HOME} "${JAVA_HOME}")
	message(STATUS "Set ENV{JAVA_HOME} to ${JAVA_HOME}")
    endif()
endif()

add_subdirectory(keyvalue_parser)
add_subdirectory(api/protobufs)
add_subdirectory(api/cpp)
add_subdirectory(api/rust)

if (NOT SKIP_JAVA)
    add_subdirectory(api/java/src/swig)
    add_subdirectory(api/java/src/cpp)
    add_subdirectory(api/java)
    add_subdirectory(api/MATLAB)
endif()

if (NOT BUILD_LIBRARY_ONLY)
    add_subdirectory(components/cpp/ServiceDirectory)
    add_subdirectory(components/cpp/Archiver)
    add_subdirectory(components/cpp/ConfigServer)
    add_subdirectory(components/cpp/LogRecorder)
    add_subdirectory(components/cpp/Playback)
    add_subdirectory(components/cpp/Relay)
endif()

if (NOT SKIP_PYTHON)
    add_subdirectory(api/python/src/swig)
    add_subdirectory(api/python/src/cpp)
endif()

install(FILES 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/FindPThreadsWin32.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/FindZeroMQ.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/GravityExternalUrls.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/GravitySupport.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/FindGravityJava.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/UseJavaClassFilelist.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gravity)


