cmake_minimum_required(VERSION 3.11)

project(gravity)

set(CMAKE_DEBUG_POSTFIX _d)

add_subdirectory(keyvalue_parser)
add_subdirectory(api/protobufs)
add_subdirectory(api/cpp)
add_subdirectory(api/java)
add_subdirectory(api/MATLAB)
add_subdirectory(api/java/src/swig)
add_subdirectory(api/java/src/cpp)
add_subdirectory(components/cpp/ServiceDirectory)
add_subdirectory(components/cpp/Archiver)
add_subdirectory(components/cpp/ConfigServer)
add_subdirectory(components/cpp/LogRecorder)
add_subdirectory(components/cpp/Playback)
add_subdirectory(components/cpp/Relay)

install(FILES 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/FindPThreadsWin32.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/GravityExternalUrls.cmake" 
            "${CMAKE_CURRENT_LIST_DIR}/../cmake/GravitySupport.cmake" DESTINATION cmake)

