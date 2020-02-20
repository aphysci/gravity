cmake_minimum_required(VERSION 3.11)

include(GravitySupport)
include(CMakePackageConfigHelpers)

project(GravityWrap)

set(LIB_NAME gravity_wrap)

find_package(JNI REQUIRED)

set(SRCS
    CPPGravityHeartbeatListener.cpp
    CPPGravityLogger.cpp
    CPPGravityRequestor.cpp
    CPPGravityServiceProvider.cpp
    CPPGravitySubscriber.cpp
    CPPGravitySubscriptionMonitor.cpp
    "${CMAKE_CURRENT_LIST_DIR}/gravity_wrap.cpp"
    
    CPPGravitySubscriptionMonitor.h
    CPPGravityHeartbeatListener.h
    CPPGravityLogger.h
    CPPGravityRequestor.h
    CPPGravityServiceProvider.h
    CPPGravitySubscriber.h
    "${CMAKE_CURRENT_LIST_DIR}/gravity_wrap.h")
    

include_directories(${JNI_INCLUDE_DIRS})
set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/gravity_wrap.cpp" "${CMAKE_CURRENT_LIST_DIR}/gravity_wrap.h" PROPERTIES GENERATED ON)
add_library(${LIB_NAME} SHARED ${SRCS})
target_link_libraries(${LIB_NAME} gravity)
gravity_add_dependency(${LIB_NAME})
install(TARGETS ${LIB_NAME} EXPORT ${PROJECT_NAME}-targets ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/../../../../../cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION cmake)
install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION cmake)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION cmake)


