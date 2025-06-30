cmake_minimum_required(VERSION 3.17)
project(ProtobufDataProduct)
#
# see CMakeLists.txt on how to set up CMAKE_PREFIX_PATH and CMAKE_MODULE_PATH
#

# for gravity_protobuf_generate
include(GravitySupport)


find_package(spdlog REQUIRED)
find_package(Gravity QUIET)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/../protobuf/*.proto")

gravity_protobuf_generate(APPEND_PATH PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR} OUT_VAR PROTO_SRCS
                            PROTOS ${PROTO_FILES})

add_custom_target(${PROJECT_NAME}_protos ALL DEPENDS ${PROTO_SRCS})

set(CMAKE_DEBUG_POSTFIX _d)
add_executable(ProtobufDataProductPublisher2 ProtobufDataProductPublisher2.cpp ${PROTO_SRCS})
set_target_properties(ProtobufDataProductPublisher2 PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(ProtobufDataProductPublisher2 PUBLIC gravity PRIVATE spdlog::spdlog_header_only)
target_include_directories(ProtobufDataProductPublisher2 PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(ProtobufDataProductPublisher2 ${PROJECT_NAME}_protos)

add_executable(ProtobufDataProductSubscriber2 ProtobufDataProductSubscriber2.cpp ${PROTO_SRCS})
set_target_properties(ProtobufDataProductSubscriber2 PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(ProtobufDataProductSubscriber2 PUBLIC gravity PRIVATE spdlog::spdlog_header_only)
target_include_directories(ProtobufDataProductSubscriber2 PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(ProtobufDataProductSubscriber2 ${PROJECT_NAME}_protos)

configure_file(run.sh.in ${ProtobufDataProduct_SOURCE_DIR}/run.sh)
configure_file(RunProtobufDataProductDebug.bat.in ${ProtobufDataProduct_SOURCE_DIR}/RunProtobufDataProductDebug.bat)
configure_file(RunProtobufDataProductRelease.bat.in ${ProtobufDataProduct_SOURCE_DIR}/RunProtobufDataProductRelease.bat)

