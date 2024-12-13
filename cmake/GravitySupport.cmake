

macro(gravity_add_dependency project_name)
    get_property(previous GLOBAL PROPERTY gravity_previous_dependency)
    if (previous)
        add_dependencies(${project_name} ${previous})
    endif()
    set_property(GLOBAL PROPERTY gravity_previous_dependency ${project_name})
endmacro()

macro(gravity_find_protobuf fail_if_missing)
    find_package(Protobuf)
    if (${fail_if_missing} AND NOT Protobuf_FOUND)
        message(FATAL_ERROR "Failed to find protobuf libraries")
    endif()
endmacro()

macro(gravity_find_spdlog fail_if_missing)
    find_package(spdlog QUIET)
    if (${fail_if_missing} AND NOT spdlog_FOUND)
        message(FATAL_ERROR "Failed to find spdlog library")
    endif()
endmacro()


macro(gravity_find_zeromq fail_if_missing)
    find_package(ZeroMQ)
    if (${fail_if_missing} AND NOT TARGET libzmq)
        message(FATAL_ERROR "Failed to find ZeroMQ libraries")
    endif()
    if (TARGET libzmq)
        get_target_property(ZMQ_INCLUDE_DIR libzmq INTERFACE_INCLUDE_DIRECTORIES)
        if (ZMQ_INCLUDE_DIR)
            set(ZeroMQ_FOUND ON)
        endif()
    endif()
endmacro()

function(gravity_find_python_debug outvar)
    get_target_property(Python_LIBRARY_RELEASE Python::Python LOCATION_RELEASE)
    get_filename_component(Python_LIBRARY_FILENAME "${Python_LIBRARY_RELEASE}" NAME_WE)
    find_library(Python_LIBRARY_DEBUG NAMES ${Python_LIBRARY_FILENAME}_d NAMES_PER_DIR HINTS ${Python_LIBRARY_DIRS})
    set(${outvar} ${Python_LIBRARY_DEBUG} PARENT_SCOPE)
endfunction()

