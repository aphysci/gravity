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

project(libkeyvalue_parser VERSION 1.0 LANGUAGES C)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

############################
#        Targets
############################
bison_target(parser keyvalue.y ${CMAKE_CURRENT_BINARY_DIR}/y.tab.c
  COMPILE_FLAGS "-y -dt")
flex_target(scanner keyvalue.l  ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
add_flex_bison_dependency(scanner parser)

add_library(kvp_objects OBJECT 
            keyvalue_parser.h params.h
            keyvalue_parser.c
            ${BISON_parser_OUTPUTS}
            ${FLEX_scanner_OUTPUTS})

# shared and static libraries built from the same object files
add_library(keyvalue_parser STATIC $<TARGET_OBJECTS:kvp_objects>)
add_library(kvp_shared SHARED $<TARGET_OBJECTS:kvp_objects>)

add_executable(test_parser keyvalue_main.c)
target_link_libraries(test_parser keyvalue_parser)
target_include_directories(test_parser PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR})

############################
#        Building
############################
target_include_directories(kvp_objects PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR})

# shared libraries need PIC
set_property(TARGET kvp_objects PROPERTY POSITION_INDEPENDENT_CODE 1)

set_target_properties(kvp_shared PROPERTIES OUTPUT_NAME "keyvalue_parser")
set_target_properties(test_parser PROPERTIES OUTPUT_NAME "keyvalue_parser")

############################
#    Install & export
############################

# 'make install' to the locations provided by GNUInstallDirs
install(TARGETS keyvalue_parser kvp_shared EXPORT KeyvalueParserConfig
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # Windows hack

# This makes the project importable from the install directory
install(EXPORT KeyvalueParserConfig DESTINATION cmake)
# and importable from the build directory
export(TARGETS keyvalue_parser kvp_shared FILE KeyvalueParserConfig.cmake)

