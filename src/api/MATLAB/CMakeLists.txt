cmake_minimum_required(VERSION 3.11)

include(GravitySupport)

project(gravity-MATLAB NONE)

find_package(GravityJava REQUIRED)
include(UseJava)

get_property(GRAVITY_JAR TARGET gravity-java PROPERTY JAR_FILE)

set(CMAKE_JAVA_INCLUDE_PATH guava-13.0.1.jar "${GRAVITY_JAR}")

add_jar(MATLABGravitySubscriber MATLABGravitySubscriber.java)
add_jar(MATLABProtobufGenerator MATLABProtobufGenerator.java)
install_jar(MATLABGravitySubscriber "${CMAKE_INSTALL_PREFIX}/lib/MATLAB/")
install_jar(MATLABProtobufGenerator "${CMAKE_INSTALL_PREFIX}/lib/MATLAB/")
install(FILES guava-13.0.1.jar LICENSE-2.0.txt DESTINATION lib/MATLAB/)
gravity_add_dependency(MATLABGravitySubscriber)
gravity_add_dependency(MATLABProtobufGenerator)

