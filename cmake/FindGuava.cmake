#================================================
# Copyright (c) Applied Physcical Sciences, Corp.
#================================================
# - Try to find Guava
#
# Usage of this module as follows:
#
#     find_package(Guava)
#
# Variables defined by this module:
#
#  Guava_FOUND        Guava jar exists
#  Guava_JAR          Path to Guava jar
#

if (NOT GUAVA_ROOT)
  set (GUAVA_ROOT /usr CACHE STRING "guava directory")
endif()

set(_guava_versions
  "27.0-jre" "27.0.1-jre" "27.1-jre" "27.0.1.jre-redhat-00001"
  "26.0-jre" "25.1-jre" "25.0-jre" "24.1.1-jre" "24.1-jre" "24.0-jre"
  "23.6-jre" "23.5-jre" "23.4-jre" "23.3-jre" "23.2-jre" "23.1-jre" "23.0"
  "22.0" "21.0" "20.0" "20.0.0-redhat-1" "19.0.0-redhat-1" "19.0"
  "18.0.0-redhat-1" "18.0" "17.0" "16.0.1" "16.0.1-redhat-3" "16.0"
  "15.0" "14.0.1" "14.0.1-redhat-1" "14.0"
  "13.0.1-redhat-3" "13.0.1-redhat-2" "13.0.1-redhat-1" "13.0.1" "13.0"
  "12.0.1" "12.0"
  "11.0.2-redhat-2" "11.0.2-redhat-1" "11.0.2" "11.0.1" "11.0"
  "10.0.1" "10.0"
  )

set(_guava_names)
foreach( ver ${_quava_versions})
  list(APPEND _guava_names "quava-${ver}.jar")
endforeach()

find_file(Guava_JAR
  NAMES "guava.jar" ${_guava_names}
  HINTS 
  ${GUAVA_ROOT}/share/java
  ${GUAVA_ROOT}/share/maven/lib
  ${GUAVA_ROOT}/local/share/java
  ${GUAVA_ROOT}/local/share/maven/lib
  )


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (Guava
  FOUND_VAR Guava_FOUND
  REQUIRED_VARS Guava_JAR
  )


mark_as_advanced(
  Guava_FOUND
  Guava_JAR
  )


