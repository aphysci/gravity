cmake_minimum_required(VERSION 3.11)

include(GravitySupport)

project(Playback)

gravity_find_protobuf(external ON)
gravity_find_zeromq(ZeroMQ_External ON)

set(SRCS
        FileReader.cpp
        FileReader.h
        FileReplay.cpp
        FileReplay.h
        )
        
include_directories("${CMAKE_BINARY_DIR}")        
add_executable(${PROJECT_NAME} ${SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(${PROJECT_NAME} gravity)
gravity_add_dependency(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
