cmake_minimum_required(VERSION 3.17)

include(GravitySupport)
include(CMakePackageConfigHelpers)

project(GravityWrapPython)

set(LIB_NAME _gravity)

find_package(Python REQUIRED COMPONENTS Interpreter Development)

set(SRCS
        "${CMAKE_BINARY_DIR}/cpp/swig/python/gravity_wrap.cpp"
        "${CMAKE_BINARY_DIR}/cpp/swig/python/gravity_wrap.h")
list(APPEND InstallConfigs Debug Release RelWithDebInfo)
    
set_source_files_properties(${SRCS} PROPERTIES GENERATED ON)

include_directories("${CMAKE_BINARY_DIR}" ${PYTHON_INCLUDE_DIRS})
if (WIN32)
    gravity_find_python_debug(Python_LIBRARY_DEBUG)
    if (NOT Python_LIBRARY_DEBUG)
        file(TOUCH "${CMAKE_BINARY_DIR}/dummy.cpp")
        list(REMOVE_ITEM InstallConfigs Debug)
        add_library(${LIB_NAME} SHARED $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:${SRCS}> "${CMAKE_BINARY_DIR}/dummy.cpp")
    else()
        add_library(${LIB_NAME} SHARED ${SRCS})
    endif()
    set_target_properties(${LIB_NAME} PROPERTIES SUFFIX  .pyd)
else()
    add_library(${LIB_NAME} SHARED ${SRCS})
    set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")
endif()

#add_definitions(-DSWIG_PYTHON_INTERPRETER_NO_DEBUG)

target_link_libraries(${LIB_NAME} gravity Python::Python)
gravity_add_dependency(${LIB_NAME})
install(TARGETS ${LIB_NAME} CONFIGURATIONS ${InstallConfigs} LIBRARY DESTINATION lib/gravity RUNTIME DESTINATION lib/gravity)
