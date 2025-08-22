cmake_minimum_required(VERSION 3.17)
project(gravity-meta)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GravityExternalUrls)
include(GravitySupport)
include(ExternalProject)

find_package(ZeroMQ)

option(GRAVITY_USE_EXTERNAL_PROTOBUF "Download, Build and Use an externally downloaded protobuf build")
option(GRAVITY_USE_EXTERNAL_ZEROMQ "Download, build and use and externally downloaded ZeroMQ build")
option(GRAVITY_USE_EXTERNAL_SPDLOG "Download, build and use an externally downloaded Spdlog build")
option(BUILD_EXAMPLES_TESTS "Build Gravity examples and tests" ON)
option(BUILD_LIBRARY_ONLY "Only build the libraries, without components or tests")
option(BUILD_STATIC_LIBRARIES "Build libraries to link statically (useful for linking with Rust)")
option(SKIP_JAVA "Skip building Gravity Java wrapper")
option(SKIP_PYTHON "Skip building Gravity Python wrapper") 
set(JAVA_HOME "" CACHE PATH "Path to JDK to use")
set(Protobuf_SRC_ROOT_FOLDER "" CACHE PATH "Path to Protobuf source build")
set(ZMQ_HOME "" CACHE PATH "Path to your local ZeroMQ installation")
set(JAVA_PROTOBUF_JAR "/usr/share/java/protobuf.jar" CACHE PATH "Location of protobuf jar file")

