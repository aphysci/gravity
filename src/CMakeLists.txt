cmake_minimum_required(VERSION 3.17)

project(gravity)

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}/deps/protobuf/cmake" "${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/cmake")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}/deps/libzmq/CMake" "${CMAKE_INSTALL_PREFIX}/deps/libzmq/share/cmake")

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
if (NOT SKIP_JAVA)
    add_subdirectory(api/protobufs)
endif()
add_subdirectory(api/cpp)
if (NOT SKIP_JAVA)
    add_subdirectory(api/java/src/swig)
    add_subdirectory(api/java/src/cpp)
    add_subdirectory(api/java)
    add_subdirectory(api/MATLAB)
endif()
add_subdirectory(components/cpp/ServiceDirectory)
add_subdirectory(components/cpp/Archiver)
add_subdirectory(components/cpp/ConfigServer)
add_subdirectory(components/cpp/LogRecorder)
add_subdirectory(components/cpp/Playback)
add_subdirectory(components/cpp/Relay)

if (NOT SKIP_PYTHON)
    add_subdirectory(api/python/src/proto/gravity)
    add_subdirectory(api/python/src/swig)
    add_subdirectory(api/python/src/cpp)
endif()

install(FILES 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/FindPThreadsWin32.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/GravityExternalUrls.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/GravitySupport.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/FindGravityJava.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/UseJavaClassFilelist.cmake" DESTINATION cmake)


