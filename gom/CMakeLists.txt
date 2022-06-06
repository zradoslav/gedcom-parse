file(GLOB_RECURSE SOURCES *.c)

add_library(gedcom_gom SHARED)

target_include_directories(gedcom_gom PRIVATE
	${CMAKE_BINARY_DIR}/include
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/utf8
)
target_sources(gedcom_gom PRIVATE ${SOURCES})
target_link_libraries(gedcom_gom PRIVATE utf8tools)

set_target_properties(gedcom_gom
	PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
