cmake_minimum_required(VERSION 3.17)

include(GravitySupport)
project(protobufs-compile)

gravity_find_protobuf(ON)

if (NOT SKIP_JAVA)
  file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/*.proto")

  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/java")
  gravity_protobuf_generate(APPEND_PATH LANGUAGE java PROTOC_OUT_DIR "${CMAKE_BINARY_DIR}/java/" GENERATE_EXTENSIONS .java OUT_VAR SRCS PROTOS ${PROTO_FILES})
  add_custom_target(${PROJECT_NAME} ALL DEPENDS "${SRCS}")
  gravity_add_dependency(${PROJECT_NAME})
endif()

if (NOT SKIP_PYTHON)
    set(PY_PROTO_FILES
            "${CMAKE_CURRENT_LIST_DIR}/GravityDataProductPB.proto"
            "${CMAKE_CURRENT_LIST_DIR}/GravityMetricsDataPB.proto"
            "${CMAKE_CURRENT_LIST_DIR}/FileArchiverControlRequestPB.proto"
            "${CMAKE_CURRENT_LIST_DIR}/FileArchiverControlResponsePB.proto"
            "${CMAKE_CURRENT_LIST_DIR}/ServiceDirectoryMapPB.proto"
            "${CMAKE_CURRENT_LIST_DIR}/ServiceDirectoryDomainUpdatePB.proto")

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/python/gravity") 
    gravity_protobuf_generate(APPEND_PATH LANGUAGE python PROTOC_OUT_DIR "${CMAKE_BINARY_DIR}/python/gravity" GENERATE_EXTENSIONS .py OUT_VAR PY_SRCS PROTOS ${PY_PROTO_FILES})
    add_custom_target(${PROJECT_NAME}-py ALL DEPENDS "${PY_SRCS}")
    gravity_add_dependency(${PROJECT_NAME}-py)
endif()
