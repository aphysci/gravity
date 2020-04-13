cmake_minimum_required(VERSION 3.17)

include(GravitySupport)

project(gravity-MATLAB NONE)

find_package(GravityJava REQUIRED)
include(UseJava)

get_property(GRAVITY_JAR TARGET gravity-java PROPERTY JAR_FILE)

set(CMAKE_JAVA_INCLUDE_PATH guava-13.0.1.jar "${GRAVITY_JAR}")

gravity_add_jar_release(MATLABGravitySubscriber MATLABGravitySubscriber.java)
gravity_add_jar_release(MATLABProtobufGenerator MATLABProtobufGenerator.java)

set(INSTALL_CONFIGS)
if (WIN32)
    set(INSTALL_CONFIGS CONFIGURATIONS Release RelWithDebInfo)
endif()
gravity_install_jar(MATLABGravitySubscriber DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/MATLAB/" ${INSTALL_CONFIGS})
gravity_install_jar(MATLABProtobufGenerator DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/MATLAB/" ${INSTALL_CONFIGS})
install(FILES guava-13.0.1.jar LICENSE-2.0.txt DESTINATION lib/MATLAB/ ${INSTALL_CONFIGS})
gravity_add_dependency(MATLABGravitySubscriber)
gravity_add_dependency(MATLABProtobufGenerator)

