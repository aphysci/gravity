cmake_minimum_required(VERSION 3.17)

project(PythonBasicService)
include(GNUInstallDirs)
include(GravitySupport)

find_package(Gravity QUIET)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/../protobuf/*.proto")



gravity_protobuf_generate(APPEND_PATH LANGUAGE python PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}" GENERATE_EXTENSIONS .py OUT_VAR SRCS PROTOS ${PROTO_FILES})
add_custom_target(${PROJECT_NAME} ALL DEPENDS "${SRCS}")

set(PY_EXEC python3)
if (WIN32)
    set(PY_EXEC python)
endif()
add_test(NAME pybasicservice COMMAND ${PY_EXEC} requestor.py WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
set_property(TEST pybasicservice PROPERTY ENVIRONMENT_MODIFICATION
  "LD_LIBRARY_PATH=path_list_prepend:${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}"
  "PYTHONPATH=path_list_prepend:${CMAKE_CURRENT_BINARY_DIR}"
  "PYTHONPATH=path_list_prepend:${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}"
)
