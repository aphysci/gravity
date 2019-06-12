#================================================
# Copyright (c) Applied Physcical Sciences, Corp.
#================================================
#
# Gravity is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program;
# If not, see <http://www.gnu.org/licenses/>.
#
#
# - Generate C++ or java sources from Google protobuf input files
#
# This module is a modified version of the function available in
# FindProtobuf::protobuf_generate_cpp
#
# Usage of this module as follows:
#
#  gravity_protobuf_generate_cpp(DEST SRCS HDRS ARGN)
#
# where:
#  - DEST is the compiled output destination path
#         This should be a path relative to ${CMAKE_CURRENT_BINARY_DIR}
#  - SRCS variable to store list of generated source files
#  - HDRS variable to store list of generated header files (C++ only)
#  - ARGN .proto files
#
# Variables defined by FindProtobuf and used by this module:
#
#  PROTOBUF_GENERATE_CPP_APPEND_PATH    Path to append to include path for each proto file
#  PROTOBUF_IMPORT_DIRS                 Additional directories to search for proto files
#  PROTOBUF_PROTOC_EXECUTABLE           Location of protoc
#


function(gravity_protobuf_generate_cpp DEST SRCS HDRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: GRAVITY_PROTOBUF_GENERATE_CPP() called without any proto files")
    return()
  endif()
  if(NOT Protobuf_FOUND)
    find_package(Protobuf QUIET)
  endif()

  if(PROTOBUF_GENERATE_CPP_APPEND_PATH)
    # Create an include path for each file specified
    foreach(FIL ${ARGN})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(ABS_PATH ${ABS_FIL} DIRECTORY)
      list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
      if(${_contains_already} EQUAL -1)
        list(APPEND _protobuf_include_path -I ${ABS_PATH})
      endif()
    endforeach()
  else()
    set(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  if(DEFINED PROTOBUF_IMPORT_DIRS)
    foreach(DIR ${PROTOBUF_IMPORT_DIRS})
      get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
      list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
      if(${_contains_already} EQUAL -1)
        list(APPEND _protobuf_include_path -I ${ABS_PATH})
      endif()
    endforeach()
  endif()

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${DEST}/${FIL_WE}.pb.cc")
    list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${DEST}/${FIL_WE}.pb.h")

    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${DEST})

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${DEST}/${FIL_WE}.pb.cc"
      "${CMAKE_CURRENT_BINARY_DIR}/${DEST}/${FIL_WE}.pb.h"
      COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS --cpp_out  ${CMAKE_CURRENT_BINARY_DIR}/${DEST} ${_protobuf_include_path} ${ABS_FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "Generating C++ from protocol buffer input ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

