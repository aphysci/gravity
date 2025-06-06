# Gravity cmake module
#
# The following import targets are created
#
# ::
#
#   gravity
#
# This module sets the following variables in your project::
#
#   Gravity_FOUND - true if gravity found on the system
#   Gravity_INCLUDE_DIR - the directory containing gravity headers
#   Gravity_LIBRARY - 


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was GravityConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

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

include(CMakeFindDependencyMacro)

# TODO: the @*USING*@ variants aren't passed to the gravity build when done as an ExternalProject
set(GRAVITY_USING_EXTERNAL_ZEROMQ OFF)
set(GRAVITY_USING_EXTERNAL_SPDLOG OFF)
set(GRAVITY_USING_EXTERNAL_PROTOBUF OFF)
if(NOT TARGET gravity)

  # if external (aka downloaded) builds were done for these libraries, use the "CMake config" files
  # which are installed into GRAVITY_ROOT.
  if (GRAVITY_USING_EXTERNAL_PROTOBUF)
    find_dependency(Protobuf CONFIG)
  else()
    find_dependency(Protobuf)
  endif()
  if (GRAVITY_USING_EXTERNAL_ZEROMQ)
    find_dependency(ZeroMQ CONFIG)
  else()
    find_dependency(ZeroMQ)
  endif()

  include("${CMAKE_CURRENT_LIST_DIR}/GravityTargets.cmake")

  if (TARGET gravity)
    get_target_property(Gravity_INCLUDE_DIR gravity INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(Gravity_LIBRARY gravity LOCATION)
  endif()


  check_required_components(gravity)

endif()
