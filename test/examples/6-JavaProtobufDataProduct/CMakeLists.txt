cmake_minimum_required(VERSION 3.11)

project(JavaProtobufDataProduct)

include(GravitySupport)

find_package(protobuf REQUIRED PATHS "${CMAKE_INSTALL_PREFIX}/deps/protobuf/cmake" "${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/cmake")
find_package(GravityJava REQUIRED)
include(UseJava)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/../protobuf/*.proto")

gravity_protobuf_generate(APPEND_PATH LANGUAGE java PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../protobuf" GENERATE_EXTENSIONS .java OUT_VAR PROTO_SRCS PROTOS ${PROTO_FILES})

add_custom_target(${PROJECT_NAME}_protos ALL DEPENDS ${PROTO_SRCS})

set(CMAKE_JAVA_INCLUDE_PATH "${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/protobuf-java.jar" "${CMAKE_INSTALL_PREFIX}/lib/gravity.jar")

set(JAVA_SRCS 
        src/JavaProtobufDataProductPublisher.java
        src/JavaProtobufDataProductSubscriber.java
        ${PROTO_SRCS})
        
add_jar(${PROJECT_NAME} ${JAVA_SRCS})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_protos)
install_jar(${PROJECT_NAME} "${CMAKE_INSTALL_PREFIX}/bin")
