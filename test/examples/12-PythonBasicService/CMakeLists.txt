cmake_minimum_required(VERSION 3.17)

include(GravitySupport)
project(PythonBasicService)

set(PY_EXEC python3)
if (WIN32)
    set(PY_EXEC python)
endif()
add_test(NAME pybasicservice COMMAND ${PY_EXEC} requestor.py WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
set_property(TEST pybasicservice PROPERTY ENVIRONMENT_MODIFICATION  
  "LD_LIBRARY_PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib;LD_LIBRRARY_PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib/gravity"
  "PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib;PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib/gravity;PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/bin;PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/deps/libzmq/bin;PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/deps/spdlog/bin;PATH=path_list_append:${CMAKE_INSTALL_PREFIX}/deps/pthreads-w32/bin"
  "PYTHONPATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib;PYTHONPATH=path_list_append:${CMAKE_INSTALL_PREFIX}/lib/gravity;PYTHONPATH=path_list_append:${CMAKE_INSTALL_PREFIX}/deps/protobuf/lib/;PYTHONPATH=path_list_append:${CMAKE_CURRENT_LIST_DIR}/../protobuf;PYTHONPATH=path_list_append:${CMAKE_CURRENT_LIST_DIR}")
