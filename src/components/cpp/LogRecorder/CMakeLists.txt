cmake_minimum_required(VERSION 3.17)

include(GravitySupport)

project(LogRecorder)

gravity_find_protobuf(ON)
gravity_find_zeromq(ON)

set(PROTO_SRCS)
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/GravityLogMessagePB.pb.cc
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/GravityLogMessagePB.pb.h)
        
set_source_files_properties(${PROTO_SRCS} PROPERTIES GENERATED TRUE)

set(SRCS
        GravityLogRecorder.cpp
        GravityLogRecorder.h
        main.cpp
        ${PROTO_SRCS}
        )
        
include_directories("${CMAKE_BINARY_DIR}")        
add_executable(${PROJECT_NAME} ${SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX} INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
target_link_libraries(${PROJECT_NAME} PUBLIC gravity)
gravity_add_dependency(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
