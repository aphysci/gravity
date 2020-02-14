cmake_minimum_required(VERSION 3.11)

include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GravityExternalUrls)

project(gravity-meta)

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

if (WIN32)
    add_external_protobuf(Debug)
    add_external_protobuf(Release)
    list(APPEND depends protobuf_Release protobuf_Debug)
    list(APPEND protobuf_depends protobuf_Release protobuf_Debug)
    
    add_external_libzmq(Debug)
    add_external_libzmq(Release)
    list(APPEND depends libzmq_Debug libzmq_Release) 
else()
    add_external_protobuf(${CMAKE_BUILD_TYPE})
    list(APPEND depends protobuf_${CMAKE_BUILD_TYPE})
    list(APPEND protobuf_depends protobuf_${CMAKE_BUILD_TYPE})
    
    add_external_libzmq(${CMAKE_BUILD_TYPE})
    list(APPEND depends libzmq_${CMAKE_BUILD_TYPE})
    
endif()

find_package(GravityJava REQUIRED)

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
        )

ExternalProject_Add(
    gravity_external 
    DEPENDS protobuf-java_external ${depends}
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src"
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBOOST_ROOT=${BOOST_ROOT}
        -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
    BUILD_ALWAYS 1)

