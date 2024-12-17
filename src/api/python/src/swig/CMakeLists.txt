cmake_minimum_required(VERSION 3.17)

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

find_package(Python REQUIRED COMPONENTS Interpreter Development)
set(SWIG_THREADS_ARG "")
if (${Python_VERSION} VERSION_LESS "3.7.0")
    set(SWIG_THREADS_ARG "-threads")
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/cpp/swig/python" "${CMAKE_BINARY_DIR}/python/gravity")
add_custom_target(${PROJECT_NAME} ALL COMMAND ${SWIG_EXECUTABLE} -c++ -o "${CMAKE_BINARY_DIR}/cpp/swig/python/gravity_wrap.cpp" -outdir "${CMAKE_BINARY_DIR}/python/gravity/" -python ${SWIG_THREADS_ARG} ${ADDTL_DEFINE} "${CMAKE_CURRENT_LIST_DIR}/gravity.i"
                    DEPENDS "${CMAKE_CURRENT_LIST_DIR}/gravity.i" 
                    BYPRODUCTS "${CMAKE_CURRENT_LIST_DIR}/../cpp/gravity_wrap.cpp" "${CMAKE_BINARY_DIR}/cpp/swig/python/gravity_wrap.cpp"
                    SOURCES "${CMAKE_CURRENT_LIST_DIR}/gravity.i")

if (WIN32)
    gravity_find_python_debug(Python_LIBRARY_DEBUG)
    list(APPEND InstallConfigs Release RelWithDebInfo)
    if (Python_LIBRARY_DEBUG)
        list(APPEND InstallConfigs Debug)
    endif()
else()
    set(InstallConfigs Debug Release RelWithDebInfo)
endif()

gravity_add_dependency(${PROJECT_NAME})
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../python/gravity"  CONFIGURATIONS ${InstallConfigs} DESTINATION ${CMAKE_INSTALL_LIBDIR}  FILES_MATCHING PATTERN "*.py")
install(DIRECTORY "${CMAKE_BINARY_DIR}/python/gravity" CONFIGURATIONS ${InstallConfigs} DESTINATION ${CMAKE_INSTALL_LIBDIR} FILES_MATCHING PATTERN "*.py")
													                 
