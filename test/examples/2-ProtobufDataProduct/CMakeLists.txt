cmake_minimum_required(VERSION 3.17)

project(ProtoBufDataProduct)

include(GravitySupport)

find_package(PThreadsWin32 REQUIRED)
gravity_find_protobuf(ON)
gravity_find_zeromq(ON)
gravity_find_spdlog(ON)
find_package(GravityKeyValueParser QUIET)
find_package(Gravity QUIET)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/../protobuf/*.proto")

gravity_protobuf_generate(APPEND_PATH PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../protobuf" OUT_VAR PROTO_SRCS 
                            PROTOS ${PROTO_FILES})

add_custom_target(${PROJECT_NAME}_protos ALL DEPENDS ${PROTO_SRCS})

set(CMAKE_DEBUG_POSTFIX _d)
add_executable(ProtobufDataProductPublisher ProtobufDataProductPublisher.cpp ${PROTO_SRCS})
set_target_properties(ProtobufDataProductPublisher PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(ProtobufDataProductPublisher gravity)
add_dependencies(ProtobufDataProductPublisher ${PROJECT_NAME}_protos)

add_executable(ProtobufDataProductSubscriber ProtobufDataProductSubscriber.cpp ${PROTO_SRCS})
set_target_properties(ProtobufDataProductSubscriber PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(ProtobufDataProductSubscriber gravity)
add_dependencies(ProtobufDataProductSubscriber ${PROJECT_NAME}_protos)
install(TARGETS ProtobufDataProductPublisher ProtobufDataProductSubscriber DESTINATION bin)
