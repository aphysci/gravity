cmake_minimum_required(VERSION 3.17)

include(FetchContent)
include(GravitySupport)
include(CMakePackageConfigHelpers)
project(Gravity)

    
set(HDRS
	"${CMAKE_CURRENT_LIST_DIR}/bridge/RustFutureResponse.h"
	"${CMAKE_CURRENT_LIST_DIR}/bridge/RustGravityDataProduct.h"
	"${CMAKE_CURRENT_LIST_DIR}/bridge/RustGravityHeartbeatListener.h"
	"${CMAKE_CURRENT_LIST_DIR}/bridge/RustGravityNode.h"
	"${CMAKE_CURRENT_LIST_DIR}/bridge/RustGravityRequestor.h"
	"${CMAKE_CURRENT_LIST_DIR}/bridge/RustGravityServiceProvider.h"
	"${CMAKE_CURRENT_LIST_DIR}/bridge/RustGravitySubscriber.h"
	"${CMAKE_CURRENT_LIST_DIR}/bridge/RustGravitySubscriptionMonitor.h"
	"${CMAKE_CURRENT_LIST_DIR}/bridge/RustSpdLog.h")


        

install(FILES ${HDRS} DESTINATION include)




