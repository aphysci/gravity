cmake_minimum_required(VERSION 3.11)

include(GravitySupport)

project(ServiceDirectory)

gravity_find_protobuf(external ON)
find_package(ZeroMQ REQUIRED PATHS "${CMAKE_INSTALL_PREFIX}/deps/libzmq/CMake")

set(PROTO_SRCS)
        
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ComponentDataLookupResponsePB.pb.cc
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ComponentLookupRequestPB.pb.cc
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ComponentServiceLookupResponsePB.pb.cc
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ServiceDirectoryBroadcastPB.pb.cc
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ServiceDirectoryDomainUpdatePB.pb.cc
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ServiceDirectoryMapPB.pb.cc
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ServiceDirectoryRegistrationPB.pb.cc
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ServiceDirectoryResponsePB.pb.cc
        #${CMAKE_CURRENT_LIST_DIR}/../../../api/cpp/protobuf/ServiceDirectoryUnregistrationPB.pb.cc)
        
set_source_files_properties(${PROTO_SRCS} PROPERTIES GENERATED TRUE)

set(SRCS
        ServiceDirectory.h
        ServiceDirectorySynchronizer.h
        ServiceDirectoryUDPBroadcaster.h
        ServiceDirectoryUDPReceiver.h
        
        ServiceDirectory.cpp
        ServiceDirectorySynchronizer.cpp
        ServiceDirectoryUDPBroadcaster.cpp
        ServiceDirectoryUDPReceiver.cpp
        
        ${PROTO_SRCS}
        )
include_directories("${CMAKE_BINARY_DIR}")        
add_executable(${PROJECT_NAME} ${SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(${PROJECT_NAME} gravity)
gravity_add_dependency(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