function(GRAVITY_INSTALL_JAR _TARGET_NAME)
    if (ARGC EQUAL 2)
      set (_DESTINATION ${ARGV1})
    else()
      cmake_parse_arguments(_install_jar
        ""
        "DESTINATION;COMPONENT;CONFIGURATIONS"
        ""
        ${ARGN})
      if (_install_jar_DESTINATION)
        set (_DESTINATION ${_install_jar_DESTINATION})
      else()
        message(SEND_ERROR "install_jar: ${_TARGET_NAME}: DESTINATION must be specified.")
      endif()

      if (_install_jar_COMPONENT)
        set (_COMPONENT COMPONENT ${_install_jar_COMPONENT})
      endif()
      
    endif()

    get_property(__FILES_DEBUG
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            JAR_FILE_DEBUG)
    get_property(__FILES_RELEASE
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            JAR_FILE_RELEASE)
    set_property(
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            INSTALL_DESTINATION
            ${_DESTINATION}
    )
    if (__FILES_DEBUG)
        install(
            FILES
                ${__FILES_DEBUG}
            DESTINATION
                ${_DESTINATION}
            ${_COMPONENT}
            CONFIGURATIONS Debug
        )
    endif()
    
    if (__FILES_RELEASE)
        install(
            FILES
                ${__FILES_RELEASE}
            DESTINATION
                ${_DESTINATION}
            ${_COMPONENT}
            CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
    else ()
        message(SEND_ERROR "install_jar: The target ${_TARGET_NAME} is not known in this scope.")
    endif ()
endfunction()

function(gravity_protobuf_generate)
  include(CMakeParseArguments)

  set(_options APPEND_PATH)
  set(_singleargs LANGUAGE OUT_VAR EXPORT_MACRO PROTOC_OUT_DIR)
  if(COMMAND target_sources)
    list(APPEND _singleargs TARGET)
  endif()
  set(_multiargs PROTOS IMPORT_DIRS GENERATE_EXTENSIONS)

  cmake_parse_arguments(protobuf_generate "${_options}" "${_singleargs}" "${_multiargs}" "${ARGN}")

  if(NOT protobuf_generate_PROTOS AND NOT protobuf_generate_TARGET)
    message(SEND_ERROR "Error: protobuf_generate called without any targets or source files")
    return()
  endif()

  if(NOT protobuf_generate_OUT_VAR AND NOT protobuf_generate_TARGET)
    message(SEND_ERROR "Error: protobuf_generate called without a target or output variable")
    return()
  endif()

  if(NOT protobuf_generate_LANGUAGE)
    set(protobuf_generate_LANGUAGE cpp)
  endif()
  string(TOLOWER ${protobuf_generate_LANGUAGE} protobuf_generate_LANGUAGE)

  if(NOT protobuf_generate_PROTOC_OUT_DIR)
    set(protobuf_generate_PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  if(WIN32 AND protobuf_generate_EXPORT_MACRO AND protobuf_generate_LANGUAGE STREQUAL cpp)
    set(_dll_export_decl "dllexport_decl=${protobuf_generate_EXPORT_MACRO}:")
  else ()
    set(_dll_export_decl)
  endif()

  if(NOT protobuf_generate_GENERATE_EXTENSIONS)
    if(protobuf_generate_LANGUAGE STREQUAL cpp)
      set(protobuf_generate_GENERATE_EXTENSIONS .pb.h .pb.cc)
    elseif(protobuf_generate_LANGUAGE STREQUAL python)
      set(protobuf_generate_GENERATE_EXTENSIONS _pb2.py)
    else()
      message(SEND_ERROR "Error: protobuf_generate given unknown Language ${LANGUAGE}, please provide a value for GENERATE_EXTENSIONS")
      return()
    endif()
  endif()

  if(protobuf_generate_TARGET)
    get_target_property(_source_list ${protobuf_generate_TARGET} SOURCES)
    foreach(_file ${_source_list})
      if(_file MATCHES "proto$")
        list(APPEND protobuf_generate_PROTOS ${_file})
      endif()
    endforeach()
  endif()

  if(NOT protobuf_generate_PROTOS)
    message(SEND_ERROR "Error: protobuf_generate could not find any .proto files")
    return()
  endif()

  if(protobuf_generate_APPEND_PATH)
    # Create an include path for each file specified
    foreach(_file ${protobuf_generate_PROTOS})
      get_filename_component(_abs_file ${_file} ABSOLUTE)
      get_filename_component(_abs_path ${_abs_file} PATH)
      list(FIND _protobuf_include_path ${_abs_path} _contains_already)
      if(${_contains_already} EQUAL -1)
          list(APPEND _protobuf_include_path -I ${_abs_path})
      endif()
    endforeach()
  else()
    set(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  foreach(DIR ${protobuf_generate_IMPORT_DIRS})
    get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
    list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
    if(${_contains_already} EQUAL -1)
        list(APPEND _protobuf_include_path -I ${ABS_PATH})
    endif()
  endforeach()

  set(_generated_srcs_all)
  foreach(_proto ${protobuf_generate_PROTOS})
    get_filename_component(_abs_file ${_proto} ABSOLUTE)
    get_filename_component(_abs_dir ${_abs_file} DIRECTORY)
    get_filename_component(_basename ${_proto} NAME_WE)
    file(RELATIVE_PATH _rel_dir ${CMAKE_CURRENT_SOURCE_DIR} ${_abs_dir})

    set(_generated_srcs)
    foreach(_ext ${protobuf_generate_GENERATE_EXTENSIONS})
      set(_tmpbasename "${_basename}")
      if ("${_ext}" MATCHES ".java$")
        if (_tmpbasename MATCHES "PB$")
          string(REPLACE "PB" "Container" _tmpbasename "${_basename}")
          set(_tmpbasename "com/aphysci/gravity/protobuf/${_tmpbasename}")
        elseif(_tmpbasename MATCHES "ConfigRequest")
          set(_tmpbasename "gravity/${_tmpbasename}")
        elseif (_tmpbasename MATCHES "descriptor$")
            set(_tmpbasename "com/google/protobuf/DescriptorProtos")
        endif()
      endif()
      set(_tmpstr "${protobuf_generate_PROTOC_OUT_DIR}/${_tmpbasename}${_ext}")
      string(REPLACE "//" "/" _tmpstr "${_tmpstr}")
      list(APPEND _generated_srcs "${_tmpstr}")
    endforeach()
    list(APPEND _generated_srcs_all ${_generated_srcs})
    
    get_target_property(PROTOC_EXE protobuf::protoc LOCATION) 
    add_custom_command(
      OUTPUT ${_generated_srcs}
      COMMAND  ${CMAKE_COMMAND} -E env ${PROTOC_EXE}
      ARGS --${protobuf_generate_LANGUAGE}_out ${_dll_export_decl}${protobuf_generate_PROTOC_OUT_DIR} ${_protobuf_include_path} ${_abs_file}
      DEPENDS ${_abs_file} protobuf::protoc
      COMMENT "Running ${protobuf_generate_LANGUAGE} protocol buffer compiler on ${_proto}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${_generated_srcs_all} PROPERTIES GENERATED TRUE)
  if(protobuf_generate_OUT_VAR)
    set(${protobuf_generate_OUT_VAR} ${_generated_srcs_all} PARENT_SCOPE)
  endif()
  if(protobuf_generate_TARGET)
    target_sources(${protobuf_generate_TARGET} PRIVATE ${_generated_srcs_all})
  endif()

endfunction()

function(gravity_add_jar_debug_release _TARGET_NAME)

    cmake_parse_arguments(_add_jar
      ""
      "VERSION;OUTPUT_DIR;OUTPUT_NAME;ENTRY_POINT;MANIFEST"
      "SOURCES;DEBUG_SOURCES;RELEASE_SOURCES;INCLUDE_JARS;GENERATE_NATIVE_HEADERS"
      ${ARGN}
    )

    # In CMake < 2.8.12, add_jar used variables which were set prior to calling
    # add_jar for customizing the behavior of add_jar. In order to be backwards
    # compatible, check if any of those variables are set, and use them to
    # initialize values of the named arguments. (Giving the corresponding named
    # argument will override the value set here.)
    #
    # New features should use named arguments only.
    if(NOT DEFINED _add_jar_VERSION AND DEFINED CMAKE_JAVA_TARGET_VERSION)
        set(_add_jar_VERSION "${CMAKE_JAVA_TARGET_VERSION}")
    endif()
    if(NOT DEFINED _add_jar_OUTPUT_DIR AND DEFINED CMAKE_JAVA_TARGET_OUTPUT_DIR)
        set(_add_jar_OUTPUT_DIR "${CMAKE_JAVA_TARGET_OUTPUT_DIR}")
    endif()
    if(NOT DEFINED _add_jar_OUTPUT_NAME AND DEFINED CMAKE_JAVA_TARGET_OUTPUT_NAME)
        set(_add_jar_OUTPUT_NAME "${CMAKE_JAVA_TARGET_OUTPUT_NAME}")
        # reset
        set(CMAKE_JAVA_TARGET_OUTPUT_NAME)
    endif()
    if(NOT DEFINED _add_jar_ENTRY_POINT AND DEFINED CMAKE_JAVA_JAR_ENTRY_POINT)
        set(_add_jar_ENTRY_POINT "${CMAKE_JAVA_JAR_ENTRY_POINT}")
    endif()

    set(_JAVA_SOURCE_FILES_DEBUG ${_add_jar_DEBUG_SOURCES})
    set(_JAVA_SOURCE_FILES_RELEASE ${_add_jar_RELEASE_SOURCES})
    set(_JAVA_SOURCE_FILES ${_add_jar_SOURCES} ${_add_jar_UNPARSED_ARGUMENTS})

    if (NOT DEFINED _add_jar_OUTPUT_DIR)
        set(_add_jar_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    else()
        get_filename_component(_add_jar_OUTPUT_DIR ${_add_jar_OUTPUT_DIR} ABSOLUTE)
    endif()
    # ensure output directory exists
    file (MAKE_DIRECTORY "${_add_jar_OUTPUT_DIR}")

    if (_add_jar_ENTRY_POINT)
        set(_ENTRY_POINT_OPTION e)
        set(_ENTRY_POINT_VALUE ${_add_jar_ENTRY_POINT})
    endif ()

    if (_add_jar_MANIFEST)
        set(_MANIFEST_OPTION m)
        get_filename_component (_MANIFEST_VALUE "${_add_jar_MANIFEST}" ABSOLUTE)
    endif ()

    unset (_GENERATE_NATIVE_HEADERS)
    if (_add_jar_GENERATE_NATIVE_HEADERS)
      # Raise an error if JDK version is less than 1.8 because javac -h is not supported
      # by earlier versions.
      if (Java_VERSION VERSION_LESS 1.8)
        message (FATAL_ERROR "ADD_JAR: GENERATE_NATIVE_HEADERS is not supported with this version of Java.")
      endif()
      cmake_parse_arguments (_add_jar_GENERATE_NATIVE_HEADERS "" "DESTINATION" "" ${_add_jar_GENERATE_NATIVE_HEADERS})
      if (NOT _add_jar_GENERATE_NATIVE_HEADERS_UNPARSED_ARGUMENTS)
        message (FATAL_ERROR "ADD_JAR: GENERATE_NATIVE_HEADERS: missing required argument.")
      endif()
      list (LENGTH _add_jar_GENERATE_NATIVE_HEADERS_UNPARSED_ARGUMENTS length)
      if (length GREATER 1)
        list (REMOVE_AT _add_jar_GENERATE_NATIVE_HEADERS_UNPARSED_ARGUMENTS 0)
        message (FATAL_ERROR "ADD_JAR: GENERATE_NATIVE_HEADERS: ${_add_jar_GENERATE_NATIVE_HEADERS_UNPARSED_ARGUMENTS}: unexpected argument(s).")
      endif()
      if (NOT _add_jar_GENERATE_NATIVE_HEADERS_DESTINATION)
        set (_add_jar_GENERATE_NATIVE_HEADERS_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${_TARGET_NAME}.dir/native_headers")
      endif()

      set (_GENERATE_NATIVE_HEADERS_TARGET ${_add_jar_GENERATE_NATIVE_HEADERS_UNPARSED_ARGUMENTS})
      set (_GENERATE_NATIVE_HEADERS_OUTPUT_DIR "${_add_jar_GENERATE_NATIVE_HEADERS_DESTINATION}")
      set (_GENERATE_NATIVE_HEADERS -h "${_GENERATE_NATIVE_HEADERS_OUTPUT_DIR}")
    endif()

    if (LIBRARY_OUTPUT_PATH)
        set(CMAKE_JAVA_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
    else ()
        set(CMAKE_JAVA_LIBRARY_OUTPUT_PATH ${_add_jar_OUTPUT_DIR})
    endif ()

    set(CMAKE_JAVA_INCLUDE_PATH
        ${CMAKE_JAVA_INCLUDE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_JAVA_OBJECT_OUTPUT_PATH}
        ${CMAKE_JAVA_LIBRARY_OUTPUT_PATH}
    )

    foreach (JAVA_INCLUDE_DIR IN LISTS CMAKE_JAVA_INCLUDE_PATH)
       string(APPEND CMAKE_JAVA_INCLUDE_PATH_FINAL "${_UseJava_PATH_SEP}${JAVA_INCLUDE_DIR}")
    endforeach()
    
    foreach (JAVA_INCLUDE_TARGET IN LISTS CMAKE_JAVA_INCLUDE_TARGET)
        get_property(TARGET_JAR_DEBUG TARGET ${JAVA_INCLUDE_TARGET} PROPERTY JAR_FILE_DEBUG)
        get_property(TARGET_JAR_RELEASE TARGET ${JAVA_INCLUDE_TARGET} PROPERTY JAR_FILE_RELEASE)
        string(APPEND CMAKE_JAVA_INCLUDE_PATH_FINAL_DEBUG "${_UseJava_PATH_SEP}${TARGET_JAR_DEBUG}")
        string(APPEND CMAKE_JAVA_INCLUDE_PATH_FINAL_RELEASE "${_UseJava_PATH_SEP}${TARGET_JAR_RELEASE}")
    endforeach()

    set(CMAKE_JAVA_CLASS_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${_TARGET_NAME}.dir")

    set(_JAVA_TARGET_OUTPUT_NAME "${_TARGET_NAME}.jar")
    if (_add_jar_OUTPUT_NAME AND _add_jar_VERSION)
        set(_JAVA_TARGET_OUTPUT_NAME "${_add_jar_OUTPUT_NAME}-${_add_jar_VERSION}.jar")
        set(_JAVA_TARGET_OUTPUT_LINK "${_add_jar_OUTPUT_NAME}.jar")
        set(_JAVA_TAREGET_OUTPUT_LINK_DEBUG "${_add_jar_OUTPUT_NAME}${CMAKE_DEBUG_POSTFIX}.jar")
    elseif (_add_jar_VERSION)
        set(_JAVA_TARGET_OUTPUT_NAME "${_TARGET_NAME}-${_add_jar_VERSION}.jar")
        set(_JAVA_TARGET_OUTPUT_LINK "${_TARGET_NAME}.jar")
        set(_JAVA_TARGET_OUTPUT_LINK_DEBUG "${_TARGET_NAME}${CMAKE_DEBUG_POSTFIX}.jar")
    elseif (_add_jar_OUTPUT_NAME)
        set(_JAVA_TARGET_OUTPUT_NAME "${_add_jar_OUTPUT_NAME}.jar")
    endif ()

    set(_JAVA_CLASS_FILES)
    set(_JAVA_COMPILE_FILES)
    set(_JAVA_COMPILE_FILES_DEBUG)
    set(_JAVA_COMPILE_FILES_RELEASE)
    set(_JAVA_COMPILE_FILELISTS)
    set(_JAVA_DEPENDS)
    set(_JAVA_COMPILE_DEPENDS)
    set(_JAVA_RESOURCE_FILES)
    set(_JAVA_RESOURCE_FILES_RELATIVE)
    set(_JAVA_CLASS_FILES_DEBUG)
    set(_JAVA_CLASS_FILES_RELEASE)
    foreach(_JAVA_SOURCE_FILE IN LISTS _JAVA_SOURCE_FILES)
        get_filename_component(_JAVA_EXT ${_JAVA_SOURCE_FILE} EXT)
        get_filename_component(_JAVA_FILE ${_JAVA_SOURCE_FILE} NAME_WE)
        get_filename_component(_JAVA_PATH ${_JAVA_SOURCE_FILE} PATH)
        get_filename_component(_JAVA_FULL ${_JAVA_SOURCE_FILE} ABSOLUTE)

        if (_JAVA_SOURCE_FILE MATCHES "^@(.+)$")
            get_filename_component(_JAVA_FULL ${CMAKE_MATCH_1} ABSOLUTE)
            list(APPEND _JAVA_COMPILE_FILELISTS ${_JAVA_FULL})

        elseif (_JAVA_EXT MATCHES ".java")
            file(RELATIVE_PATH _JAVA_REL_BINARY_PATH ${CMAKE_CURRENT_BINARY_DIR} ${_JAVA_FULL})
            file(RELATIVE_PATH _JAVA_REL_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${_JAVA_FULL})
            string(LENGTH ${_JAVA_REL_BINARY_PATH} _BIN_LEN)
            string(LENGTH ${_JAVA_REL_SOURCE_PATH} _SRC_LEN)
            if (_BIN_LEN LESS _SRC_LEN)
                set(_JAVA_REL_PATH ${_JAVA_REL_BINARY_PATH})
            else ()
                set(_JAVA_REL_PATH ${_JAVA_REL_SOURCE_PATH})
            endif ()
            get_filename_component(_JAVA_REL_PATH ${_JAVA_REL_PATH} PATH)

            list(APPEND _JAVA_COMPILE_FILES ${_JAVA_SOURCE_FILE})
            set(_JAVA_CLASS_FILE "${CMAKE_JAVA_CLASS_OUTPUT_PATH}/${_JAVA_REL_PATH}/${_JAVA_FILE}.class")
            set(_JAVA_CLASS_FILES ${_JAVA_CLASS_FILES} ${_JAVA_CLASS_FILE})

        elseif (_JAVA_EXT MATCHES ".jar"
                OR _JAVA_EXT MATCHES ".war"
                OR _JAVA_EXT MATCHES ".ear"
                OR _JAVA_EXT MATCHES ".sar")
            # Ignored for backward compatibility

        elseif (_JAVA_EXT STREQUAL "")
            list(APPEND CMAKE_JAVA_INCLUDE_PATH ${JAVA_JAR_TARGET_${_JAVA_SOURCE_FILE}} ${JAVA_JAR_TARGET_${_JAVA_SOURCE_FILE}_CLASSPATH})
            list(APPEND _JAVA_DEPENDS ${JAVA_JAR_TARGET_${_JAVA_SOURCE_FILE}})

        else ()
            __java_copy_file(${CMAKE_CURRENT_SOURCE_DIR}/${_JAVA_SOURCE_FILE}
                             ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/${_JAVA_SOURCE_FILE}
                             "Copying ${_JAVA_SOURCE_FILE} to the build directory")
            list(APPEND _JAVA_RESOURCE_FILES ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/${_JAVA_SOURCE_FILE})
            list(APPEND _JAVA_RESOURCE_FILES_RELATIVE ${_JAVA_SOURCE_FILE})
        endif ()
    endforeach()
    
    foreach(_JAVA_SOURCE_FILE IN LISTS _JAVA_SOURCE_FILES_DEBUG)
        get_filename_component(_JAVA_EXT ${_JAVA_SOURCE_FILE} EXT)
        get_filename_component(_JAVA_FULL ${_JAVA_SOURCE_FILE} ABSOLUTE)
        get_filename_component(_JAVA_FILE ${_JAVA_SOURCE_FILE} NAME_WE)
        
        if (_JAVA_EXT MATCHES ".java")
            file(RELATIVE_PATH _JAVA_REL_BINARY_PATH ${CMAKE_CURRENT_BINARY_DIR} ${_JAVA_FULL})
            file(RELATIVE_PATH _JAVA_REL_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${_JAVA_FULL})
            string(LENGTH ${_JAVA_REL_BINARY_PATH} _BIN_LEN)
            string(LENGTH ${_JAVA_REL_SOURCE_PATH} _SRC_LEN)
            if (_BIN_LEN LESS _SRC_LEN)
                set(_JAVA_REL_PATH ${_JAVA_REL_BINARY_PATH})
            else ()
                set(_JAVA_REL_PATH ${_JAVA_REL_SOURCE_PATH})
            endif ()
            get_filename_component(_JAVA_REL_PATH ${_JAVA_REL_PATH} PATH)

            list(APPEND _JAVA_COMPILE_FILES_DEBUG ${_JAVA_SOURCE_FILE})
            set(_JAVA_CLASS_FILE "${CMAKE_JAVA_CLASS_OUTPUT_PATH}/${_JAVA_REL_PATH}/${_JAVA_FILE}.class")
            set(_JAVA_CLASS_FILES_DEBUG ${_JAVA_CLASS_FILES_DEBUG} ${_JAVA_CLASS_FILE})
        endif()
    endforeach()
    
    foreach(_JAVA_SOURCE_FILE IN LISTS _JAVA_SOURCE_FILES_RELEASE)
        get_filename_component(_JAVA_EXT ${_JAVA_SOURCE_FILE} EXT)
        get_filename_component(_JAVA_FULL ${_JAVA_SOURCE_FILE} ABSOLUTE)
        get_filename_component(_JAVA_FILE ${_JAVA_SOURCE_FILE} NAME_WE)
        
        if (_JAVA_EXT MATCHES ".java")
            file(RELATIVE_PATH _JAVA_REL_BINARY_PATH ${CMAKE_CURRENT_BINARY_DIR} ${_JAVA_FULL})
            file(RELATIVE_PATH _JAVA_REL_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${_JAVA_FULL})
            string(LENGTH ${_JAVA_REL_BINARY_PATH} _BIN_LEN)
            string(LENGTH ${_JAVA_REL_SOURCE_PATH} _SRC_LEN)
            if (_BIN_LEN LESS _SRC_LEN)
                set(_JAVA_REL_PATH ${_JAVA_REL_BINARY_PATH})
            else ()
                set(_JAVA_REL_PATH ${_JAVA_REL_SOURCE_PATH})
            endif ()
            get_filename_component(_JAVA_REL_PATH ${_JAVA_REL_PATH} PATH)

            list(APPEND _JAVA_COMPILE_FILES_RELEASE ${_JAVA_SOURCE_FILE})
            set(_JAVA_CLASS_FILE "${CMAKE_JAVA_CLASS_OUTPUT_PATH}/${_JAVA_REL_PATH}/${_JAVA_FILE}.class")
            set(_JAVA_CLASS_FILES_RELEASE ${_JAVA_CLASS_FILES_RELEASE} ${_JAVA_CLASS_FILE})
        endif()
    endforeach()
    
    

    foreach(_JAVA_INCLUDE_JAR IN LISTS _add_jar_INCLUDE_JARS)
        if (TARGET ${_JAVA_INCLUDE_JAR})
            get_target_property(_JAVA_JAR_PATH ${_JAVA_INCLUDE_JAR} JAR_FILE)
            if (_JAVA_JAR_PATH)
                string(APPEND CMAKE_JAVA_INCLUDE_PATH_FINAL "${_UseJava_PATH_SEP}${_JAVA_JAR_PATH}")
                list(APPEND CMAKE_JAVA_INCLUDE_PATH ${_JAVA_JAR_PATH})
                list(APPEND _JAVA_DEPENDS ${_JAVA_INCLUDE_JAR})
                list(APPEND _JAVA_COMPILE_DEPENDS ${_JAVA_JAR_PATH})
            else ()
                message(SEND_ERROR "add_jar: INCLUDE_JARS target ${_JAVA_INCLUDE_JAR} is not a jar")
            endif ()
        else ()
            string(APPEND CMAKE_JAVA_INCLUDE_PATH_FINAL "${_UseJava_PATH_SEP}${_JAVA_INCLUDE_JAR}")
            list(APPEND CMAKE_JAVA_INCLUDE_PATH "${_JAVA_INCLUDE_JAR}")
            list(APPEND _JAVA_DEPENDS "${_JAVA_INCLUDE_JAR}")
            list(APPEND _JAVA_COMPILE_DEPENDS "${_JAVA_INCLUDE_JAR}")
        endif ()
    endforeach()

    if (_JAVA_COMPILE_FILES OR _JAVA_COMPILE_FILELISTS)
        set (_JAVA_SOURCES_FILELISTS)
        set (_JAVA_SOURCES_FILELISTS_DEBUG)
        set (_JAVA_SOURCES_FILELISTS_RELEASE)
        
        if (_JAVA_COMPILE_FILES)
            # Create the list of files to compile.
            set(_JAVA_SOURCES_FILE ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_sources)
            string(REPLACE ";" "\"\n\"" _JAVA_COMPILE_STRING "\"${_JAVA_COMPILE_FILES}\"")
            file(WRITE ${_JAVA_SOURCES_FILE} ${_JAVA_COMPILE_STRING})
            list (APPEND _JAVA_SOURCES_FILELISTS "@${_JAVA_SOURCES_FILE}")
            
            if (_JAVA_COMPILE_FILES_DEBUG)
                set(_JAVA_COMPILE_STRING)
                set(_JAVA_SOURCES_FILE ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_sources_debug)
                string(REPLACE ";" "\"\n\"" _JAVA_COMPILE_STRING "\"${_JAVA_COMPILE_FILES_DEBUG}\"")
                file(WRITE ${_JAVA_SOURCES_FILE} ${_JAVA_COMPILE_STRING})
                list (APPEND _JAVA_SOURCES_FILELISTS_DEBUG "@${_JAVA_SOURCES_FILE}")
            endif()
            
            if (_JAVA_COMPILE_FILES_RELEASE)
                set(_JAVA_COMPILE_STRING)
                set(_JAVA_SOURCES_FILE ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_sources_release)
                string(REPLACE ";" "\"\n\"" _JAVA_COMPILE_STRING "\"${_JAVA_COMPILE_FILES_RELEASE}\"")
                file(WRITE ${_JAVA_SOURCES_FILE} ${_JAVA_COMPILE_STRING})
                list (APPEND _JAVA_SOURCES_FILELISTS_RELEASE "@${_JAVA_SOURCES_FILE}")
            endif()
        endif()
        if (_JAVA_COMPILE_FILELISTS)
            foreach (_JAVA_FILELIST IN LISTS _JAVA_COMPILE_FILELISTS)
                list (APPEND _JAVA_SOURCES_FILELISTS "@${_JAVA_FILELIST}")
            endforeach()
        endif()

        # Compile the java files and create a list of class files
        add_custom_command(
            # NOTE: this command generates an artificial dependency file
            OUTPUT ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_compiled_${_TARGET_NAME}
            COMMAND ${CMAKE_COMMAND}
            ARGS
                -DCMAKE_JAVA_CLASS_OUTPUT_PATH=${CMAKE_JAVA_CLASS_OUTPUT_PATH}
                -DCMAKE_JAR_CLASSES_PREFIX="${CMAKE_JAR_CLASSES_PREFIX}"
                -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CleanJavaClassFilelist.cmake 
            COMMAND ${Java_JAVAC_EXECUTABLE}
            ARGS
                ${CMAKE_JAVA_COMPILE_FLAGS}
                -classpath "${CMAKE_JAVA_INCLUDE_PATH_FINAL}$<$<NOT:$<CONFIG:Debug>>:${CMAKE_JAVA_INCLUDE_PATH_FINAL_RELEASE}>$<$<CONFIG:Debug>:${CMAKE_JAVA_INCLUDE_PATH_FINAL_DEBUG}>"
                -d ${CMAKE_JAVA_CLASS_OUTPUT_PATH}
                ${_GENERATE_NATIVE_HEADERS}
                ${_JAVA_SOURCES_FILELISTS} $<$<NOT:$<CONFIG:Debug>>:${_JAVA_SOURCES_FILELISTS_RELEASE}>$<$<CONFIG:Debug>:${_JAVA_SOURCES_FILELISTS_DEBUG}>
            COMMAND ${CMAKE_COMMAND}
            ARGS
                -E touch ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_compiled_${_TARGET_NAME}
            DEPENDS ${_JAVA_COMPILE_FILES} ${_JAVA_COMPILE_FILELISTS} ${_JAVA_COMPILE_DEPENDS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building Java objects for ${_TARGET_NAME}.jar"
        )
        add_custom_command(
            OUTPUT ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist
            COMMAND ${CMAKE_COMMAND}
            ARGS
                -DCMAKE_JAVA_CLASS_OUTPUT_PATH=${CMAKE_JAVA_CLASS_OUTPUT_PATH}
                -DCMAKE_JAR_CLASSES_PREFIX="${CMAKE_JAR_CLASSES_PREFIX}"
                -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/UseJavaClassFilelist.cmake
            DEPENDS ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_compiled_${_TARGET_NAME}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    else ()
        # create an empty java_class_filelist
        if (NOT EXISTS ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist)
            file(WRITE ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist "")
        endif()
    endif ()

    # create the jar file
    set(_JAVA_JAR_OUTPUT_PATH
      "${_add_jar_OUTPUT_DIR}/${_JAVA_TARGET_OUTPUT_NAME}")
    get_filename_component(_JAVA_TARGET_OUTPUT_NAME_DEBUG "${_JAVA_TARGET_OUTPUT_NAME}" NAME_WE)
    set(_JAVA_TARGET_OUTPUT_NAME_DEBUG "${_JAVA_TARGET_OUTPUT_NAME_DEBUG}${CMAKE_DEBUG_POSTFIX}.jar")
    set(_JAVA_JAR_OUTPUT_PATH_DEBUG "${_add_jar_OUTPUT_DIR}/${_JAVA_TARGET_OUTPUT_NAME_DEBUG}")
    if (CMAKE_JNI_TARGET)
        add_custom_command(
            OUTPUT ${_JAVA_JAR_OUTPUT_PATH}
            COMMAND ${Java_JAR_EXECUTABLE}
            ARGS
                -cf${_ENTRY_POINT_OPTION}${_MANIFEST_OPTION} $<$<NOT:$<CONFIG:Debug>>:${_JAVA_JAR_OUTPUT_PATH}>$<$<CONFIG:Debug>:${_JAVA_JAR_OUTPUT_PATH_DEBUG}> ${_ENTRY_POINT_VALUE} ${_MANIFEST_VALUE}
                ${_JAVA_RESOURCE_FILES_RELATIVE} @java_class_filelist
            COMMAND ${CMAKE_COMMAND}
            ARGS
                -D_JAVA_TARGET_DIR=${_add_jar_OUTPUT_DIR}
                -D_JAVA_TARGET_OUTPUT_NAME=$<$<NOT:$<CONFIG:Debug>>:${_JAVA_TARGET_OUTPUT_NAME}>$<$<CONFIG:Debug>:${_JAVA_TARGET_OUTPUT_NAME_DEBUG}>
                -D_JAVA_TARGET_OUTPUT_LINK=$<$<NOT:$<CONFIG:Debug>>:${_JAVA_TARGET_OUTPUT_LINK}>$<$<CONFIG:Debug>:${_JAVA_TARGET_OUTPUT_LINK_DEBUG}>
                -P ${_JAVA_SYMLINK_SCRIPT}
            COMMAND ${CMAKE_COMMAND}
            ARGS
                -D_JAVA_TARGET_DIR=${_add_jar_OUTPUT_DIR}
                -D_JAVA_TARGET_OUTPUT_NAME=$<$<NOT:$<CONFIG:Debug>>:${_JAVA_JAR_OUTPUT_PATH}>$<$<CONFIG:Debug>:${_JAVA_JAR_OUTPUT_PATH_DEBUG}>
                -D_JAVA_TARGET_OUTPUT_LINK=$<$<NOT:$<CONFIG:Debug>>:${_JAVA_TARGET_OUTPUT_LINK}>$<$<CONFIG:Debug>:${_JAVA_TARGET_OUTPUT_LINK_DEBUG}>
                -P ${_JAVA_SYMLINK_SCRIPT}
            DEPENDS ${_JAVA_RESOURCE_FILES} ${_JAVA_DEPENDS} ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist
            WORKING_DIRECTORY ${CMAKE_JAVA_CLASS_OUTPUT_PATH}
            COMMENT "Creating Java archive ${_JAVA_TARGET_OUTPUT_NAME}"
        )
    else ()
        add_custom_command(
            OUTPUT ${_JAVA_JAR_OUTPUT_PATH}
            COMMAND ${Java_JAR_EXECUTABLE}
            ARGS
                -cf${_ENTRY_POINT_OPTION}${_MANIFEST_OPTION} $<$<NOT:$<CONFIG:Debug>>:${_JAVA_JAR_OUTPUT_PATH}>$<$<CONFIG:Debug>:${_JAVA_JAR_OUTPUT_PATH_DEBUG}> ${_ENTRY_POINT_VALUE} ${_MANIFEST_VALUE}
                ${_JAVA_RESOURCE_FILES_RELATIVE} @java_class_filelist
            COMMAND ${CMAKE_COMMAND}
            ARGS
                -D_JAVA_TARGET_DIR=${_add_jar_OUTPUT_DIR}
                -D_JAVA_TARGET_OUTPUT_NAME=$<$<NOT:$<CONFIG:Debug>>:${_JAVA_TARGET_OUTPUT_NAME}>$<$<CONFIG:Debug>:${_JAVA_TARGET_OUTPUT_NAME_DEBUG}>
                -D_JAVA_TARGET_OUTPUT_LINK=$<$<NOT:$<CONFIG:Debug>>:${_JAVA_TARGET_OUTPUT_LINK}>$<$<CONFIG:Debug>:${_JAVA_TARGET_OUTPUT_LINK_DEBUG}>
                -P ${_JAVA_SYMLINK_SCRIPT}
            WORKING_DIRECTORY ${CMAKE_JAVA_CLASS_OUTPUT_PATH}
            DEPENDS ${_JAVA_RESOURCE_FILES} ${_JAVA_DEPENDS} ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist
            COMMENT "Creating Java archive ${_JAVA_TARGET_OUTPUT_NAME}"
        )
    endif ()

    # Add the target and make sure we have the latest resource files.
    add_custom_target(${_TARGET_NAME} ALL DEPENDS ${_JAVA_JAR_OUTPUT_PATH} SOURCES ${_JAVA_SOURCE_FILES} $<$<NOT:$<CONFIG:Debug>>:${_JAVA_SOURCE_FILES_RELEASE}>$<$<CONFIG:Debug>:${_JAVA_SOURCE_FILES_DEBUG}>)

    set_property(
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            INSTALL_FILES_DEBUG
                ${_JAVA_JAR_OUTPUT_PATH_DEBUG}
    )
    
    set_property(
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            INSTALL_FILES_RELEASE
                ${_JAVA_JAR_OUTPUT_PATH}
    )

    if (_JAVA_TARGET_OUTPUT_LINK)
        set_property(
            TARGET
                ${_TARGET_NAME}
            PROPERTY
                INSTALL_FILES_DEBUG
                    ${_JAVA_JAR_OUTPUT_PATH_DEBUG}
                    ${_add_jar_OUTPUT_DIR}/${_JAVA_TARGET_OUTPUT_LINK_DEBUG}
        )
        
        set_property(
            TARGET
                ${_TARGET_NAME}
            PROPERTY
                INSTALL_FILES_RELEASE
                    ${_JAVA_JAR_OUTPUT_PATH}
                    ${_add_jar_OUTPUT_DIR}/${_JAVA_TARGET_OUTPUT_LINK}
        )

        if (CMAKE_JNI_TARGET)
            set_property(
                TARGET
                    ${_TARGET_NAME}
                PROPERTY
                    JNI_SYMLINK_DEBUG
                        ${_add_jar_OUTPUT_DIR}/${_JAVA_TARGET_OUTPUT_LINK_DEBUG}
            )
            set_property(
                TARGET
                    ${_TARGET_NAME}
                PROPERTY
                    JNI_SYMLINK_RELEASE
                        ${_add_jar_OUTPUT_DIR}/${_JAVA_TARGET_OUTPUT_LINK}
            )
        endif ()
    endif ()

    set_property(
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            JAR_FILE_RELEASE
                ${_JAVA_JAR_OUTPUT_PATH}
    )
    
    set_property(
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            JAR_FILE_DEBUG
                ${_JAVA_JAR_OUTPUT_PATH_DEBUG}
    )

    set_property(
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            CLASSDIR
                ${CMAKE_JAVA_CLASS_OUTPUT_PATH}
    )

  if (_GENERATE_NATIVE_HEADERS)
    # create an INTERFACE library encapsulating include directory for generated headers
    add_library (${_GENERATE_NATIVE_HEADERS_TARGET} INTERFACE)
    target_include_directories (${_GENERATE_NATIVE_HEADERS_TARGET} INTERFACE
      "${_GENERATE_NATIVE_HEADERS_OUTPUT_DIR}"
      ${JNI_INCLUDE_DIRS})
    # this INTERFACE library depends on jar generation
    add_dependencies (${_GENERATE_NATIVE_HEADERS_TARGET} ${_TARGET_NAME})

    set_property (DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
      "${_GENERATE_NATIVE_HEADERS_OUTPUT_DIR}")
  endif()
endfunction()
