cmake_minimum_required(VERSION 3.17)

include(GravitySupport)

project(gravity-MATLAB NONE)

find_package(GravityJava REQUIRED)
include(UseJava)

set(CMAKE_JAVA_INCLUDE_PATH guava-13.0.1.jar)
set(CMAKE_JAVA_INCLUDE_TARGET gravity-java)

set(ORIG_CMAKE_DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set(CMAKE_DEBUG_POSTFIX)
gravity_add_jar_debug_release(MATLABGravitySubscriber MATLABGravitySubscriber.java)
gravity_add_jar_debug_release(MATLABProtobufGenerator MATLABProtobufGenerator.java)
set(CMAKE_DEBUG_POSTFIX ${ORIG_CMAKE_DEBUG_POSTFIX})

gravity_install_jar(MATLABGravitySubscriber DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/MATLAB/")
gravity_install_jar(MATLABProtobufGenerator DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/MATLAB/")
install(FILES guava-13.0.1.jar LICENSE-2.0.txt DESTINATION lib/MATLAB/)
gravity_add_dependency(MATLABGravitySubscriber)
gravity_add_dependency(MATLABProtobufGenerator)
