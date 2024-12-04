cmake_minimum_required(VERSION 3.17)

include(GravitySupport)
project(PythonPubSub)

gravity_find_protobuf(ON)
gravity_find_spdlog(ON)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/../protobuf/*.proto")

gravity_protobuf_generate(APPEND_PATH LANGUAGE python PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../protobuf" GENERATE_EXTENSIONS .py OUT_VAR SRCS PROTOS ${PROTO_FILES})
add_custom_target(${PROJECT_NAME} ALL DEPENDS "${SRCS}")
gravity_add_dependency(${PROJECT_NAME})
add_test(NAME pypubsub COMMAND python3 pypub.py WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
set_property(TEST pypubsub PROPERTY ENVIRONMENT_MODIFICATION  
  "LD_LIBRARY_PATH=string_append:${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/lib/gravity"
  "PYTHONPATH=string_append:${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/lib/gravity:${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/:${CMAKE_CURRENT_LIST_DIR}/../protobuf:${CMAKE_CURRENT_LIST_DIR}")
