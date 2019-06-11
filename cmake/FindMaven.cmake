#================================================
# Copyright (c) Applied Physcical Sciences, Corp.
#================================================
# - Try to find Maven
#
# Usage of this module as follows:
#
#     find_package(Maven)
#
# Variables defined by this module:
#
#  Maven_FOUND               Maven executable exists
#  Maven_EXECUTABLE          Path to Maven executable
#

set (MAVEN_ROOT /usr/bin CACHE STRING "maven directory")

find_program(Maven_EXECUTABLE NAMES mvn
  HINTS ENV${MAVEN_ROOT}/mvn ${MAVEN_ROOT}/mvn)

# handle the QUIETLY and REQUIRED arguments and set Maven_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (Maven
  FOUND_VAR Maven_FOUND
  REQUIRED_VARS Maven_EXECUTABLE
  )


if (Maven_FOUND)
  exec_program(env ARGS ${Maven_EXECUTABLE} -version OUTPUT_VARIABLE Maven_OUTPUT
    RETURN_VALUE Maven_RETURN)
  if (Maven_RETURN STREQUAL "0")
    string(REPLACE ";" " " Maven_OUTPUT2 ${Maven_OUTPUT})
    string(REPLACE "\n" ";" Maven_OUTPUT3 ${Maven_OUTPUT2})
    list(GET Maven_OUTPUT3 0 Maven_VERSION)
    set(Maven_FOUND TRUE)
    if (NOT Maven_FIND_QUIETLY)
      message(STATUS "Found Maven: ${Maven_EXECUTABLE} (found version \"${Maven_VERSION}\")")
    endif ()
  else ()
    message(STATUS "Maven: could not resolve maven version")
    set(Maven_FOUND FALSE)
  endif ()
endif()


mark_as_advanced(
  Maven_FOUND
  Maven_EXECUTABLE
  Maven_VERSION
  )


