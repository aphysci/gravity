cmake_minimum_required(VERSION 3.11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GravityExternalUrls)
include(GravitySupport)
include(ExternalProject)

project(gravity-meta)

option(GRAVITY_USE_INTERNAL_PROTOBUF "Download, Build and Use an internal protobuf build")
option(GRAVITY_USE_INTERNAL_ZEROMQ "Download, build and use and internal ZeroMQ build")
option(BUILD_EXAMPLES "Build Gravity examples" ON)
option(SKIP_JAVA "Skip building Gravity Java wrapper")
option(SKIP_PYTHON "Skip building Gravity Python wrapper") 
set(JAVA_HOME "" CACHE PATH "Path to JDK to use")
set(BOOST_ROOT "" CACHE PATH "Path to boost to use")
set(Protobuf_SRC_ROOT_FOLDER "" CACHE PATH "Path to Protobuf source build")
option(Protobuf_USE_STATIC_LIBS "Use a static external build of protobufs")
set(ZMQ_HOME "" CACHE PATH "Path to your external ZeroMQ installation")

get_filename_component(ABS_CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")
set(CMAKE_INSTALL_PREFIX "${ABS_CMAKE_INSTALL_PREFIX}")

string(REPLACE "\"" "" Protobuf_SRC_ROOT_FOLDER "${Protobuf_SRC_ROOT_FOLDER}")
file(TO_CMAKE_PATH "${Protobuf_SRC_ROOT_FOLDER}" Protobuf_SRC_ROOT_FOLDER)

macro(add_external_protobuf build_type)
    ExternalProject_Add(    
        protobuf_${build_type}
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/protobuf"
        URL ${protobuf_url}
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/deps/protobuf
            -Dprotobuf_BUILD_SHARED_LIBS:BOOL=ON
            -Dprotobuf_BUILD_TESTS:BOOL=OFF
            -Dprotobuf_BUILD_EXAMPLES:BOOL=OFF
            -Dprotobuf_WITH_ZLIB:BOOL=OFF
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        SOURCE_SUBDIR cmake
        BUILD_COMMAND ""
        INSTALL_COMMAND cmake --build . --target install --config ${build_type})
endmacro()

macro(add_external_libzmq build_type)
    ExternalProject_Add(    
        libzmq_${build_type}
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libzmq"
        URL ${libzmq_url}
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/deps/libzmq
            -DBUILD_SHARED:BOOL=ON
            -DBUILD_STATIC:BOOL=OFF
            -DZMQ_BUILD_TESTS:BOOL=OFF
            -DBUILD_TESTS:BOOL=OFF
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        BUILD_COMMAND ""
        INSTALL_COMMAND cmake --build . --target install --config ${build_type})
endmacro()

find_package(Boost)

if (NOT Boost_FOUND)
    if (NOT EXISTS "${CMAKE_INSTALL_PREFIX}/deps/boost/bootstrap.bat")
        ExternalProject_Add(
            boost
            SOURCE_DIR "${CMAKE_INSTALL_PREFIX}/deps/boost"
            URL ${boost_url}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND "")
        list(APPEND depends boost)
    endif()
    set(BOOST_ROOT "${CMAKE_INSTALL_PREFIX}/deps/boost")
else()
    list(GET Boost_INCLUDE_DIRS 0 BOOST_ROOT)
endif()

gravity_find_protobuf(Protobuf_External OFF)
gravity_find_zeromq(ZeroMQ_External OFF)

if (WIN32)
    if (NOT Protobuf_External)
        add_external_protobuf(Debug)
        add_external_protobuf(Release)
        list(APPEND depends protobuf_Release protobuf_Debug)
        list(APPEND protobuf_depends protobuf_Release protobuf_Debug)
    endif()
    if (NOT ZeroMQ_External)
        add_external_libzmq(Debug)
        add_external_libzmq(Release)
        list(APPEND depends libzmq_Debug libzmq_Release)
    endif()        
else()
    if (NOT Protobuf_External)
        add_external_protobuf(${CMAKE_BUILD_TYPE})
        list(APPEND depends protobuf_${CMAKE_BUILD_TYPE})
        list(APPEND protobuf_depends protobuf_${CMAKE_BUILD_TYPE})
    endif()
    if (NOT ZeroMQ_External)
        add_external_libzmq(${CMAKE_BUILD_TYPE})
        list(APPEND depends libzmq_${CMAKE_BUILD_TYPE})
    endif()
endif()

if (Protobuf_External)
    message(STATUS "Using external protobuf libraries")
endif()
if (ZeroMQ_External)
    message(STATUS "Using external zeromq libraries")
endif()

find_package(GravityJava)

if (NOT Java_FOUND)
    set(SKIP_JAVA ON)
elseif(NOT SKIP_JAVA)
    set(SKIP_JAVA OFF)
endif()

if (NOT SKIP_JAVA AND NOT Protobuf_External)
    ExternalProject_Add(
        protobuf-java_external
        DEPENDS ${protobuf_depends}
        SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/api/protobufs/protobufs-java"
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DProtobufs_SRC_DIR=${CMAKE_CURRENT_BINARY_DIR}/protobuf
            -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	    -DGRAVITY_USE_INTERNAL_PROTOBUF=${GRAVITY_USE_INTERNAL_PROTOBUF}
        )
    list(APPEND depends protobuf-java_external)
endif()

if (SKIP_JAVA)
    message(STATUS "Building without Java")
endif()

set(Python_ADDITIONAL_VERSIONS 2 3)
find_package(PythonInterp)
find_package(PythonLibs)

if (NOT PYTHONLIBS_FOUND)
    set(SKIP_PYTHON ON)
elseif(NOT SKIP_PYTHON)
    set(SKIP_PYTHON OFF)
endif()

if (NOT SKIP_PYTHON AND "${PYTHON_VERSION_STRING}" VERSION_LESS "2.7.9")
     message(WARNING "Please install python version 2.7.9 or later to build the python wrapper, skipping python for this build")
     set(SKIP_PYTHON ON)
endif()

if (SKIP_PYTHON)
    message(STATUS "Building without Python")
elseif(NOT Protobuf_External)
    ExternalProject_Add(
	    py-protobufs-external
	    DEPENDS ${protobuf_depends}
	    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/py-protobuf
	    LOG_CONFIGURE ON
	    LOG_BUILD ON
	    LOG_INSTALL ON
	    INSTALL_COMMAND ""
	    CMAKE_ARGS
              -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
	      -DProtobufs_SRC_DIR=${CMAKE_CURRENT_BINARY_DIR}/protobuf
	    -DGRAVITY_USE_INTERNAL_PROTOBUF=${GRAVITY_USE_INTERNAL_PROTOBUF}
        -DGRAVITY_USE_INTERNAL_ZEROMQ=${GRAVITY_USE_INTERNAL_ZEROMQ}
	      )
      list(APPEND depends py-protobufs-external)
endif()

ExternalProject_Add(
    gravity_external 
    DEPENDS ${depends}
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src"
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBOOST_ROOT=${BOOST_ROOT}
        -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
        -DSKIP_JAVA=${SKIP_JAVA}
        -DSKIP_PYTHON=${SKIP_PYTHON}
        -DGRAVITY_USE_INTERNAL_PROTOBUF=${GRAVITY_USE_INTERNAL_PROTOBUF}
        -DGRAVITY_USE_INTERNAL_ZEROMQ=${GRAVITY_USE_INTERNAL_ZEROMQ}
        -DProtobuf_SRC_ROOT_FOLDER=${Protobuf_SRC_ROOT_FOLDER}
        -DProtobuf_USE_STATIC_LIBS=${Protobuf_USE_STATIC_LIBS}
    BUILD_ALWAYS 1)

if (BUILD_EXAMPLES)
    ExternalProject_Add(
        gravity_external_examples 
        DEPENDS gravity_external
        SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/test/examples"
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        CMAKE_ARGS
	    -DGRAVITY_ROOT=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DBOOST_ROOT=${BOOST_ROOT}
            -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
            -DSKIP_JAVA=${SKIP_JAVA}
            -DSKIP_PYTHON=${SKIP_PYTHON}
            -DGRAVITY_USE_INTERNAL_PROTOBUF=${GRAVITY_USE_INTERNAL_PROTOBUF}
            -DGRAVITY_USE_INTERNAL_ZEROMQ=${GRAVITY_USE_INTERNAL_ZEROMQ}
            -DProtobuf_SRC_ROOT_FOLDER=${Protobuf_SRC_ROOT_FOLDER}
            -DProtobuf_USE_STATIC_LIBS=${Protobuf_USE_STATIC_LIBS}
    BUILD_ALWAYS 1)
endif()
