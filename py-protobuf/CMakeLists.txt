cmake_minimum_required(VERSION 3.17)

project(py-protobuf)
include(GNUInstallDirs)
find_package(protobuf REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")

get_target_property(PROTOC_EXE protobuf::protoc LOCATION) 

file(COPY "${PROTOC_EXE}" DESTINATION "${Protobufs_SRC_DIR}/src/")
message(STATUS "PROTOC_EXE: ${PROTOC_EXE}")
if (NOT WIN32)
    get_filename_component(PROTOC_FN "${PROTOC_EXE}" NAME)
    if (NOT "${PROTOC_FN}" STREQUAL "protoc")
        file(RENAME "${Protobufs_SRC_DIR}/src/${PROTOC_FN}" "${Protobufs_SRC_DIR}/src/protoc")
        message(STATUS "PROTOC_EXE now: ${Protobufs_SRC_DIR}/src/protoc")
    endif()
endif()


find_package(Python REQUIRED COMPONENTS Interpreter Development)

set(PYTHON_EXE "${Python_EXECUTABLE}")
message(STATUS "PYTHON_EXE = ${PYTHON_EXE}")
file(MAKE_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

if (WIN32)
    configure_file("${CMAKE_CURRENT_LIST_DIR}/py-protobufs-setup.bat.in" "${CMAKE_CURRENT_LIST_DIR}/py-protobufs-setup.bat" @ONLY)
    add_custom_target(${PROJECT_NAME} ALL 
	    COMMAND "${CMAKE_CURRENT_LIST_DIR}/py-protobufs-setup.bat" 
        COMMAND 
	${CMAKE_COMMAND} -E copy_directory ${Protobufs_SRC_DIR}/python/build/lib/google/ "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/google")
else()
    configure_file("${CMAKE_CURRENT_LIST_DIR}/py-protobufs-setup.sh.in" "${CMAKE_CURRENT_LIST_DIR}/py-protobufs-setup.sh" @ONLY)
    add_custom_target(${PROJECT_NAME} ALL 
	    COMMAND "${CMAKE_CURRENT_LIST_DIR}/py-protobufs-setup.sh" 
        COMMAND 
	${CMAKE_COMMAND} -E copy_directory ${Protobufs_SRC_DIR}/python/build/`ls ${Protobufs_SRC_DIR}/python/build/`/google/ "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/google")
endif()

