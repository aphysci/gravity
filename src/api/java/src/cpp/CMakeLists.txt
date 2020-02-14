cmake_minimum_required(VERSION 3.11)

include(GravitySupport)

project(libgravity-wrap)

find_package(JNI REQUIRED)

set(SRCS
    CPPGravityHeartbeatListener.cpp
    CPPGravityLogger.cpp
    CPPGravityRequestor.cpp
    CPPGravityServiceProvider.cpp
    CPPGravitySubscriber.cpp
    CPPGravitySubscriptionMonitor.cpp
    gravity_wrap.cpp
    
    CPPGravitySubscriptionMonitor.h
    CPPGravityHeartbeatListener.h
    CPPGravityLogger.h
    CPPGravityRequestor.h
    CPPGravityServiceProvider.h
    CPPGravitySubscriber.h
    gravity_wrap.h)
    
get_filename_component(JavaBin_DIR "${Java_JAVAC_EXECUTABLE}" DIRECTORY)
get_filename_component(Java_DIR "${JavaBin_DIR}/../" ABSOLUTE)
include_directories(${JNI_INCLUDE_DIRS})

add_library(${PROJECT_NAME} SHARED ${SRCS})
target_link_libraries(${PROJECT_NAME} gravity)
gravity_add_dependency(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install(EXPORT ${PROJECT_NAME} DESTINATION cmake)


