cmake_minimum_required(VERSION 3.17)
project(Archiving)
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
add_executable(ArchiveTest ArchiveTest.cpp ${PROTO_SRCS})
set_target_properties(ArchiveTest PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(ArchiveTest PUBLIC gravity PRIVATE spdlog::spdlog_header_only)
target_include_directories(ArchiveTest PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(ArchiveTest ${PROJECT_NAME}_protos)

add_executable(ReplayTest ReplayTest.cpp ${PROTO_SRCS})
set_target_properties(ReplayTest PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(ReplayTest PUBLIC gravity PRIVATE spdlog::spdlog_header_only)
target_include_directories(ReplayTest PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(ReplayTest ${PROJECT_NAME}_protos)

configure_file(run.sh.in ${Archiving_SOURCE_DIR}/run.sh)
