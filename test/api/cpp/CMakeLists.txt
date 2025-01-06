cmake_minimum_required(VERSION 3.17)
project(CppUnitTests)

#
# see CMakeLists.txt on how to set up CMAKE_PREFIX_PATH and CMAKE_MODULE_PATH
#

# for gravity_protobuf_generate
include(GravitySupport)

find_package(spdlog REQUIRED)
find_package(Gravity QUIET)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/protobuf/*.proto")
gravity_protobuf_generate(APPEND_PATH PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR} OUT_VAR PROTO_SRCS
                            PROTOS ${PROTO_FILES})

add_custom_target(${PROJECT_NAME}_protos ALL DEPENDS ${PROTO_SRCS})

file(GLOB SRCS "tests/*.cpp")

set(CMAKE_DEBUG_POSTFIX _d)
add_executable(unit_tests ${SRCS} ${PROTO_SRCS})
target_link_libraries(unit_tests PUBLIC gravity PRIVATE spdlog::spdlog_header_only)
target_include_directories(unit_tests PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(unit_tests ${PROJECT_NAME}_protos)

add_test(unit_tests ${CMAKE_CURRENT_BINARY_DIR}/unit_tests)
set_tests_properties(unit_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
set_property(TEST unit_tests PROPERTY ENVIRONMENT_MODIFICATION  
                                            "LD_LIBRARY_PATH=path_list_prepend:${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}"
                                            "PATH=path_list_prepend:${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_BINDIR}"
                                            "PYTHONPATH=path_list_prepend:${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}"
                                            "PYTHONPATH=path_list_prepend:${CMAKE_CURRENT_BINARY_DIR}"
                                          )


