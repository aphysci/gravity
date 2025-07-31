cmake_minimum_required(VERSION 3.17)

project(JavaProtobufDataProduct)
include(GNUInstallDirs)
include(GravitySupport)

find_package(Gravity QUIET)

if (Protobuf_SRC_ROOT_FOLDER)
	set(LOCAL_PROTOBUF_JAR "${Protobuf_SRC_ROOT_FOLDER}/java/target/protobuf-java-${Protobuf_VERSION}.jar")
endif()
#Need prefix path here because build/install gets created at the end of the build process, and doesn't exist yet when this gets built
set(PUBLIC_JAR "${JAVA_PROTOBUF_JAR}" "${LOCAL_PROTOBUF_JAR}")
#list(JOIN PUBLIC_JAR ":" PUBLIC_JAR_CACHE)
#set(PUBLIC_JAR_CACHE ${PUBLIC_JAR_CACHE} CACHE ON STRING)
find_package(GravityJava REQUIRED)
include(UseJava)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/../protobuf/*.proto")

gravity_protobuf_generate(APPEND_PATH LANGUAGE java PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}" GENERATE_EXTENSIONS .java OUT_VAR PROTO_SRCS PROTOS ${PROTO_FILES})

add_custom_target(${PROJECT_NAME}_protos ALL DEPENDS ${PROTO_SRCS})

#if(WIN32)
set(CMAKE_JAVA_INCLUDE_PATH ${PUBLIC_JAR}) #Takes semicolons as dividers
#else()
#set(CMAKE_JAVA_INCLUDE_PATH ${PUBLIC_JAR_CACHE}) #Linux takes colons as dividers
 #endif()
string(APPEND CMAKE_JAVA_INCLUDE_PATH_FINAL_RELEASE "${_UseJava_PATH_SEP}${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}/gravity.jar${_UseJava_PATH_SEP}${CMAKE_BINARY_DIR}/src/api/java/gravity.jar")
string(APPEND CMAKE_JAVA_INCLUDE_PATH_FINAL_DEBUG "${_UseJava_PATH_SEP}${ABS_GRAVITY_ROOT}/${CMAKE_INSTALL_LIBDIR}/gravity${CMAKE_DEBUG_POSTFIX}.jar${_UseJava_PATH_SEP}${CMAKE_BINARY_DIR}/src/api/java/gravity${CMAKE_DEBUG_POSTFIX}.jar")
message(STATUS "CMAKE_JAVA_INCLUDE_PATH_FINAL_DEBUG: ${CMAKE_JAVA_INCLUDE_PATH_FINAL_DEBUG}")

if (WIN32)
    get_target_property(ZeroMQ_LIBRARY_RELEASE libzmq LOCATION_RELEASE)
    get_filename_component(LIBZMQ_LIBRARY_DIR "${ZeroMQ_LIBRARY_RELEASE}" DIRECTORY)
    set(LOCAL_ZEROMQ_RUNTIME_DIR "${LIBZMQ_LIBRARY_DIR}/../bin")
    
    get_target_property(Protobuf_LIBRARY_RELEASE protobuf::libprotobuf LOCATION_RELEASE)
    get_filename_component(LOCAL_PROTOBUF_RUNTIME_DIR "${Protobuf_LIBRARY_RELEASE}" DIRECTORY)
    
    # configure_file("${CMAKE_CURRENT_LIST_DIR}/RunJavaProtobufDataProductDebug.bat.in" "${CMAKE_BINARY_DIR}/RunJavaProtobufDataProductDebug.bat" @ONLY)
    #configure_file("${CMAKE_CURRENT_LIST_DIR}/RunJavaProtobufDataProductRelease.bat.in" "${CMAKE_BINARY_DIR}/RunJavaProtobufDataProductRelease.bat" @ONLY)
    
endif()

set(JAVA_SRCS 
        src/JavaProtobufDataProductPublisher.java
        src/JavaProtobufDataProductSubscriber.java
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

gravity_install_jar(${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

if (WIN32)
    install(FILES "${CMAKE_BINARY_DIR}/RunJavaProtobufDataProductDebug.bat" DESTINATION bin CONFIGURATIONS Debug)
    install(FILES "${CMAKE_BINARY_DIR}/RunJavaProtobufDataProductRelease.bat" DESTINATION bin CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
endif()

configure_file(run.sh.in ${JavaProtobufDataProduct_SOURCE_DIR}/run.sh)
#configure_file(RunJavaProtobufDataProductDebug.bat.in ${JavaProtobufDataProduct_SOURCE_DIR}/RunJavaProtobufDataProductDebug.bat)
#configure_file(RunJavaProtobufDataProductRelease.bat.in ${JavaProtobufDataProduct_SOURCE_DIR}/RunJavaProtobufDataProductRelease.bat)
