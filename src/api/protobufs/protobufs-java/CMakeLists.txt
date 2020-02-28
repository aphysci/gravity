cmake_minimum_required(VERSION 3.11)

include(FetchContent)
include(GravitySupport)

project(protobufs-java NONE)

if (NOT Protobufs_SRC_DIR)
    message(FATAL_ERROR "Protobufs_SRC_DIR not set")
endif()

find_package(GravityJava REQUIRED)
include(UseJava)

gravity_find_protobuf(external ON)

gravity_protobuf_generate(APPEND_PATH LANGUAGE java PROTOC_OUT_DIR "${Protobufs_SRC_DIR}/java/core/src/main/java" IMPORT_DIRS "${Protobufs_SRC_DIR}/src" GENERATE_EXTENSIONS .java OUT_VAR SRCS PROTOS 
                    "${Protobufs_SRC_DIR}/src/google/protobuf/descriptor.proto")
                    
add_custom_target(${PROJECT_NAME}_protos ALL DEPENDS "${SRCS}")
message("SRCS: ${SRCS}")

file(GLOB Java_SRCS "${Protobufs_SRC_DIR}/java/core/src/main/java/com/google/protobuf/*.java")
set(Generated_Java "${Protobufs_SRC_DIR}/java/core/src/main/java/com/google/protobuf/DescriptorProtos.java")
set_source_files_properties(${Generated_Java} PROPERTIES GENERATED TRUE)

foreach(file ${Java_SRCS})
    string(TOLOWER ${file} file_lower)
    string(TOLOWER ${Generated_Java} srcs_lower)
    string(COMPARE EQUAL "${file_lower}" "${srcs_lower}" eq)
    if (eq)
        list(REMOVE_ITEM Java_SRCS ${file})
        break()
    endif()
endforeach()

        
add_jar(protobuf-java ${Generated_Java} ${Java_SRCS})
add_dependencies(protobuf-java ${PROJECT_NAME}_protos)
install_jar(protobuf-java "${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib")
