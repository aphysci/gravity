cmake_minimum_required(VERSION 3.17)

project(JavaProtobufDataProduct)

include(GravitySupport)

gravity_find_protobuf(ON)
set(PUBLIC_JAR "/usr/share/java/protobuf.jar" "${Protobuf_SRC_ROOT_FOLDER}/java/target/protobuf-java-${Protobuf_VERSION}.jar")
find_package(GravityJava REQUIRED)
include(UseJava)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/../protobuf/*.proto")

gravity_protobuf_generate(APPEND_PATH LANGUAGE java PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../protobuf" GENERATE_EXTENSIONS .java OUT_VAR PROTO_SRCS PROTOS ${PROTO_FILES})

add_custom_target(${PROJECT_NAME}_protos ALL DEPENDS ${PROTO_SRCS})

set(CMAKE_JAVA_INCLUDE_PATH "${ABS_GRAVITY_ROOT}/deps/protobuf/lib/protobuf-java.jar" "${ABS_GRAVITY_ROOT}/lib/gravity.jar" ${PUBLIC_JAR})

set(JAVA_SRCS 
        src/JavaProtobufDataProductPublisher.java
        src/JavaProtobufDataProductSubscriber.java
        ${PROTO_SRCS})
        
add_jar(${PROJECT_NAME} ${JAVA_SRCS})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_protos)

set(INSTALL_CONFIGS)
if (WIN32)
    set(INSTALL_CONFIGS CONFIGURATIONS Release RelWithDebInfo)
endif()

gravity_install_jar(${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" ${INSTALL_CONFIGS})