get_filename_component(ABS_CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")
string(REPLACE "\"" "" ABS_CMAKE_INSTALL_PREFIX "${ABS_CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_PREFIX "${ABS_CMAKE_INSTALL_PREFIX}")
file(TO_CMAKE_PATH "${CMAKE_INSTALL_PREFIX}" CMAKE_INSTALL_PREFIX)

string(REPLACE "\"" "" Protobuf_SRC_ROOT_FOLDER "${Protobuf_SRC_ROOT_FOLDER}")
file(TO_CMAKE_PATH "${Protobuf_SRC_ROOT_FOLDER}" Protobuf_SRC_ROOT_FOLDER)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

set(PROTOBUF_CXX_FLAGS)
if (NOT WIN32)
  set(PROTOBUF_CXX_FLAGS -DCMAKE_CXX_FLAGS=-fPIC)
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
  endif()
endif()

# when using ExternalProject, this is where they will be installed during the build process.
# then, the 'make install' target will just install staging directory to CMAKE_INSTALL_PREFIX.
set(PUBLIC_STAGING_DIR ${CMAKE_BINARY_DIR}/staging)

macro(add_external_protobuf)
    if (NOT BUILD_STATIC_LIBRARIES)
        set(BUILD_SHARED_LIBS_PROTO ON)
    else() 
        set(BUILD_SHARED_LIBS_PROTO OFF)
    endif()
    ExternalProject_Add(    
        protobuf_external
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/protobuf"
        URL ${protobuf_url}
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        LOG_DOWNLOAD ON
        LOG_OUTPUT_ON_FAILURE ON
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${PUBLIC_STAGING_DIR}
            -Dprotobuf_BUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS_PROTO}
            -Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF
            -Dprotobuf_BUILD_TESTS:BOOL=OFF
            -Dprotobuf_BUILD_EXAMPLES:BOOL=OFF
            -Dprotobuf_WITH_ZLIB:BOOL=OFF
	    ${PROTOBUF_CXX_FLAGS}
        SOURCE_SUBDIR cmake
        BUILD_COMMAND cmake --build .  --config $<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>
        INSTALL_COMMAND cmake --build . --target install --config $<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>
    )
endmacro()

macro(add_external_libzmq)
    ExternalProject_Add(    
        libzmq_external
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libzmq"
        URL ${libzmq_url}
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        LOG_DOWNLOAD ON
        LOG_OUTPUT_ON_FAILURE ON
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${PUBLIC_STAGING_DIR}
            -DBUILD_SHARED:BOOL=ON
            -DBUILD_STATIC:BOOL=${BUILD_STATIC_LIBRARIES}
            -DZMQ_BUILD_TESTS:BOOL=OFF
            -DBUILD_TESTS:BOOL=OFF
        BUILD_COMMAND cmake --build .  --config $<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>
        INSTALL_COMMAND cmake --build . --target install --config $<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>
    )
endmacro()

macro(add_external_spdlog)
    set(_ext_spdlog_cmake_args "-DCMAKE_INSTALL_PREFIX=${PUBLIC_STAGING_DIR};-DSPDLOG_BUILD_EXAMPLE=OFF")
    if (WIN32)
       list(APPEND _ext_spdlog_cmake_args "-DSPDLOG_BUILD_SHARED=ON")
    endif()
    if (UNIX)
        list(APPEND _ext_spdlog_cmake_args  "-DCMAKE_CXX_FLAGS=-fPIC;-DCMAKE_CXX_VISIBILITY_PRESET=hidden")
	endif()
    ExternalProject_Add(
        spdlog_external
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/spdlog"
        URL ${spdlog_url}
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        LOG_DOWNLOAD ON
        LOG_OUTPUT_ON_FAILURE ON
        CMAKE_ARGS ${_ext_spdlog_cmake_args}
        BUILD_COMMAND cmake --build .  --config $<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>
        INSTALL_COMMAND cmake --build . --target install --config $<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>
    )
endmacro()

if (NOT GRAVITY_USE_EXTERNAL_PROTOBUF)
    gravity_find_protobuf(OFF)
endif()

if (NOT GRAVITY_USE_EXTERNAL_ZEROMQ)
    gravity_find_zeromq(OFF)
endif()

if (NOT GRAVITY_USE_EXTERNAL_SPDLOG)
    gravity_find_spdlog(OFF)
endif()

if (WIN32)
    if (NOT Protobuf_FOUND OR GRAVITY_USE_EXTERNAL_PROTOBUF)
        add_external_protobuf()
        list(APPEND depends protobuf_external)
        list(APPEND protobuf_depends protobuf_external)
    endif()
    if (NOT ZeroMQ_FOUND OR GRAVITY_USE_EXTERNAL_ZEROMQ)
        add_external_libzmq()
        list(APPEND depends libzmq_external)
    endif()        
    if (NOT spdlog_FOUND OR GRAVITY_USE_EXTERNAL_SPDLOG)
        add_external_spdlog(Release)
        list(APPEND depends spdlog_external)
    endif()
else()
    if (NOT Protobuf_FOUND OR GRAVITY_USE_EXTERNAL_PROTOBUF)
        add_external_protobuf()
        list(APPEND depends protobuf_external)
        list(APPEND protobuf_depends protobuf_external)
    endif()
    if (NOT ZeroMQ_FOUND OR GRAVITY_USE_EXTERNAL_ZEROMQ)
        add_external_libzmq()
        list(APPEND depends libzmq_external)
    endif()
    if (NOT spdlog_FOUND OR GRAVITY_USE_EXTERNAL_SPDLOG)
        add_external_spdlog()
        list(APPEND depends spdlog_external)
    endif()
endif()

set(GRAVITY_USING_EXTERNAL_ZEROMQ)
set(GRAVITY_USING_EXTERNAL_SPDLOG)
set(GRAVITY_USING_EXTERNAL_PROTOBUF)

if (Protobuf_FOUND AND NOT GRAVITY_USE_EXTERNAL_PROTOBUF)
    message(STATUS "Using local protobuf libraries")

else()
    message(STATUS "Using external protobuf build")
    set(GRAVITY_USING_EXTERNAL_PROTOBUF ON)

endif()
if (ZeroMQ_FOUND AND NOT GRAVITY_USE_EXTERNAL_ZEROMQ)
    message(STATUS "Using local zeromq libraries")
else()
    message(STATUS "Using external zeromq build")
    set(GRAVITY_USING_EXTERNAL_ZEROMQ ON)
    
endif()
if (spdlog_FOUND AND NOT GRAVITY_USE_EXTERNAL_SPDLOG)
    message(STATUS "Using local spdlog libraries")
else()
    message(STATUS "Using external spdlog build")
    set(GRAVITY_USING_EXTERNAL_SPDLOG ON)
    
endif()



find_package(GravityJava)

if (NOT Java_FOUND)
    set(SKIP_JAVA ON)
elseif(NOT SKIP_JAVA)
    set(SKIP_JAVA OFF)
endif()

list(APPEND CMAKE_PREFIX_PATH "${PUBLIC_STAGING_DIR}")

string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ALT_SEP "${CMAKE_PREFIX_PATH}")

if (NOT SKIP_JAVA AND NOT Protobuf_FOUND)
    ExternalProject_Add(
        protobuf-java_external
        DEPENDS ${protobuf_depends}
        SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/api/protobufs/protobufs-java"
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        LOG_DOWNLOAD ON
        LOG_OUTPUT_ON_FAILURE ON
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${PUBLIC_STAGING_DIR}
            -DProtobufs_SRC_DIR=${CMAKE_CURRENT_BINARY_DIR}/protobuf
            -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
            -DGRAVITY_USE_EXTERNAL_PROTOBUF=${GRAVITY_USE_EXTERNAL_PROTOBUF}
            -Dprotobuf_MODULE_COMPATIBLE=ON
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_ALT_SEP}
        LIST_SEPARATOR |
        )
    list(APPEND depends protobuf-java_external)
endif()

if (SKIP_JAVA)
    message(STATUS "Building without Java")
endif()

find_package(Python COMPONENTS Interpreter Development)

if (NOT Python_FOUND)
    set(SKIP_PYTHON ON)
elseif(NOT SKIP_PYTHON)
    set(SKIP_PYTHON OFF)
endif()

