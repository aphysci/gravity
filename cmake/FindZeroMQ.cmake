

if (NOT GRAVITY_USE_EXTERNAL_ZEROMQ AND DEFINED ENV{ZMQ_HOME})
    set(zmq_dir $ENV{ZMQ_HOME})
elseif(ZMQ_HOME)
    set(zmq_dir "${ZMQ_HOME}")
else()
    set(zmq_dir)
endif()

string(REPLACE "\"" "" zmq_dir "${zmq_dir}")
file(TO_CMAKE_PATH "${zmq_dir}" zmq_dir)

include(SelectLibraryConfigurations)

function(_zeromq_find_libraries name filename)
  if(${name}_LIBRARIES)
    # Use result recorded by a previous call.
    return()
  elseif(${name}_LIBRARY)
    # Honor cache entry used by CMake 3.5 and lower.
    set(${name}_LIBRARIES "${${name}_LIBRARY}" PARENT_SCOPE)
  else()
    find_library(${name}_LIBRARY_RELEASE
      NAMES ${filename}
      PATHS "${zmq_dir}/lib")
    mark_as_advanced(${name}_LIBRARY_RELEASE)
        
    find_library(${name}_LIBRARY_DEBUG
      NAMES ${filename}d
      PATHS ${zmq_dir}/lib)
    mark_as_advanced(${name}_LIBRARY_DEBUG)
    
    if (MSVC)
        string(REPLACE "\." "_" ZeroMQ_FILE_VERSION "${ZeroMQ_VERSION}")
        if (NOT ${name}_LIBRARY_RELEASE)
            foreach(vcver v151 v150 v141 v140 v120 v110 v100 v90)
                file(GLOB ${name}_LIBRARY_RELEASE LIST_DIRECTORIES false "${zmq_dir}/lib/lib${filename}-${vcver}-mt-${ZeroMQ_FILE_VERSION}.lib")
                if (${name}_LIBRARY_RELEASE)
                    break()
                endif()
            endforeach()
        endif()
        
        set(${name}_LIBRARY_RELEASE ${${name}_LIBRARY_RELEASE} CACHE STRING "ZeroMQ release library" FORCE)
    
        if (NOT ${name}_LIBRARY_DEBUG)
            foreach(vcver v151 v150 v141 v140 v120 v110 v100 v90)
                file(GLOB ${name}_LIBRARY_DEBUG LIST_DIRECTORIES false "${zmq_dir}/lib/lib${filename}-${vcver}-mt-gd-${ZeroMQ_FILE_VERSION}.lib")
                if (${name}_LIBRARY_DEBUG)
                    break()
                endif()
            endforeach()
        endif()
        
        set(${name}_LIBRARY_DEBUG ${${name}_LIBRARY_DEBUG} CACHE STRING "ZeroMQ debug library" FORCE)
    endif()

    select_library_configurations(${name})

    if(UNIX AND Threads_FOUND)
      list(APPEND ${name}_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
    endif()

    set(${name}_LIBRARY "${${name}_LIBRARY}" PARENT_SCOPE)
    set(${name}_LIBRARIES "${${name}_LIBRARIES}" PARENT_SCOPE)
  endif()
endfunction()

find_path(ZeroMQ_INCLUDE_DIR
    zmq.h
    PATHS "${zmq_dir}/include"
)

if (ZeroMQ_INCLUDE_DIR)
    set(ZeroMQ_VERSION "")
    file(STRINGS ${ZeroMQ_INCLUDE_DIR}/zmq.h ZeroMQ_VERSION_CONTENTS REGEX "#define[ \t]+ZMQ_VERSION_MAJOR[ \t]+")
    if(ZeroMQ_VERSION_CONTENTS MATCHES "#define[ \t]+ZMQ_VERSION_MAJOR[ \t]+([0-9]+)")
      set(ZMQ_VERSION_MAJOR "${CMAKE_MATCH_1}")
    endif()
    unset(ZeroMQ_VERSION_CONTENTS)
    
    file(STRINGS ${ZeroMQ_INCLUDE_DIR}/zmq.h ZeroMQ_VERSION_CONTENTS REGEX "#define[ \t]+ZMQ_VERSION_MINOR[ \t]+")
    if(ZeroMQ_VERSION_CONTENTS MATCHES "#define[ \t]+ZMQ_VERSION_MINOR[ \t]+([0-9]+)")
      set(ZMQ_VERSION_MINOR "${CMAKE_MATCH_1}")
    endif()
    unset(ZeroMQ_VERSION_CONTENTS)
    
    file(STRINGS ${ZeroMQ_INCLUDE_DIR}/zmq.h ZeroMQ_VERSION_CONTENTS REGEX "#define[ \t]+ZMQ_VERSION_PATCH[ \t]+")
    if(ZeroMQ_VERSION_CONTENTS MATCHES "#define[ \t]+ZMQ_VERSION_PATCH[ \t]+([0-9]+)")
      set(ZMQ_VERSION_PATCH "${CMAKE_MATCH_1}")
    endif()
    unset(ZeroMQ_VERSION_CONTENTS)
    
    set(ZeroMQ_VERSION "${ZMQ_VERSION_MAJOR}")
    if (ZMQ_VERSION_MINOR OR "${ZMQ_VERSION_MINOR}" STREQUAL "0")
        set(ZeroMQ_VERSION "${ZeroMQ_VERSION}.${ZMQ_VERSION_MINOR}")
        if (ZMQ_VERSION_PATCH OR "${ZMQ_VERSION_PATCH}" STREQUAL "0")
            set(ZeroMQ_VERSION "${ZeroMQ_VERSION}.${ZMQ_VERSION_PATCH}")
        endif()
    endif()
endif()

if(MSVC)
    set(ZeroMQ_ORIG_FIND_LIBRARY_PREFIXES "${CMAKE_FIND_LIBRARY_PREFIXES}")
    set(CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
endif()

if(UNIX)
  # Protobuf headers may depend on threading.
  find_package(Threads QUIET)
endif()

_zeromq_find_libraries(ZeroMQ zmq)

if(MSVC)
    set(CMAKE_FIND_LIBRARY_PREFIXES "${ZeroMQ_ORIG_FIND_LIBRARY_PREFIXES}")
endif()

find_path(ZeroMQ_INCLUDE_DIR
    zmq.h
    PATHS "${zmq_dir}/include"
)

mark_as_advanced(ZeroMQ_INCLUDE_DIR)


if(ZeroMQ_LIBRARY)
  if(NOT TARGET libzmq)
      add_library(libzmq UNKNOWN IMPORTED)
      set_target_properties(libzmq PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ZeroMQ_INCLUDE_DIR}")
      if(EXISTS "${ZeroMQ_LIBRARY}")
        set_target_properties(libzmq PROPERTIES
          IMPORTED_LOCATION "${ZeroMQ_LIBRARY}")
      endif()
      if(EXISTS "${ZeroMQ_LIBRARY_RELEASE}")
        set_property(TARGET libzmq APPEND PROPERTY
          IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(libzmq PROPERTIES
          IMPORTED_LOCATION_RELEASE "${ZeroMQ_LIBRARY_RELEASE}")
  get_filename_component(ZeroMQ_LIBRARY_DIRECTORY ${ZeroMQ_LIBRARY_RELEASE} DIRECTORY)
  set(ZeroMQ_LIBRARY_DIRECTORY "${ZeroMQ_LIBRARY_DIRECTORY}" CACHE ON FILEPATH)
      endif()
      if(EXISTS "${ZeroMQ_LIBRARY_DEBUG}")
        set_property(TARGET libzmq APPEND PROPERTY
          IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(libzmq PROPERTIES
          IMPORTED_LOCATION_DEBUG "${ZeroMQ_LIBRARY_DEBUG}")
      endif()
      if(UNIX AND TARGET Threads::Threads)
        set_property(TARGET libzmq APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES Threads::Threads)
      endif()
  endif()
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZeroMQ
    REQUIRED_VARS ZeroMQ_LIBRARIES ZeroMQ_INCLUDE_DIR
    VERSION_VAR ZeroMQ_VERSION
)

if(ZeroMQ_FOUND)
    set(ZeroMQ_INCLUDE_DIRS ${ZeroMQ_INCLUDE_DIR})
endif()


