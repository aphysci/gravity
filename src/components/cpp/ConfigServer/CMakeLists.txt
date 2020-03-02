cmake_minimum_required(VERSION 3.11)

include(GravitySupport)

project(ConfigServer)

gravity_find_protobuf(external ON)
find_package(ZeroMQ REQUIRED PATHS "${CMAKE_INSTALL_PREFIX}/deps/libzmq/CMake")

set(PROTO_SRCS)
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ConfigRequest.pb.cc
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ConfigRequest.pb.h)
        
set_source_files_properties(${PROTO_SRCS} PROPERTIES GENERATED TRUE)

set(SRCS
        ConfigServer.cpp
        ${PROTO_SRCS}
        )
        
include_directories("${CMAKE_BINARY_DIR}")        
add_executable(${PROJECT_NAME} ${SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(${PROJECT_NAME} gravity)
gravity_add_dependency(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
