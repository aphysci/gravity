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

project(libgravity VERSION 1.0 LANGUAGES CXX)

#####################
#  Source files
#####################

set (HEADERS 
  CommUtil.h
  DomainDataKey.h
  FutureResponse.h
  GravityConfigParser.h
  GravityDataProduct.h
  GravityHeartbeat.h
  GravityHeartbeatListener.h
  GravityLogger.h
  GravityMetrics.h
  GravityMetricsManager.h
  GravityMetricsUtil.h
  GravityNode.h
  GravityPublishManager.h
  GravityRequestManager.h
  GravityRequestor.h
  GravitySemaphore.h
  GravityServiceManager.h
  GravityServiceProvider.h
  GravitySubscriber.h
  GravitySubscriptionManager.h
  GravitySubscriptionMonitor.h
  KeyValueParserWrap.h
  Utility.h)

set (SOURCES
  CommUtil.cpp
  DomainDataKey.cpp
  FutureResponse.cpp
  GravityConfigParser.cpp
  GravityDataProduct.cpp
  GravityHeartbeat.cpp
  GravityLogger.cpp
  GravityMetrics.cpp
  GravityMetricsManager.cpp
  GravityMetricsUtil.cpp
  GravityNode.cpp
  GravityPublishManager.cpp
  GravityRequestManager.cpp
  GravityRequestor.cpp
  GravityServiceManager.cpp
  GravityServiceProvider.cpp
  GravitySubscriber.cpp
  GravitySubscriptionManager.cpp
  GravitySubscriptionMonitor.cpp
  Semaphore.cpp
  Utility.cpp)

set(LIBSRC ${HEADERS} ${SOURCES})

set(PROTOBUF_IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../protobufs" )
set (GRAVITY_MESSAGES
  ${PROTOBUF_IMPORT_DIRS}/ComponentDataLookupResponsePB.proto
  ${PROTOBUF_IMPORT_DIRS}/ComponentLookupRequestPB.proto
  ${PROTOBUF_IMPORT_DIRS}/ComponentServiceLookupResponsePB.proto
  ${PROTOBUF_IMPORT_DIRS}/ConfigRequest.proto
  ${PROTOBUF_IMPORT_DIRS}/FileArchiverControlRequestPB.proto
  ${PROTOBUF_IMPORT_DIRS}/FileArchiverControlResponsePB.proto
  ${PROTOBUF_IMPORT_DIRS}/GravityDataProductPB.proto
  ${PROTOBUF_IMPORT_DIRS}/GravityLogMessagePB.proto
  ${PROTOBUF_IMPORT_DIRS}/GravityMetricsDataPB.proto
  ${PROTOBUF_IMPORT_DIRS}/ServiceDirectoryBroadcastPB.proto
  ${PROTOBUF_IMPORT_DIRS}/ServiceDirectoryDomainUpdatePB.proto
  ${PROTOBUF_IMPORT_DIRS}/ServiceDirectoryMapPB.proto
  ${PROTOBUF_IMPORT_DIRS}/ServiceDirectoryRegistrationPB.proto
  ${PROTOBUF_IMPORT_DIRS}/ServiceDirectoryResponsePB.proto
  ${PROTOBUF_IMPORT_DIRS}/ServiceDirectoryUnregistrationPB.proto)


#####################
#  Targets
#####################

include(${gravity_SOURCE_DIR}/cmake/GravityProtobuf.cmake)
gravity_protobuf_generate_cpp(protobuf PROTO_SOURCES PROTO_HEADERS ${GRAVITY_MESSAGES})

add_library(gravitylib OBJECT ${LIBSRC} ${PROTO_SOURCES} ${PROTO_HEADERS})

# shared and static libraries built from the same object files
add_library(gravity STATIC $<TARGET_OBJECTS:gravitylib>)
add_library(gravity_shared SHARED $<TARGET_OBJECTS:gravitylib>)

#####################
#  Properties
#####################

# shared libraries need PIC
set_property(TARGET gravitylib PROPERTY POSITION_INDEPENDENT_CODE 1)

set_target_properties(gravity_shared PROPERTIES OUTPUT_NAME "gravity")

#####################
#  Dependencies
#####################

target_link_libraries(gravity_shared PRIVATE 
  keyvalue_parser
  zmq
  protobuf
  Threads::Threads)

target_link_libraries(gravity PRIVATE 
  keyvalue_parser
  zmq
  protobuf
  Threads::Threads)

target_include_directories(gravitylib
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${libkeyvalue_parser_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/protobuf>
)

target_include_directories(gravity
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/protobuf>
)

target_include_directories(gravity_shared
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/protobuf>
)

# 'make install' to the locations provided by GNUInstallDirs
install(TARGETS gravity gravity_shared EXPORT GravityConfig
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # Windows only
install(FILES ${HEADERS} 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${PROTO_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/protobuf)

# This makes the project importable from the install directory
install(EXPORT GravityConfig DESTINATION cmake)
# and importable from the build directory
export(TARGETS gravity gravity_shared FILE GravityConfig.cmake)

