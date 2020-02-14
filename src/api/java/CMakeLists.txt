cmake_minimum_required(VERSION 3.11)

project(gravity-java NONE)

find_package(GravityJava REQUIRED)
include(UseJava)

set(CMAKE_JAVA_INCLUDE_PATH "${CMAKE_INSTALL_PREFIX}/deps/protobuf/bin/protobuf-java.jar")
set(CMAKE_JAVA_COMPILE_FLAGS -sourcepath "${CMAKE_CURRENT_LIST_DIR}/src/java")

set(JAVA_SRCS
        src/java/com/aphysci/gravity/FutureResponse.java
        src/java/com/aphysci/gravity/GravityDataProduct.java
        src/java/com/aphysci/gravity/GravityHeartbeatListener.java
        src/java/com/aphysci/gravity/GravityRequestor.java
        src/java/com/aphysci/gravity/GravityServiceProvider.java
        src/java/com/aphysci/gravity/GravitySubscriber.java
        src/java/com/aphysci/gravity/GravitySubscriptionMonitor.java
        src/java/com/aphysci/gravity/Logger.java)
        
add_jar(${PROJECT_NAME} ${JAVA_SRCS} OUTPUT_NAME gravity OUTPUT_DIR "${CMAKE_INSTALL_PREFIX}/lib")
install_jar(${PROJECT_NAME} "${CMAKE_INSTALL_PREFIX}/lib")
gravity_add_dependency(${PROJECT_NAME})