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

project(ProtobufDataProduct_example VERSION 1.0 LANGUAGES CXX)

# Linux example runner
configure_file(run.sh.cmake run.sh)

add_executable(ProtobufDataProductPublisher 
  ProtobufDataProductPublisher.cpp 
)
add_executable(ProtobufDataProductSubscriber 
  ProtobufDataProductSubscriber.cpp
)

add_custom_target (${PROJECT_NAME}_run 
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run.sh
  SOURCES run.sh.cmake
)

add_dependencies(ProtobufDataProductPublisher ServiceDirectory)
add_dependencies(ProtobufDataProductSubscriber ServiceDirectory)
add_dependencies(${PROJECT_NAME}_run 
  ServiceDirectory
  ProtobufDataProductPublisher
  ProtobufDataProductSubscriber
)


target_link_libraries(ProtobufDataProductPublisher PRIVATE 
  gravity
  examplemessages
  keyvalue_parser
  zmq
  protobuf
  Threads::Threads
)

target_link_libraries(ProtobufDataProductSubscriber PRIVATE 
  gravity
  examplemessages
  keyvalue_parser
  zmq
  protobuf
  Threads::Threads
)


# 'make install' to the locations provided by GNUInstallDirs
install(TARGETS ProtobufDataProductPublisher ProtobufDataProductSubscriber
        EXPORT ${PROJECT_NAME}Config
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # Windows only

