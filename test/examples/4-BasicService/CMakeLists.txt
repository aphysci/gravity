cmake_minimum_required(VERSION 3.17)
project(BasicService)
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
add_executable(BasicServiceProvider BasicServiceProvider.cpp ${PROTO_SRCS})
set_target_properties(BasicServiceProvider PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(BasicServiceProvider PUBLIC gravity PRIVATE spdlog::spdlog_header_only)
target_include_directories(BasicServiceProvider PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(BasicServiceProvider ${PROJECT_NAME}_protos)

add_executable(BasicServiceRequestor BasicServiceRequestor.cpp ${PROTO_SRCS})
set_target_properties(BasicServiceRequestor PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(BasicServiceRequestor PUBLIC gravity PRIVATE spdlog::spdlog_header_only)
target_include_directories(BasicServiceRequestor PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(BasicServiceRequestor ${PROJECT_NAME}_protos)

configure_file(run.sh.in ${BasicService_SOURCE_DIR}/run.sh)

