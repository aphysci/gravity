cmake_minimum_required(VERSION 3.11)

include(GravitySupport)
project(protobufs-compile)

gravity_find_protobuf(external ON)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/*.proto")

gravity_protobuf_generate(APPEND_PATH LANGUAGE java PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../java/src/java" GENERATE_EXTENSIONS .java OUT_VAR SRCS PROTOS ${PROTO_FILES})
add_custom_target(${PROJECT_NAME} ALL DEPENDS "${SRCS}")
gravity_add_dependency(${PROJECT_NAME})

if (NOT SKIP_PYTHON)
    set(PY_PROTO_FILES
            "${CMAKE_CURRENT_LIST_DIR}/GravityDataProductPB.proto"
            "${CMAKE_CURRENT_LIST_DIR}/GravityMetricsDataPB.proto"
            "${CMAKE_CURRENT_LIST_DIR}/FileArchiverControlRequestPB.proto"
            "${CMAKE_CURRENT_LIST_DIR}/FileArchiverControlResponsePB.proto"
            "${CMAKE_CURRENT_LIST_DIR}/ServiceDirectoryMapPB.proto"
            "${CMAKE_CURRENT_LIST_DIR}/ServiceDirectoryDomainUpdatePB.proto")
    
    gravity_protobuf_generate(APPEND_PATH LANGUAGE python PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../python/src/python/gravity" GENERATE_EXTENSIONS .py OUT_VAR PY_SRCS PROTOS ${PY_PROTO_FILES})
    add_custom_target(${PROJECT_NAME}-py ALL DEPENDS "${PY_SRCS}")
    gravity_add_dependency(${PROJECT_NAME}-py)
    install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../python/src/python/gravity/" DESTINATION lib/gravity FILES_MATCHING PATTERN *.py)
endif()
