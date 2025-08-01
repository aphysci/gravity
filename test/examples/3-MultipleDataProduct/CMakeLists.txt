cmake_minimum_required(VERSION 3.17)
project(MultipleDataProduct)
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
add_executable(MultipleDataProductPublisher MultipleDataProductPublisher.cpp ${PROTO_SRCS})
set_target_properties(MultipleDataProductPublisher PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(MultipleDataProductPublisher PRIVATE gravity spdlog::spdlog_header_only)
target_include_directories(MultipleDataProductPublisher PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(MultipleDataProductPublisher ${PROJECT_NAME}_protos)

add_executable(MultipleDataProductSubscriber MultipleDataProductSubscriber.cpp ${PROTO_SRCS})
set_target_properties(MultipleDataProductSubscriber PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(MultipleDataProductSubscriber PRIVATE gravity spdlog::spdlog_header_only)
target_include_directories(MultipleDataProductSubscriber PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(MultipleDataProductSubscriber ${PROJECT_NAME}_protos)

configure_file(run.sh.in ${MultipleDataProduct_SOURCE_DIR}/run.sh)
configure_file(RunMultipleDataProductDebug.bat.in ${MultipleDataProduct_SOURCE_DIR}/RunMultipleDataProductDebug.bat)
configure_file(RunMultipleDataProductRelease.bat.in ${MultipleDataProduct_SOURCE_DIR}/RunMultipleDataProductRelease.bat)

