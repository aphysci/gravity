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
# - Provide gravity version information
#
# Usage of this module as follows:
#
#     gravity_version(VAR)
#
# Variables defined by this module:
#
#  VAR                A complete version string
#  VAR_MAJOR          Major revision number
#  VAR_MINOR          Minor rev
#  VAR_PATCH          PAtch level
#

include (cmake/GetGitRevisionDescription.cmake)

function(gravity_version _var)
  set(VERSION_REGEX "^v(([0-9]+).([0-9]+).([0-9]+))$")
  git_describe(_latest_ver --tags --abbrev=0)
  if("GIT-NOTFOUND" STREQUAL ${_latest_ver})
    set(${_var}_MAJOR "${_latest_ver}" PARENT_SCOPE)
    set(${_var}_MINOR "${_latest_ver}" PARENT_SCOPE)
    set(${_var}_PATCH "${_latest_ver}" PARENT_SCOPE)
    set(${_var} "${_latest_ver}" PARENT_SCOPE)
    return()
  endif()
  string (REGEX REPLACE ${VERSION_REGEX} "\\1" out "${_latest_ver}")
  set(${_var} "${out}" PARENT_SCOPE)
  string (REGEX REPLACE ${VERSION_REGEX} "\\2" out "${_latest_ver}")
  set(${_var}_MAJOR "${out}" PARENT_SCOPE)

  string (REGEX REPLACE ${VERSION_REGEX} "\\3" out "${_latest_ver}")
  set(${_var}_MINOR "${out}" PARENT_SCOPE)

  string (REGEX REPLACE ${VERSION_REGEX} "\\4" out "${_latest_ver}")
  set(${_var}_PATCH "${out}" PARENT_SCOPE)
endfunction()


