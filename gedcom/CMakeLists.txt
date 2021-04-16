find_package(BISON)
find_package(FLEX)

BISON_TARGET(gedcom_parser gedcom.y ${CMAKE_CURRENT_BINARY_DIR}/gedcom.tabgen.c
	COMPILE_FLAGS "--name-prefix=gedcom_ --defines"
)
FLEX_TARGET(gedcom_1byte_lex gedcom_1byte.lex  ${CMAKE_CURRENT_BINARY_DIR}/gedcom_1byte.c
	COMPILE_FLAGS "-Pgedcom_1byte_"
)
FLEX_TARGET(gedcom_hilo_lex gedcom_hilo.lex  ${CMAKE_CURRENT_BINARY_DIR}/gedcom_hilo.c
	COMPILE_FLAGS "-Pgedcom_hilo_"
)
FLEX_TARGET(gedcom_lohi_lex gedcom_lohi.lex  ${CMAKE_CURRENT_BINARY_DIR}/gedcom_lohi.c
	COMPILE_FLAGS "-Pgedcom_lohi_"
)
ADD_FLEX_BISON_DEPENDENCY(gedcom_1byte_lex gedcom_parser)
ADD_FLEX_BISON_DEPENDENCY(gedcom_hilo_lex gedcom_parser)
ADD_FLEX_BISON_DEPENDENCY(gedcom_lohi_lex gedcom_parser)

BISON_TARGET(gedcom_date_parser gedcom_date.y ${CMAKE_CURRENT_BINARY_DIR}/gedcom_date.tabgen.c
	COMPILE_FLAGS "--name-prefix=gedcom_date_ --defines"
)
FLEX_TARGET(gedcom_date_lex gedcom_date.lex  ${CMAKE_CURRENT_BINARY_DIR}/gedcom_date.c
	COMPILE_FLAGS "-Pgedcom_date_"
)
ADD_FLEX_BISON_DEPENDENCY(gedcom_date_lex gedcom_date_parser)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/gedcom)
include_directories(${CMAKE_SOURCE_DIR}/gedcom/calendar)
include_directories(${CMAKE_SOURCE_DIR}/utf8)

include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/gedcom)

add_custom_command(OUTPUT tag_names.h
	MAIN_DEPENDENCY ${BISON_gedcom_parser_OUTPUT_HEADER}
	COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/process_tags ${BISON_gedcom_parser_OUTPUT_HEADER} tag_names.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(OUTPUT include/gedcom-tags.h
	MAIN_DEPENDENCY ${BISON_gedcom_parser_OUTPUT_HEADER}
	COMMAND mkdir -p include
	COMMAND grep -P "\"(USER)?TAG\"" ${BISON_gedcom_parser_OUTPUT_HEADER}
	| sed -re "\"s/^\\s+(\\S+)\\s=\\s([0-9]+).+/#define \\1 \\2/g\"" > include/gedcom-tags.h
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

file(GLOB_RECURSE SOURCES *.c calendar/*c)
list(APPEND SOURCES
	${BISON_gedcom_parser_OUTPUTS}
	${FLEX_gedcom_1byte_lex_OUTPUTS}
	${FLEX_gedcom_lohi_lex_OUTPUTS}
	${FLEX_gedcom_hilo_lex_OUTPUTS}
	${BISON_gedcom_date_parser_OUTPUTS}
	${FLEX_gedcom_date_lex_OUTPUTS}
	tag_names.h
	include/gedcom-tags.h
)


add_library(gedcom SHARED)
target_sources(gedcom PRIVATE ${SOURCES})

set_target_properties(gedcom
	PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
