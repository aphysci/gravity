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

project(ServiceDirectoryReregister_test VERSION 1.0 LANGUAGES CXX)

configure_file(test.sh.cmake test.sh)
configure_file(Gravity.ini ${CMAKE_CURRENT_BINARY_DIR})

add_executable(${PROJECT_NAME}_provider   Provider.cpp)
add_executable(${PROJECT_NAME}_publisher  Publisher.cpp)
add_executable(${PROJECT_NAME}_requestor  Requestor.cpp)
add_executable(${PROJECT_NAME}_subscriber Subscriber.cpp)

add_custom_target (${PROJECT_NAME}_run 
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test.sh
  SOURCES test.sh.cmake
)

set_target_properties(${PROJECT_NAME}_provider PROPERTIES OUTPUT_NAME "provider")
set_target_properties(${PROJECT_NAME}_publisher PROPERTIES OUTPUT_NAME "publisher")
set_target_properties(${PROJECT_NAME}_requestor PROPERTIES OUTPUT_NAME "requestor")
set_target_properties(${PROJECT_NAME}_subscriber PROPERTIES OUTPUT_NAME "subscriber")


add_dependencies(${PROJECT_NAME}_provider   ServiceDirectory)
add_dependencies(${PROJECT_NAME}_publisher  ServiceDirectory)
add_dependencies(${PROJECT_NAME}_requestor  ServiceDirectory)
add_dependencies(${PROJECT_NAME}_subscriber ServiceDirectory)
add_dependencies(${PROJECT_NAME}_run 
  ServiceDirectory
  ${PROJECT_NAME}_provider
  ${PROJECT_NAME}_publisher
  ${PROJECT_NAME}_requestor
  ${PROJECT_NAME}_subscriber
)

target_include_directories(${PROJECT_NAME}_requestor
  PRIVATE
  ${gravity_SOURCE_DIR}/test
)

target_include_directories(${PROJECT_NAME}_subscriber
  PRIVATE
  ${gravity_SOURCE_DIR}/test
)

target_link_libraries(${PROJECT_NAME}_provider PRIVATE 
  gravity
  keyvalue_parser
  zmq
  protobuf
  Threads::Threads
)

target_link_libraries(${PROJECT_NAME}_publisher PRIVATE 
  gravity
  keyvalue_parser
  zmq
  protobuf
  Threads::Threads
)

target_link_libraries(${PROJECT_NAME}_requestor PRIVATE 
  gravity
  keyvalue_parser
  zmq
  protobuf
  Threads::Threads
)

target_link_libraries(${PROJECT_NAME}_subscriber PRIVATE 
  gravity
  keyvalue_parser
  zmq
  protobuf
  Threads::Threads
)

