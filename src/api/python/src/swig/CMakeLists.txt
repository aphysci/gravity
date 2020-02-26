cmake_minimum_required(VERSION 3.11)

include(GravitySupport)

project(gravity-py-swig NONE)

include(FetchContent)
include(GravityExternalUrls)

if (WIN32)
    if (NOT Swig_DIR)
        FetchContent_Populate(FetchSwig URL ${swigwin_url} SOURCE_DIR swig)
        set(Swig_DIR "${CMAKE_CURRENT_BINARY_DIR}/swig")
    endif()
    list(APPEND CMAKE_PROGRAM_PATH "${Swig_DIR}")
    find_package(SWIG REQUIRED MODULE)
else()
    find_package(SWIG REQUIRED MODULE) 
endif()

set(ADDTL_DEFINE)
if (WIN32)
    set(ADDTL_DEFINE -DWIN32)
endif()
add_custom_target(${PROJECT_NAME} ALL COMMAND ${SWIG_EXECUTABLE} -c++ -o "${CMAKE_CURRENT_LIST_DIR}/../cpp/gravity_wrap.cpp" -outdir "${CMAKE_CURRENT_LIST_DIR}/../python/gravity/" -python -threads ${ADDTL_DEFINE} "${CMAKE_CURRENT_LIST_DIR}/gravity.i"
                    DEPENDS "${CMAKE_CURRENT_LIST_DIR}/gravity.i" 
                    BYPRODUCTS "${CMAKE_CURRENT_LIST_DIR}/../cpp/gravity_wrap.cpp"
                    SOURCES "${CMAKE_CURRENT_LIST_DIR}/gravity.i")

gravity_add_dependency(${PROJECT_NAME})
													                 
