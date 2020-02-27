cmake_minimum_required(VERSION 3.11)

include(GravitySupport)
include(CMakePackageConfigHelpers)

project(GravityWrapPython)

set(LIB_NAME _gravity)

find_package (Python REQUIRED COMPONENTS Interpreter Development)

set(SRCS
    "${CMAKE_CURRENT_LIST_DIR}/gravity_wrap.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gravity_wrap.h")
    
set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/gravity_wrap.cpp" "${CMAKE_CURRENT_LIST_DIR}/gravity_wrap.h" PROPERTIES GENERATED ON)

add_library(${LIB_NAME} SHARED ${SRCS})
if (WIN32)
    set_target_properties(${LIB_NAME} PROPERTIES SUFFIX  .pyd)
else()
    set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")
endif()
target_link_libraries(${LIB_NAME} gravity Python::Python)
gravity_add_dependency(${LIB_NAME})
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION bin/gravity RUNTIME DESTINATION bin/gravity)