if (NOT SKIP_PYTHON AND "${Python_VERSION}" VERSION_LESS "2.7.9")
     message(WARNING "Please install python version 2.7.9 or later to build the python wrapper, skipping python for this build")
     set(SKIP_PYTHON ON)
endif()

if (SKIP_PYTHON)
    message(STATUS "Building without Python")
elseif(NOT Protobuf_FOUND)
    set(_py_protobufs_external_deps "")
    if (GRAVITY_USING_EXTERNAL_PROTOBUF)
        list(APPEND _py_protobufs_external_deps protobuf_external)
    endif()
    ExternalProject_Add(
	    py-protobufs-external
	    DEPENDS ${protobuf_depends}
	    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/py-protobuf
	    LOG_CONFIGURE ON
	    LOG_BUILD ON
	    LOG_INSTALL ON
        LOG_DOWNLOAD ON
        LOG_OUTPUT_ON_FAILURE ON
	    INSTALL_COMMAND ""
        LIST_SEPARATOR |
        DEPENDS ${_py_protobufs_external_deps}
	    CMAKE_ARGS
              -DCMAKE_INSTALL_PREFIX=${PUBLIC_STAGING_DIR}
	      -DProtobufs_SRC_DIR=${CMAKE_CURRENT_BINARY_DIR}/protobuf
	    -DGRAVITY_USE_EXTERNAL_PROTOBUF=${GRAVITY_USE_EXTERNAL_PROTOBUF}
        -DGRAVITY_USE_EXTERNAL_ZEROMQ=${GRAVITY_USE_EXTERNAL_ZEROMQ}
        -Dprotobuf_MODULE_COMPATIBLE=ON
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_ALT_SEP}
	      )
      list(APPEND depends py-protobufs-external)
endif()

if (NOT depends)
    add_subdirectory(src)
    set(GRAVITY_ROOT "${CMAKE_INSTALL_PREFIX}")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH_ALT_SEP})
    
    if (NOT BUILD_LIBRARY_ONLY)
        include(CTest)
        add_subdirectory(test)

        set(CPACK_GENERATOR "TGZ")
        include(CPack)
    endif()

    message(STATUS "No external libraries - doing a toplevel build")
else()
    ExternalProject_Add(
        gravity_external 
        DEPENDS ${depends}
        SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src"
        LIST_SEPARATOR |
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${PUBLIC_STAGING_DIR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
            -DSKIP_JAVA=${SKIP_JAVA}
            -DSKIP_PYTHON=${SKIP_PYTHON}
            -DZMQ_HOME=${ZMQ_HOME}
            -DBUILD_LIBRARY_ONLY=${BUILD_LIBRARY_ONLY}
            -DBUILD_STATIC_LIBRARIES=${BUILD_STATIC_LIBRARIES}
            -DGRAVITY_USE_EXTERNAL_PROTOBUF=${GRAVITY_USING_EXTERNAL_PROTOBUF}
            -DGRAVITY_USE_EXTERNAL_ZEROMQ=${GRAVITY_USING_EXTERNAL_ZEROMQ}
            -DGRAVITY_USE_EXTERNAL_SPDLOG=${GRAVITY_USING_EXTERNAL_SPDLOG}
            -DProtobuf_SRC_ROOT_FOLDER=${Protobuf_SRC_ROOT_FOLDER}
            -Dprotobuf_MODULE_COMPATIBLE=ON
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_ALT_SEP}
        BUILD_ALWAYS OFF
        BUILD_COMMAND cmake --build .  --config $<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>
        INSTALL_COMMAND cmake --build . --target install --config $<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>
        )

    install(
            DIRECTORY ${PUBLIC_STAGING_DIR}/
            DESTINATION "."
            USE_SOURCE_PERMISSIONS
    )




    if (BUILD_EXAMPLES_TESTS AND NOT BUILD_LIBRARY_ONLY)
        ExternalProject_Add(
            gravity_external_examples_tests
            DEPENDS gravity_external
            SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/test/"
            LIST_SEPARATOR |
            CMAKE_ARGS
                -DGRAVITY_ROOT=${PUBLIC_STAGING_DIR}
		-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/test/examples
		-DTOPLEVEL_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
		-DEXTERNAL_JAVA_JAR=${protobufs-java_BINARY_DIR}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
                -DSKIP_JAVA=${SKIP_JAVA}
                -DZMQ_HOME=${ZMQ_HOME}
                -DSKIP_PYTHON=${SKIP_PYTHON}
                -DProtobuf_SRC_ROOT_FOLDER=${Protobuf_SRC_ROOT_FOLDER}
                -Dprotobuf_MODULE_COMPATIBLE=ON
                -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_ALT_SEP}
                -DPThreadsWin32_INSTALL_DIR=${CMAKE_INSTALL_PREFIX}
            INSTALL_COMMAND ""
            BUILD_ALWAYS OFF)
    endif()
endif()
