cmake_minimum_required(VERSION 3.11)

project(libkeyvalue_parser)

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

add_definitions(-DLIBKEYVALUE_PARSER_EXPORTS)
set(SRCS keyvalue_parser.h keyvalue_parser.c params.h lex.yy.c y.tab.c)
add_library(${PROJECT_NAME} SHARED ${SRCS})
gravity_add_dependency(${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${Lex_CMD} -o "${CMAKE_CURRENT_LIST_DIR}/lex.yy.c" "${CMAKE_CURRENT_LIST_DIR}/keyvalue.l")
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${Yacc_CMD} -dt "${CMAKE_CURRENT_LIST_DIR}/keyvalue.y" -o "${CMAKE_CURRENT_LIST_DIR}/y.tab.c")

target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib)
install(FILES keyvalue_parser.h DESTINATION include)
install(EXPORT ${PROJECT_NAME} DESTINATION cmake)