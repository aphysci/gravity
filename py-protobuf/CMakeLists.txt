cmake_minimum_required(VERSION 3.11)

project(py-protobuf)

find_package(protobuf REQUIRED PATHS "${CMAKE_INSTALL_PREFIX}/deps/protobuf/cmake" "${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")

get_target_property(PROTOC_EXE protobuf::protoc LOCATION) 

file(COPY "${PROTOC_EXE}" DESTINATION "${Protobufs_SRC_DIR}/src/")

find_package(Python REQUIRED COMPONENTS Interpreter Development)

get_target_property(PYTHON_EXE Python::Interpreter LOCATION)
message(STATUS "PYTHON_EXE = ${PYTHON_EXE}")
file(MAKE_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin/")

configure_file("${CMAKE_CURRENT_LIST_DIR}/py-protobufs-setup.sh.in" "${CMAKE_CURRENT_LIST_DIR}/py-protobufs-setup.sh" @ONLY)
if (NOT WIN32)
    add_custom_target(${PROJECT_NAME} ALL 
	    COMMAND "${CMAKE_CURRENT_LIST_DIR}/py-protobufs-setup.sh" 
        COMMAND 
	${CMAKE_COMMAND} -E copy_directory ${Protobufs_SRC_DIR}/python/build/`ls ${Protobufs_SRC_DIR}/python/build/`/google/ "${CMAKE_INSTALL_PREFIX}/bin/google")
endif()

