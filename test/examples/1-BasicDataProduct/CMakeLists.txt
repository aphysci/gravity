cmake_minimum_required(VERSION 3.11)

project(BasicDataProduct)

list(APPEND CMAKE_MODULE_PATH "${GRAVITY_ROOT}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${GRAVITY_ROOT}/cmake")

set(PThreadsWin32_DIR "${ABS_GRAVITY_ROOT}/deps/pthreads-win32")
find_package(PThreadsWin32 REQUIRED)
find_package(ZeroMQ REQUIRED PATHS "${ABS_GRAVITY_ROOT}/deps/libzmq/CMake")
find_package(protobuf REQUIRED PATHS "${ABS_GRAVITY_ROOT}/deps/protobuf/cmake")
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
install(TARGETS BasicDataProductPublisher BasicDataProductSubscriber DESTINATION bin)