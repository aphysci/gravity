cmake_minimum_required(VERSION 3.17)

project(AnomalyDetection)
include(GNUInstallDirs)
include(GravitySupport)

find_package(Gravity QUIET)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/../protobuf/*.proto")



gravity_protobuf_generate(APPEND_PATH LANGUAGE python PROTOC_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" GENERATE_EXTENSIONS .py OUT_VAR SRCS PROTOS ${PROTO_FILES})
add_custom_target(${PROJECT_NAME} ALL DEPENDS "${SRCS}")

set(PY_EXEC python3)
if (WIN32)
    set(PY_EXEC python)
endif()

configure_file(train.sh.in ${AnomalyDetection_SOURCE_DIR}/train.sh)
configure_file(detect.sh.in ${AnomalyDetection_SOURCE_DIR}/detect.sh)
