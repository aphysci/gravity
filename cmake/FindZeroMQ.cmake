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
# - Try to find ZeroMQ headers and libraries
# 
# module adapted from https://github.com/zeromq/czmq/
#
# Usage of this module as follows:
#
#     find_package(ZeroMQ)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  ZeroMQ_ROOT               Set this variable to the root installation of
#                            ZeroMQ if the module has problems finding
#                            the proper installation path.
#
# Variables defined by this module:
#
#  ZeroMQ_FOUND              True if ZeroMQ libs/headers found
#  ZeroMQ_LIBRARIES          The ZeroMQ libraries
#  ZeroMQ_LIBRARY_PATH       Full path to the zmq library used
#  ZeroMQ_INCLUDE_DIRS       The location of ZeroMQ headers
#  ZeroMQ_VERSION            The version of ZeroMQ

if (NOT ZeroMQ_ROOT)
  set(ZeroMQ_ROOT "$ENV{ZeroMQ_ROOT}")
endif()

if (NOT MSVC)
  include(FindPkgConfig)
  pkg_check_modules(PC_ZeroMQ "libzmq")
  if (PC_ZeroMQ_FOUND)
    # add CFLAGS from pkg-config file, e.g. draft api.
    add_definitions(${PC_ZeroMQ_CFLAGS} ${PC_ZeroMQ_CFLAGS_OTHER})
    # some libraries install the headers is a subdirectory of the include dir
    # returned by pkg-config, so use a wildcard match to improve chances of finding
    # headers and SOs.
    set(PC_ZeroMQ_INCLUDE_HINTS ${PC_ZeroMQ_INCLUDE_DIRS} ${PC_ZeroMQ_INCLUDE_DIRS}/*)
    set(PC_ZeroMQ_LIBRARY_HINTS ${PC_ZeroMQ_LIBRARY_DIRS} ${PC_ZeroMQ_LIBRARY_DIRS}/*)
  endif(PC_ZeroMQ_FOUND)
endif (NOT MSVC)

find_path (
  ZeroMQ_INCLUDE_DIRS
  NAMES zmq.h
  HINTS 
    ${ZeroMQ_ROOT}/include
    ${PC_ZeroMQ_INCLUDE_HINTS}
    include
  )

if (MSVC)
  # libzmq dll/lib built with MSVC is named using the Boost convention.
  # https://github.com/zeromq/czmq/issues/577
  # https://github.com/zeromq/czmq/issues/1972
  if (MSVC_IDE)
    set(MSVC_TOOLSET "-${CMAKE_VS_PLATFORM_TOOLSET}")
  else ()
    set(MSVC_TOOLSET "")
  endif ()

  # Retrieve ZeroMQ version number from zmq.h
  file(STRINGS "${ZeroMQ_INCLUDE_DIRS}/zmq.h" zmq_version_defines
    REGEX "#define ZMQ_VERSION_(MAJOR|MINOR|PATCH)")
  foreach(ver ${zmq_version_defines})
    if(ver MATCHES "#define ZMQ_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
      set(ZMQ_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
  endforeach()

  set(_zmq_version ${ZMQ_VERSION_MAJOR}_${ZMQ_VERSION_MINOR}_${ZMQ_VERSION_PATCH})

  set(_zmq_debug_names
    "libzmq${MSVC_TOOLSET}-mt-gd-${_zmq_version}" # Debug, BUILD_SHARED
    "libzmq${MSVC_TOOLSET}-mt-sgd-${_zmq_version}" # Debug, BUILD_STATIC
    "libzmq-mt-gd-${_zmq_version}" # Debug, BUILD_SHARED
    "libzmq-mt-sgd-${_zmq_version}" # Debug, BUILD_STATIC
    )

  set(_zmq_release_names
    "libzmq${MSVC_TOOLSET}-mt-${_zmq_version}" # Release|RelWithDebInfo|MinSizeRel, BUILD_SHARED
    "libzmq${MSVC_TOOLSET}-mt-s-${_zmq_version}" # Release|RelWithDebInfo|MinSizeRel, BUILD_STATIC
    "libzmq-mt-${_zmq_version}" # Release|RelWithDebInfo|MinSizeRel, BUILD_SHARED
    "libzmq-mt-s-${_zmq_version}" # Release|RelWithDebInfo|MinSizeRel, BUILD_STATIC
    )

  find_library (ZeroMQ_LIBRARY_DEBUG
    NAMES ${_zmq_debug_names}
    )

  find_library (ZeroMQ_LIBRARY_RELEASE
    NAMES ${_zmq_release_names}
    )

  include(SelectLibraryConfigurations)
  select_library_configurations(ZeroMQ)
endif (MSVC)

if (NOT ZeroMQ_LIBRARIES)
  find_library (
    ZeroMQ_LIBRARIES
    NAMES libzmq zmq
    HINTS 
      ${ZeroMQ_ROOT}/lib
      ${PC_ZeroMQ_LIBRARY_HINTS})
endif ()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  ZeroMQ
  REQUIRED_VARS ZeroMQ_LIBRARIES ZeroMQ_INCLUDE_DIRS
)
mark_as_advanced(
  ZeroMQ_FOUND
  ZeroMQ_LIBRARIES ZeroMQ_INCLUDE_DIRS
)

