cmake_minimum_required(VERSION 3.17)

include(GravitySupport)

project(Relay)

gravity_find_protobuf(ON)
gravity_find_zeromq(ON)
gravity_find_spdlog(ON)

set(SRCS
        Relay.cpp
        )
        
include_directories("${CMAKE_BINARY_DIR}")        
add_executable(${PROJECT_NAME} ${SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX} INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE gravity spdlog::spdlog_header_only)
gravity_add_dependency(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
