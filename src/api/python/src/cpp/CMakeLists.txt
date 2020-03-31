cmake_minimum_required(VERSION 3.11)

include(GravitySupport)
include(CMakePackageConfigHelpers)

project(GravityWrapPython)

set(LIB_NAME _gravity)

set(Python_ADDITIONAL_VERSIONS 2 3)
find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)

set(SRCS
	"${CMAKE_BINARY_DIR}/cpp/swig/python/gravity_wrap.cpp"
	"${CMAKE_BINARY_DIR}/cpp/swig/python/gravity_wrap.h")
    
set_source_files_properties(${SRCS} PROPERTIES GENERATED ON)

include_directories("${CMAKE_BINARY_DIR}" ${PYTHON_INCLUDE_DIRS})
add_library(${LIB_NAME} SHARED ${SRCS})
if (WIN32)
    set_target_properties(${LIB_NAME} PROPERTIES SUFFIX  .pyd)
else()
    set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")
endif()
add_definitions(-DSWIG_PYTHON_INTERPRETER_NO_DEBUG)
target_link_libraries(${LIB_NAME} gravity ${PYTHON_LIBRARIES})
gravity_add_dependency(${LIB_NAME})
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION lib/gravity RUNTIME DESTINATION lib/gravity)
