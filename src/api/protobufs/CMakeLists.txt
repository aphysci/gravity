cmake_minimum_required(VERSION 3.11)

include(GravitySupport)
project(protobufs-compile)

find_package(protobuf REQUIRED PATHS "${CMAKE_INSTALL_PREFIX}/deps/protobuf/cmake" "${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/cmake")

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/*.proto")

gravity_protobuf_generate(APPEND_PATH LANGUAGE java PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../java/src/java" GENERATE_EXTENSIONS .java OUT_VAR SRCS PROTOS ${PROTO_FILES})
add_custom_target(${PROJECT_NAME} ALL DEPENDS "${SRCS}")
gravity_add_dependency(${PROJECT_NAME})
