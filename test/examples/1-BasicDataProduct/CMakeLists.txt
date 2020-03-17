cmake_minimum_required(VERSION 3.11)
include(GravitySupport)
project(BasicDataProduct)

set(PThreadsWin32_DIR "${ABS_GRAVITY_ROOT}/deps/pthreads-w32")
find_package(PThreadsWin32 REQUIRED)
gravity_find_protobuf(external ON)
gravity_find_zeromq(ZeroMQ_External ON)
find_package(GravityKeyValueParser REQUIRED)
find_package(Gravity REQUIRED)

if (EXISTS "${ABS_GRAVITY_ROOT}/deps/boost")
    set(BOOT_ROOT "${ABS_GRAVITY_ROOT}/deps/boost")
endif()

find_package(Boost REQUIRED)

set(CMAKE_DEBUG_POSTFIX _d)
add_executable(BasicDataProductPublisher BasicDataProductPublisher.cpp)
set_target_properties(BasicDataProductPublisher PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(BasicDataProductPublisher gravity)

add_executable(BasicDataProductSubscriber BasicDataProductSubscriber.cpp)
set_target_properties(BasicDataProductSubscriber PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(BasicDataProductSubscriber gravity)
install(TARGETS BasicDataProductPublisher BasicDataProductSubscriber DESTINATION examples)
