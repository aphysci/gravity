cmake_minimum_required(VERSION 3.11)

include(FetchContent)
include(GravitySupport)
project(gravity)

find_package(protobuf REQUIRED PATHS "${CMAKE_INSTALL_PREFIX}/deps/protobuf/cmake")
find_package(ZeroMQ REQUIRED PATHS "${CMAKE_INSTALL_PREFIX}/deps/libzmq/CMake")
find_package(Boost REQUIRED)

gravity_protobuf_generate(APPEND_PATH PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}/protobuf" OUT_VAR PROTO_SRCS PROTOS 
                    ../protobufs/ComponentDataLookupResponsePB.proto 
                    ../protobufs/ComponentLookupRequestPB.proto 
                    ../protobufs/ComponentServiceLookupResponsePB.proto 
                    ../protobufs/GravityDataProductPB.proto 
                    ../protobufs/GravityDataProductPB.proto 
                    ../protobufs/GravityLogMessagePB.proto 
                    ../protobufs/GravityMetricsDataPB.proto
                    ../protobufs/ServiceDirectoryRegistrationPB.proto 
                    ../protobufs/ServiceDirectoryResponsePB.proto 
                    ../protobufs/ServiceDirectoryUnregistrationPB.proto 
                    ../protobufs/ServiceDirectoryMapPB.proto
                    ../protobufs/ServiceDirectoryDomainUpdatePB.proto
                    ../protobufs/ConfigRequest.proto
                    ../protobufs/ServiceDirectoryBroadcastPB.proto
                    ../protobufs/FileArchiverControlRequestPB.proto
                    ../protobufs/FileArchiverControlResponsePB.proto
                    ../protobufs/ConfigRequest.proto)
                    
add_custom_target(${PROJECT_NAME}_protos ALL DEPENDS "${PROTO_SRCS}")

if (WIN32)
    find_package(PThreadsWin32 REQUIRED)
    include_directories("${PThreadsWin32_Include}")
endif()
    

set(HDRS
        CommUtil.h
        DomainDataKey.h
        FutureResponse.h
        GravityConfigParser.h
        GravityDataProduct.h
        GravityHeartbeat.h
        GravityHeartbeatListener.h
        GravityLogger.h
        GravityMetrics.h
        GravityMetricsManager.h
        GravityMetricsUtil.h
        GravityNode.h
        GravityPublishManager.h
        GravityRequestManager.h
        GravityRequestor.h
        GravitySemaphore.h
        GravityServiceManager.h
        GravityServiceProvider.h
        GravitySubscriber.h
        GravitySubscriptionManager.h
        GravitySubscriptionMonitor.h
        Utility.h)

set(SRCS
        ${HDRS}
        CommUtil.cpp
        DomainDataKey.cpp
        FutureResponse.cpp
        GravityConfigParser.cpp
        GravityDataProduct.cpp
        GravityHeartbeat.cpp
        GravityLogger.cpp
        GravityMetrics.cpp
        GravityMetricsManager.cpp
        GravityMetricsUtil.cpp
        GravityNode.cpp
        GravityPublishManager.cpp
        GravityRequestManager.cpp
        GravityRequestor.cpp
        GravityServiceManager.cpp
        GravityServiceProvider.cpp
        GravitySubscriber.cpp
        GravitySubscriptionManager.cpp
        GravitySubscriptionMonitor.cpp
        Semaphore.cpp
        Utility.cpp
        ${PROTO_SRCS})
        
add_definitions(-DGRAVITY_EXPORTS)
include_directories(${Boost_INCLUDE_DIRS})
add_library(${PROJECT_NAME} SHARED ${SRCS})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_protos)
gravity_add_dependency(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} PUBLIC libkeyvalue_parser protobuf::libprotobuf libzmq)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PThreadsWin32Lib_Installed})
if (WIN32)
    target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${PThreadsWin32_Include}>
        $<INSTALL_INTERFACE:deps/pthreads-w32/include>
    )
endif()
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install(FILES ${HDRS} DESTINATION include)
install(EXPORT ${PROJECT_NAME} DESTINATION cmake)






