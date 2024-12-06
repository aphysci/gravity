cmake_minimum_required(VERSION 3.17)

include(GravitySupport)
project(PythonPubSub)

gravity_find_protobuf(ON)
gravity_find_spdlog(ON)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/../protobuf/*.proto")

gravity_protobuf_generate(APPEND_PATH LANGUAGE python PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../protobuf" GENERATE_EXTENSIONS .py OUT_VAR SRCS PROTOS ${PROTO_FILES})
add_custom_target(${PROJECT_NAME} ALL DEPENDS "${SRCS}")
gravity_add_dependency(${PROJECT_NAME})
set(PY_EXEC python3)
if (WIN32)
    set(PY_EXEC C:/hostedtoolcache/windows/Python/3.12.7/x64/python3.exe)
endif()
add_test(NAME pypubsub COMMAND ${PY_EXEC} pypub.py WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
set_property(TEST pypubsub PROPERTY ENVIRONMENT_MODIFICATION  
  "LD_LIBRARY_PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib;LD_LIBRRARY_PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib/gravity"
  "PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib;PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib/gravity;PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/bin;PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/deps/libzmq/bin;PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/deps/spdlog/bin;PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/deps/pthreads-w32/bin"
  "PYTHONPATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib;PYTHONPATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib/gravity;PYTHONPATH=path_list_append:${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/;PYTHONPATH=path_list_append:${CMAKE_CURRENT_LIST_DIR}/../protobuf;PYTHONPATH=path_list_append:${CMAKE_CURRENT_LIST_DIR}")
