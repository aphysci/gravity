cmake_minimum_required(VERSION 3.11)
include(GravitySupport)
project(BasicDataProduct)

find_package(PThreadsWin32 REQUIRED)
gravity_find_protobuf(ON)
gravity_find_zeromq(ON)
find_package(GravityKeyValueParser REQUIRED)
find_package(Gravity REQUIRED)

set(CMAKE_DEBUG_POSTFIX _d)
add_executable(BasicDataProductPublisher BasicDataProductPublisher.cpp)
set_target_properties(BasicDataProductPublisher PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(BasicDataProductPublisher gravity)

add_executable(BasicDataProductSubscriber BasicDataProductSubscriber.cpp)
set_target_properties(BasicDataProductSubscriber PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(BasicDataProductSubscriber gravity)
install(TARGETS BasicDataProductPublisher BasicDataProductSubscriber DESTINATION bin)
