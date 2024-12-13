cmake_minimum_required(VERSION 3.17)

project(gravity-java NONE)

find_package(GravityJava REQUIRED)
include(UseJava)

gravity_find_protobuf(ON)
    
set(PUBLIC_JAR "/usr/share/java/protobuf.jar" "${Protobuf_SRC_ROOT_FOLDER}/java/target/protobuf-java-${Protobuf_VERSION}.jar")

set(CMAKE_JAVA_INCLUDE_PATH "${CMAKE_INSTALL_PREFIX}/lib/protobuf-java.jar" ${PUBLIC_JAR})
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
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/ComponentDataLookupResponseContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/ComponentLookupRequestContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/ComponentServiceLookupResponseContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/FileArchiverControlRequestContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/FileArchiverControlResponseContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/GravityDataProductContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/GravityLogMessageContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/GravityMetricsDataContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/ServiceDirectoryBroadcastContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/ServiceDirectoryDomainUpdateContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/ServiceDirectoryMapContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/ServiceDirectoryRegistrationContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/ServiceDirectoryResponseContainer.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/protobuf/ServiceDirectoryUnregistrationContainer.java"
	"${CMAKE_BINARY_DIR}/java/gravity/ConfigRequest.java")
set_source_files_properties(${JAVA_PROTO_SRCS} PROPERTIES GENERATED TRUE)

set(SWIG_JAVA_SRCS 
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/CPPGravityHeartbeatListener.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/CPPGravityLogger.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/CPPGravityRequestor.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/CPPGravityServiceProvider.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/CPPGravitySubscriber.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/CPPGravitySubscriptionMonitor.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/GravityNode.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/GravityReturnCode.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/GravityTransportType.java"
	"${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/Log.java")
set_source_files_properties(${SWIG_JAVA_SRCS} PROPERTIES GENERATED TRUE)

set(SWIG_CONFIG_JAVA_SRCS "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravity.java"
                          "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravity_d.java"
                          "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravityJNI.java"
                          "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravity_dJNI.java"
                          "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravityConstants.java"
                          "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravity_dConstants.java"
                          )
set_source_files_properties(${SWIG_CONFIG_JAVA_SRCS} PROPERTIES GENERATED TRUE)

gravity_add_jar_debug_release(${PROJECT_NAME} SOURCES ${JAVA_SRCS} ${JAVA_PROTO_SRCS} ${SWIG_JAVA_SRCS} DEBUG_SOURCES 
            "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravity_d.java"
            "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravity_dJNI.java"
            "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravity_dConstants.java"
            RELEASE_SOURCES
            "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravityJNI.java"
            "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravity.java" 
            "${CMAKE_BINARY_DIR}/java/com/aphysci/gravity/swig/gravityConstants.java"
            OUTPUT_NAME gravity)

gravity_install_jar(${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
gravity_add_dependency(${PROJECT_NAME})
