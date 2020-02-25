cmake_minimum_required(VERSION 3.11)

project(gravity-java NONE)

find_package(GravityJava REQUIRED)
include(UseJava)

set(CMAKE_JAVA_INCLUDE_PATH "${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/protobuf-java.jar")
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
        
set(JAVA_PROTO_SRCS 
        src/java/com/aphysci/gravity/protobuf/ComponentDataLookupResponseContainer.java
        src/java/com/aphysci/gravity/protobuf/ComponentLookupRequestContainer.java
        src/java/com/aphysci/gravity/protobuf/ComponentServiceLookupResponseContainer.java
        src/java/com/aphysci/gravity/protobuf/FileArchiverControlRequestContainer.java
        src/java/com/aphysci/gravity/protobuf/FileArchiverControlResponseContainer.java
        src/java/com/aphysci/gravity/protobuf/GravityDataProductContainer.java
        src/java/com/aphysci/gravity/protobuf/GravityLogMessageContainer.java
        src/java/com/aphysci/gravity/protobuf/GravityMetricsDataContainer.java
        src/java/com/aphysci/gravity/protobuf/ServiceDirectoryBroadcastContainer.java
        src/java/com/aphysci/gravity/protobuf/ServiceDirectoryDomainUpdateContainer.java
        src/java/com/aphysci/gravity/protobuf/ServiceDirectoryMapContainer.java
        src/java/com/aphysci/gravity/protobuf/ServiceDirectoryRegistrationContainer.java
        src/java/com/aphysci/gravity/protobuf/ServiceDirectoryResponseContainer.java
        src/java/com/aphysci/gravity/protobuf/ServiceDirectoryUnregistrationContainer
        src/java/gravity/ConfigRequest.java)
set_source_files_properties(${JAVA_PROTO_SRCS} PROPERTIES GENERATED TRUE)

set(SWIG_JAVA_SRCS 
        src/java/com/aphysci/gravity/swig/CPPGravityHeartbeatListener.java
        src/java/com/aphysci/gravity/swig/CPPGravityLogger.java
        src/java/com/aphysci/gravity/swig/CPPGravityRequestor.java
        src/java/com/aphysci/gravity/swig/CPPGravityServiceProvider.java
        src/java/com/aphysci/gravity/swig/CPPGravitySubscriber.java
        src/java/com/aphysci/gravity/swig/CPPGravitySubscriptionMonitor.java
        src/java/com/aphysci/gravity/swig/gravity.java
        src/java/com/aphysci/gravity/swig/gravityConstants.java
        src/java/com/aphysci/gravity/swig/gravityJNI.java
        src/java/com/aphysci/gravity/swig/GravityNode.java
        src/java/com/aphysci/gravity/swig/GravityReturnCode.java
        src/java/com/aphysci/gravity/swig/GravityTransportType.java
        src/java/com/aphysci/gravity/swig/Log.java)
set_source_files_properties(${SWIG_JAVA_SRCS} PROPERTIES GENERATED TRUE)

add_jar(${PROJECT_NAME} ${JAVA_SRCS} ${JAVA_PROTO_SRCS} ${SWIG_JAVA_SRCS} OUTPUT_NAME gravity)
install_jar(${PROJECT_NAME} "${CMAKE_INSTALL_PREFIX}/lib")
gravity_add_dependency(${PROJECT_NAME})