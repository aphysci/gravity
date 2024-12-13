cmake_minimum_required(VERSION 3.17)

include(FetchContent)
include(GravitySupport)
include(CMakePackageConfigHelpers)
project(Gravity)

set(LIB_NAME gravity)
set(PROTO_LIB_NAME gravity_protobufs)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

gravity_find_protobuf(ON)
gravity_find_zeromq(ON)
gravity_find_spdlog(ON)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_LIST_DIR}/../protobufs/*.proto")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/protobuf")

gravity_protobuf_generate(APPEND_PATH PROTOC_OUT_DIR "${CMAKE_BINARY_DIR}/protobuf" OUT_VAR PROTO_SRCS 
                            EXPORT_MACRO GRAVITY_PROTOBUF_API
                            PROTOS ${PROTO_FILES})
                            
add_custom_target(${LIB_NAME}_protos ALL DEPENDS ${PROTO_SRCS})

if (WIN32)
    find_package(PThreadsWin32 REQUIRED)
    include_directories("${PThreadsWin32_Include}")
endif()
    

set(PROTO_HDRS
	"${CMAKE_BINARY_DIR}/protobuf/ComponentDataLookupResponsePB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/ComponentLookupRequestPB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/ComponentServiceLookupResponsePB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/GravityDataProductPB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/GravityLogMessagePB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/GravityConfigParamPB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/GravityMetricsDataPB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/ServiceDirectoryRegistrationPB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/ServiceDirectoryResponsePB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/ServiceDirectoryUnregistrationPB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/ServiceDirectoryMapPB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/ServiceDirectoryDomainUpdatePB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/ConfigRequest.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/ServiceDirectoryBroadcastPB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/FileArchiverControlRequestPB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/FileArchiverControlResponsePB.pb.h"
	"${CMAKE_BINARY_DIR}/protobuf/ConfigRequest.pb.h")

set(HDRS
	"${CMAKE_CURRENT_LIST_DIR}/CommUtil.h"
	"${CMAKE_CURRENT_LIST_DIR}/DomainDataKey.h"
	"${CMAKE_CURRENT_LIST_DIR}/FutureResponse.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityConfigParser.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityDataProduct.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityHeartbeat.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityHeartbeatListener.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityLogger.h"
	"${CMAKE_CURRENT_LIST_DIR}/SpdLog.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityMetrics.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityMetricsManager.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityMetricsUtil.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityNode.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityPublishManager.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityRequestManager.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityRequestor.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravitySemaphore.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityServiceManager.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravityServiceProvider.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravitySubscriber.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravitySubscriptionManager.h"
	"${CMAKE_CURRENT_LIST_DIR}/GravitySubscriptionMonitor.h"
	"${CMAKE_CURRENT_LIST_DIR}/Utility.h")

set(SRCS
        ${HDRS}
	"${CMAKE_CURRENT_LIST_DIR}/CommUtil.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/DomainDataKey.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/FutureResponse.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityConfigParser.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityDataProduct.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityHeartbeat.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityLogger.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/SpdLog.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityMetrics.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityMetricsManager.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityMetricsUtil.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityNode.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityPublishManager.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityRequestManager.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityRequestor.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityServiceManager.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravityServiceProvider.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravitySubscriber.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravitySubscriptionManager.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GravitySubscriptionMonitor.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/Semaphore.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/Utility.cpp")
        
# create gravity_protobuf library target and link against libprotobuf
add_library(${PROTO_LIB_NAME} SHARED ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(${PROTO_LIB_NAME} PUBLIC protobuf::libprotobuf)

add_dependencies(${PROTO_LIB_NAME} ${LIB_NAME}_protos)
install(TARGETS ${PROTO_LIB_NAME} EXPORT ${PROJECT_NAME}-targets ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)

# set GRAVITY_PROTOBUF_API compile-time definition
if (WIN32)
    target_compile_definitions(${PROTO_LIB_NAME} PRIVATE -DGRAVITY_PROTOBUF_API=__declspec\(dllexport\) INTERFACE -DGRAVITY_PROTOBUF_API=__declspec\(dllimport\))
else()
    target_compile_definitions(${PROTO_LIB_NAME} PUBLIC -DGRAVITY_PROTOBUF_API=)
endif()

add_definitions(-DGRAVITY_EXPORTS -D_USRDLL)
include_directories("${CMAKE_BINARY_DIR}")
add_library(${LIB_NAME} SHARED ${SRCS})
set_target_properties(${LIB_NAME} PROPERTIES INSTALL_RPATH "\$ORIGIN")
add_dependencies(${LIB_NAME} ${PROTO_LIB_NAME})
gravity_add_dependency(${LIB_NAME})
target_include_directories(${LIB_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(${LIB_NAME} PRIVATE spdlog::spdlog_header_only libzmq)
target_link_libraries(${LIB_NAME} PRIVATE ${PThreadsWin32Lib_Installed})
if (WIN32)
    target_link_libraries(${LIB_NAME} PUBLIC Ws2_32.lib)
endif()

target_link_libraries(${LIB_NAME} PUBLIC protobuf::libprotobuf ${PROTO_LIB_NAME} PRIVATE keyvalue_parser )
if (WIN32)
    target_include_directories(${LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${PThreadsWin32_Include}>
        $<INSTALL_INTERFACE:deps/pthreads-w32/include>
    )
endif()
install(TARGETS ${LIB_NAME} EXPORT ${PROJECT_NAME}-targets ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install(FILES ${HDRS} DESTINATION include)
install(FILES ${PROTO_HDRS} DESTINATION include/protobuf)

message(STATUS "GRAVITY_USING_EXTERNAL_ZEROMQ: ${GRAVITY_USING_EXTERNAL_ZEROMQ}")
message(STATUS "GRAVITY_USING_EXTERNAL_PROTOBUF: ${GRAVITY_USING_EXTERNAL_PROTOBUF}")
message(STATUS "GRAVITY_USING_EXTERNAL_SPDLOG: ${GRAVITY_USING_EXTERNAL_SPDLOG}")

configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION lib/cmake/gravity)
install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION lib/cmake/gravity)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION lib/cmake/gravity)

find_program(GIT_EXE git)
if (GIT_EXE)
    execute_process(
          COMMAND "${GIT_EXE}" status "${CMAKE_CURRENT_LIST_DIR}"
          RESULT_VARIABLE STATUS
          OUTPUT_VARIABLE OUTPUT2 
          ERROR_QUIET)
    if(STATUS AND NOT STATUS EQUAL 0)
        message(STATUS "${GIT_EXE} FAILED: ${STATUS}")
    else()
        message(STATUS "${GIT_EXE} SUCCESS: ${OUTPUT2}")
        add_custom_command(TARGET ${LIB_NAME} POST_BUILD COMMAND "${GIT_EXE}" describe --tag --dirty > "${CMAKE_BINARY_DIR}/VERSION.txt")
        INSTALL(FILES "${CMAKE_BINARY_DIR}/VERSION.txt" DESTINATION .)
    endif()
endif()






