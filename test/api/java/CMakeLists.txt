cmake_minimum_required(VERSION 3.17)

project(JavaUnitTests)
include(GNUInstallDirs)
include(GravitySupport)

find_package(Gravity QUIET)

find_package(GravityJava REQUIRED)
include(UseJava)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/*.proto")
gravity_protobuf_generate(APPEND_PATH LANGUAGE java PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}" GENERATE_EXTENSIONS .java OUT_VAR PROTO_SRCS PROTOS ${PROTO_FILES})
add_custom_target(${PROJECT_NAME}_protos ALL DEPENDS ${PROTO_SRCS})

set(LOCAL_PROTOBUF_JAR "${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}/protobuf-java.jar")
set(PUBLIC_JAR "/usr/share/java/protobuf.jar")
set(CMAKE_JAVA_INCLUDE_PATH "${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}/protobuf-java.jar" ${PUBLIC_JAR})
string(APPEND CMAKE_JAVA_INCLUDE_PATH_FINAL_RELEASE "${_UseJava_PATH_SEP}${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}/gravity.jar${_UseJava_PATH_SEP}${CMAKE_BINARY_DIR}/src/api/java/gravity.jar")
string(APPEND CMAKE_JAVA_INCLUDE_PATH_FINAL_DEBUG "${_UseJava_PATH_SEP}${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}/gravity${CMAKE_DEBUG_POSTFIX}.jar${_UseJava_PATH_SEP}${CMAKE_BINARY_DIR}/src/api/java/gravity${CMAKE_DEBUG_POSTFIX}.jar")

set(JAVA_SRCS 
        src/com/aphysci/gravity/GravityJavaTest.java
        ${PROTO_SRCS})

set(ORIG_CMAKE_DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set(CMAKE_DEBUG_POSTFIX)
gravity_add_jar_debug_release(${PROJECT_NAME} ${JAVA_SRCS})
list(APPEND _DEPS ${PROJECT_NAME}_protos)
if (TARGET gravity-java)
    list(APPEND _DEPS gravity-java)
endif()
add_dependencies(${PROJECT_NAME} ${_DEPS})
set(CMAKE_DEBUG_POSTFIX ${ORIG_CMAKE_DEBUG_POSTFIX})

#Setup for the test: java doesn't allow jars in the classpath that don't exist
#                    and java.library.path needs to have the libzmq directory even if in a system location
set(CP_FOR_TEST)
list(APPEND CP_FOR_TEST "${CMAKE_CURRENT_BINARY_DIR}/JavaUnitTests.jar")
if (GRAVITY_USING_EXTERNAL_PROTOBUF)
    list(APPEND CP_FOR_TEST "${LOCAL_PROTOBUF_JAR}")
else()
    list(APPEND CP_FOR_TEST "${PUBLIC_JAR}")
endif()
list(APPEND CP_FOR_TEST "${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}/gravity.jar")
string(REPLACE ";" "${_UseJava_PATH_SEP}" CP_FOR_TEST_PATH_SEP "${CP_FOR_TEST}")
get_target_property(ZeroMQ_LIBRARY_RELEASE libzmq LOCATION_RELEASE)
get_filename_component(LIBZMQ_LIBRARY_DIR "${ZeroMQ_LIBRARY_RELEASE}" DIRECTORY)

add_test(NAME JavaUnitTest COMMAND ${Java_JAVA_EXECUTABLE} -cp "${CP_FOR_TEST_PATH_SEP}" 
                                        "-Djava.library.path=${LIBZMQ_LIBRARY_DIR}${_UseJava_PATH_SEP}${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}" com.aphysci.gravity.GravityJavaTest)
set_property(TEST JavaUnitTest PROPERTY ENVIRONMENT_MODIFICATION  
  "LD_LIBRARY_PATH=path_list_prepend:${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}"
  "PATH=path_list_prepend:${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_BINDIR}")