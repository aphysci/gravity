#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "keyvalue_parser" for configuration "Release"
set_property(TARGET keyvalue_parser APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(keyvalue_parser PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libkeyvalue_parser.a"
  )

list(APPEND _cmake_import_check_targets keyvalue_parser )
list(APPEND _cmake_import_check_files_for_keyvalue_parser "${_IMPORT_PREFIX}/lib/libkeyvalue_parser.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
