cmake_minimum_required(VERSION 3.17)

project(GravityKeyValueParser)

set(LIB_NAME keyvalue_parser)

include(CMakePackageConfigHelpers)
include(FetchContent)
include(GravityExternalUrls)
include(GravitySupport)

if (WIN32)
    if (NOT Lex_Yacc_DIR)
        FetchContent_Populate(LexYacc URL ${lexyacc_win_url} SOURCE_DIR win_flex_bison)
        set(Lex_Yacc_DIR "${CMAKE_CURRENT_BINARY_DIR}/win_flex_bison")
    endif()
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    if (NOT Lex_Yacc_DIR)
        set(Lex_Yacc_DIR "/usr/bin" "/usr/local/bin")
    endif()
endif()

find_program(Lex_CMD NAMES win_flex.exe flex PATHS "${Lex_Yacc_DIR}")
if (NOT Lex_CMD)
    message(FATAL_ERROR "Could not locate lex cmd")
endif()
find_program(Yacc_CMD NAMES win_bison.exe bison PATHS "${Lex_Yacc_DIR}")
if (NOT Yacc_CMD)
    message(FATAL_ERROR "Could not locate yacc cmd")
endif()

set(SRCS keyvalue_parser.h keyvalue_parser.c params.h "${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" "${CMAKE_CURRENT_BINARY_DIR}/y.tab.c")
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" "${CMAKE_CURRENT_BINARY_DIR}/y.tab.c" PROPERTIES GENERATED ON)
add_library(${LIB_NAME} STATIC ${SRCS})
target_compile_definitions(${LIB_NAME} PUBLIC "-DLIBKEYVALUE_STATIC")
set_target_properties(${LIB_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

gravity_add_dependency(${LIB_NAME})

if (WIN32)
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" COMMAND ${Lex_CMD} -o "${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" "${CMAKE_CURRENT_LIST_DIR}/keyvalue.l")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/y.tab.c" COMMAND ${Yacc_CMD} -dt "${CMAKE_CURRENT_LIST_DIR}/keyvalue.y" -o "${CMAKE_CURRENT_BINARY_DIR}/y.tab.c")
else()
    add_custom_target(RunLexYacc ALL ${Lex_CMD} -o "${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" "${CMAKE_CURRENT_LIST_DIR}/keyvalue.l" COMMAND ${Yacc_CMD} -dt "${CMAKE_CURRENT_LIST_DIR}/keyvalue.y" -o "${CMAKE_CURRENT_BINARY_DIR}/y.tab.c" DEPENDS "${CMAKE_CURRENT_LIST_DIR}/keyvalue.l" "${CMAKE_CURRENT_LIST_DIR}/keyvalue.y" BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/y.tab.c" "${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c")
    add_dependencies(${LIB_NAME} RunLexYacc)
endif()


target_include_directories(${LIB_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                                         "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

install(TARGETS ${LIB_NAME} EXPORT ${PROJECT_NAME}-targets RUNTIME DESTINATION bin ARCHIVE DESTINATION lib)
#install(FILES keyvalue_parser.h DESTINATION include)
configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/../../cmake/${PROJECT_NAME}Config.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
   INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gravity)
install(EXPORT ${PROJECT_NAME}-targets
   FILE ${PROJECT_NAME}Targets.cmake
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gravity)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gravity)
