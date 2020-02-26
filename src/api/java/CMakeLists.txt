cmake_minimum_required(VERSION 3.11)

project(gravity-java NONE)

find_package(GravityJava REQUIRED)
include(UseJava)

set(CMAKE_JAVA_INCLUDE_PATH "${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/protobuf-java.jar")
set(CMAKE_JAVA_COMPILE_FLAGS -sourcepath "${CMAKE_CURRENT_LIST_DIR}/src/java")

set(JAVA_SRCS
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/FutureResponse.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/GravityDataProduct.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/GravityHeartbeatListener.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/GravityRequestor.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/GravityServiceProvider.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/GravitySubscriber.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/GravitySubscriptionMonitor.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/Logger.java")
        
set(JAVA_PROTO_SRCS 
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/ComponentDataLookupResponseContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/ComponentLookupRequestContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/ComponentServiceLookupResponseContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/FileArchiverControlRequestContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/FileArchiverControlResponseContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/GravityDataProductContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/GravityLogMessageContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/GravityMetricsDataContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/ServiceDirectoryBroadcastContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/ServiceDirectoryDomainUpdateContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/ServiceDirectoryMapContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/ServiceDirectoryRegistrationContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/ServiceDirectoryResponseContainer.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/protobuf/ServiceDirectoryUnregistrationContainer"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/gravity/ConfigRequest.java")
set_source_files_properties(${JAVA_PROTO_SRCS} PROPERTIES GENERATED TRUE)

set(SWIG_JAVA_SRCS 
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/CPPGravityHeartbeatListener.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/CPPGravityLogger.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/CPPGravityRequestor.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/CPPGravityServiceProvider.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/CPPGravitySubscriber.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/CPPGravitySubscriptionMonitor.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/gravity.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/gravityConstants.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/gravityJNI.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/GravityNode.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/GravityReturnCode.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/GravityTransportType.java"
        "${CMAKE_CURRENT_LIST_DIR}/src/java/com/aphysci/gravity/swig/Log.java")
set_source_files_properties(${SWIG_JAVA_SRCS} PROPERTIES GENERATED TRUE)

add_jar(${PROJECT_NAME} ${JAVA_SRCS} ${JAVA_PROTO_SRCS} ${SWIG_JAVA_SRCS} OUTPUT_NAME gravity)
install_jar(${PROJECT_NAME} "${CMAKE_INSTALL_PREFIX}/lib")
gravity_add_dependency(${PROJECT_NAME})