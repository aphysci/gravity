# (C) Copyright 2019, Applied Physical Sciences Corp., A General Dynamics Company
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

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
include (cmake/GravityVersion.cmake)

gravity_version(GRAVITY_VERSION)

project(gravity VERSION ${GRAVITY_VERSION}
                LANGUAGES C CXX)

file(GENERATE OUTPUT "VERSION.txt" 
  CONTENT "v${GRAVITY_VERSION}")
install(FILES "${gravity_BINARY_DIR}/VERSION.txt" 
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WITH_JAVA   "Enable Java bindings " ON)
option(WITH_PYTHON "Enable Python bindings " ON)

################
#  Dependencies
################

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(ZeroMQ 3.2.3 REQUIRED)
if(ZeroMQ_FOUND)
  add_library(zmq INTERFACE IMPORTED)
  set_property(TARGET zmq PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES ${ZeroMQ_INCLUDE_DIR})
endif()

find_package(Protobuf 2.6.1 REQUIRED)
# NOTE:
# These variables change to mixed case 'Protobuf_' at cmake version 3.7
if(Protobuf_FOUND)
  message(STATUS "PROTOBUF_PROTOC_EXECUTABLE:   ${PROTOBUF_PROTOC_EXECUTABLE}")
  message(STATUS "PROTOBUF_INCLUDE_DIRS:        ${PROTOBUF_INCLUDE_DIRS}")
  message(STATUS "PROTOBUF_LIBRARIES:           ${PROTOBUF_LIBRARIES}")
  add_library(protobuf INTERFACE IMPORTED)
  set_property(TARGET protobuf PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES ${PROTOBUF_INCLUDE_DIRS})
endif()

if(WITH_PYTHON)
  find_package(PythonInterp 2.7 REQUIRED)
  message(STATUS "PYTHON_EXECUTABLE:            ${PYTHON_EXECUTABLE}")
  find_package(PythonLibs REQUIRED)
  message(STATUS "PYTHON_INCLUDE_DIRS:          ${PYTHON_INCLUDE_DIRS}")
  message(STATUS "PYTHON_LIBRARIES:             ${PYTHON_LIBRARIES}")
  message(STATUS "Try to find python dependencies")
  find_package(SWIG 3.0.4 REQUIRED)
endif()

if(WITH_JAVA)
  find_package(Java 1.8 COMPONENTS Development REQUIRED)
  message(STATUS "Try to find java dependencies")
  find_package(SWIG 3.0.4 REQUIRED)
  find_package(Maven 3.3 REQUIRED)
  find_package(Guava 13.0 REQUIRED)
endif()

find_package(Doxygen)

###########################
# Build subdirectories
###########################

add_subdirectory(src/keyvalue_parser)
add_subdirectory(src/api/cpp)
add_subdirectory(src/components/cpp/Archiver)


###########################
# Install Configuration
###########################

message(STATUS ""                                                      )
message(STATUS "Gravity version          = ${GRAVITY_VERSION}"    )
message(STATUS "CMAKE_INSTALL_PREFIX     = ${CMAKE_INSTALL_PREFIX}"    )
message(STATUS "CMAKE_INSTALL_LIBDIR     = ${CMAKE_INSTALL_LIBDIR}"    )
message(STATUS "CMAKE_INSTALL_INCLUDEDIR = ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "CMAKE_INSTALL_DATADIR    = ${CMAKE_INSTALL_DATADIR}"   )
message(STATUS "CMAKE_INSTALL_BINDIR     = ${CMAKE_INSTALL_BINDIR}"    )

################
#   Packaging
################

set(CPACK_PACKAGE_VENDOR "Applied Physical Sciences, Corp.")
set(CPACK_GENERATOR "ZIP;TGZ;${CPACK_GENERATOR}")
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_ZIP "ON")
set(CPACK_PACKAGE_VERSION_MAJOR ${GRAVITY_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${GRAVITY_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${GRAVITY_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/build/GravityLGPLHeader_cpp_java.txt")

set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_SOURCE_DIR}/.git/")

include (CPack)

