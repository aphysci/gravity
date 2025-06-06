# GravityKeyValueParser cmake module
#
# The following import targets are created
#
# ::
#
#   keyvalue_parser
#
# This module sets the following variables in your project::
#
#   GravityKeyValueParser_FOUND - true if keyvalue_parser found on the system
#   GravityKeyValueParser_INCLUDE_DIR - the directory containing keyvalue_parser headers
#   GravityKeyValueParser_LIBRARY - 


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was GravityKeyValueParserConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

if(NOT TARGET keyvalue_parser)
  include("${CMAKE_CURRENT_LIST_DIR}/GravityKeyValueParserTargets.cmake")

  if (TARGET keyvalue_parser)
    get_target_property(GravityKeyValueParser_INCLUDE_DIR keyvalue_parser INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(GravityKeyValueParser_LIBRARY keyvalue_parser LOCATION)
  endif()
endif()
