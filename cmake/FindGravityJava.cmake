

if (JAVA_HOME)
    find_package(Java COMPONENTS Development PATHS "${JAVA_HOME}")
else()
    find_package(Java COMPONENTS Development)
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GravityJava REQUIRED_VARS Java_FOUND)
