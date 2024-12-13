cmake_minimum_required(VERSION 3.17)

include(GravitySupport)

project(Playback)

gravity_find_protobuf(ON)
gravity_find_zeromq(ON)
gravity_find_spdlog(ON)

set(SRCS
        FileReader.cpp
        FileReader.h
        FileReplay.cpp
        FileReplay.h
        )
        
include_directories("${CMAKE_BINARY_DIR}")        
add_executable(${PROJECT_NAME} ${SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX} INSTALL_RPATH "\$ORIGIN/../lib")
target_link_libraries(${PROJECT_NAME} PUBLIC gravity PRIVATE spdlog::spdlog_header_only)
gravity_add_dependency(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
