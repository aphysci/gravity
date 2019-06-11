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

set (SOURCES
  ComponentDataLookupResponsePB.proto
  ComponentLookupRequestPB.proto
  ComponentServiceLookupResponsePB.proto
  ConfigRequest.proto
  FileArchiverControlRequestPB.proto
  FileArchiverControlResponsePB.proto
  GravityDataProductPB.proto
  GravityLogMessagePB.proto
  GravityMetricsDataPB.proto
  ServiceDirectoryBroadcastPB.proto
  ServiceDirectoryDomainUpdatePB.proto
  ServiceDirectoryMapPB.proto
  ServiceDirectoryRegistrationPB.proto
  ServiceDirectoryResponsePB.proto
  ServiceDirectoryUnregistrationPB.proto)

protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${SOURCES})

set (PROTO_FILES ${PROTO_HEADERS} ${PROTO_SOURCES} PARENT_SCOPE)

add_library(protobufs STATIC ${PROTO_SOURCES} ${PROTO_HEADERS})
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(protobufs ${PROTOBUF_LIBRARIES})

#install(TARGETS protobufs EXPORT ProtobufsConfig
#  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
#  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
#  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # Windows hack
#install(FILES ${PROTO_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/protobuf)
## This makes the project importable from the install directory
#install(EXPORT ProtobufsConfig DESTINATION cmake)
## and importable from the build directory
#export(TARGETS protobufs FILE ProtobufsConfig.cmake)


