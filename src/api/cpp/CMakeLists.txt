cmake_minimum_required(VERSION 3.11)

include(FetchContent)
include(GravitySupport)
include(CMakePackageConfigHelpers)
project(Gravity)

set(LIB_NAME gravity)
set(PROTO_LIB_NAME gravity_protobufs)

find_package(protobuf REQUIRED PATHS "${CMAKE_INSTALL_PREFIX}/deps/protobuf/cmake" "${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/cmake")
find_package(ZeroMQ REQUIRED PATHS "${CMAKE_INSTALL_PREFIX}/deps/libzmq/CMake" "${CMAKE_INSTALL_PREFIX}/deps/libzmq/share/cmake")
find_package(Boost REQUIRED)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/protobuf")
gravity_protobuf_generate(APPEND_PATH PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}/protobuf" OUT_VAR PROTO_SRCS 
                            EXPORT_MACRO GRAVITY_PROTOBUF_API
                            PROTOS 
                    ../protobufs/ComponentDataLookupResponsePB.proto 
                    ../protobufs/ComponentLookupRequestPB.proto 
                    ../protobufs/ComponentServiceLookupResponsePB.proto 
                    ../protobufs/GravityDataProductPB.proto  
                    ../protobufs/GravityLogMessagePB.proto 
                    ../protobufs/GravityMetricsDataPB.proto
                    ../protobufs/ConfigRequest.proto
                    ../protobufs/FileArchiverControlRequestPB.proto
                    ../protobufs/FileArchiverControlResponsePB.proto
                    ../protobufs/ConfigRequest.proto
                    ../protobufs/ServiceDirectoryResponsePB.proto
                    ../protobufs/ServiceDirectoryRegistrationPB.proto 
                    ../protobufs/ServiceDirectoryUnregistrationPB.proto
                    ../protobufs/ServiceDirectoryBroadcastPB.proto
                    ../protobufs/ServiceDirectoryMapPB.proto
                    ../protobufs/ServiceDirectoryDomainUpdatePB.proto
                    )
                            
add_custom_target(${LIB_NAME}_protos ALL DEPENDS ${PROTO_SRCS})

if (WIN32)
    find_package(PThreadsWin32 REQUIRED)
    include_directories("${PThreadsWin32_Include}")
endif()
    

set(PROTO_HDRS
        protobuf/ComponentDataLookupResponsePB.pb.h
        protobuf/ComponentLookupRequestPB.pb.h
        protobuf/ComponentServiceLookupResponsePB.pb.h
        protobuf/GravityDataProductPB.pb.h
        protobuf/GravityLogMessagePB.pb.h
        protobuf/GravityMetricsDataPB.pb.h
        protobuf/ServiceDirectoryRegistrationPB.pb.h
        protobuf/ServiceDirectoryResponsePB.pb.h
        protobuf/ServiceDirectoryUnregistrationPB.pb.h
        protobuf/ServiceDirectoryMapPB.pb.h
        protobuf/ServiceDirectoryDomainUpdatePB.pb.h
        protobuf/ConfigRequest.pb.h
        protobuf/ServiceDirectoryBroadcastPB.pb.h
        protobuf/FileArchiverControlRequestPB.pb.h
        protobuf/FileArchiverControlResponsePB.pb.h
        protobuf/ConfigRequest.pb.h)

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
        Utility.cpp)
        

        
add_library(${PROTO_LIB_NAME} SHARED ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(${PROTO_LIB_NAME} PUBLIC protobuf::libprotobuf)
add_dependencies(${PROTO_LIB_NAME} ${LIB_NAME}_protos)
install(TARGETS ${PROTO_LIB_NAME} EXPORT ${PROJECT_NAME}-targets ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)

if (WIN32)
    target_compile_definitions(${PROTO_LIB_NAME} PRIVATE -DGRAVITY_PROTOBUF_API=__declspec\(dllexport\) INTERFACE -DGRAVITY_PROTOBUF_API=__declspec\(dllimport\))
else()
    target_compile_definitions(${PROTO_LIB_NAME} PUBLIC -DGRAVITY_PROTOBUF_API=)
endif()

add_definitions(-DGRAVITY_EXPORTS -D_USRDLL)
include_directories(${Boost_INCLUDE_DIRS})
add_library(${LIB_NAME} SHARED ${SRCS})
add_dependencies(${LIB_NAME} ${PROTO_LIB_NAME})
gravity_add_dependency(${LIB_NAME})
target_include_directories(${LIB_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(${LIB_NAME} PUBLIC ${PROTO_LIB_NAME} keyvalue_parser protobuf::libprotobuf libzmq)
target_link_libraries(${LIB_NAME} PRIVATE ${PThreadsWin32Lib_Installed})
if (WIN32)
    target_include_directories(${LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${PThreadsWin32_Include}>
        $<INSTALL_INTERFACE:deps/pthreads-w32/include>
    )
endif()
install(TARGETS ${LIB_NAME} EXPORT ${PROJECT_NAME}-targets ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install(FILES ${HDRS} DESTINATION include)
install(FILES ${PROTO_HDRS} DESTINATION include/protobuf)
configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION cmake)
install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION cmake)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION cmake)






