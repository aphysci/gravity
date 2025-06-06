#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "gravity_protobufs" for configuration "Release"
set_property(TARGET gravity_protobufs APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(gravity_protobufs PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libgravity_protobufs.so"
  IMPORTED_SONAME_RELEASE "libgravity_protobufs.so"
  )

list(APPEND _cmake_import_check_targets gravity_protobufs )
list(APPEND _cmake_import_check_files_for_gravity_protobufs "${_IMPORT_PREFIX}/lib/libgravity_protobufs.so" )

# Import target "gravity" for configuration "Release"
set_property(TARGET gravity APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(gravity PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libgravity.so"
  IMPORTED_SONAME_RELEASE "libgravity.so"
  )

list(APPEND _cmake_import_check_targets gravity )
list(APPEND _cmake_import_check_files_for_gravity "${_IMPORT_PREFIX}/lib/libgravity.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
