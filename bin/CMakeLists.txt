add_executable(gedcom-check gedcom-check.c)
add_dependencies(gedcom-check utf8tools gedcom)
target_include_directories(gedcom-check PRIVATE
	${CMAKE_BINARY_DIR}/include
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/utf8
)
target_link_libraries(gedcom-check PRIVATE utf8tools gedcom)

add_executable(gedcom-sanitize gedcom-sanitize.c)
add_dependencies(gedcom-sanitize utf8tools gedcom)
target_include_directories(gedcom-sanitize PRIVATE
	${CMAKE_BINARY_DIR}/include
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/utf8
)
target_link_libraries(gedcom-sanitize PRIVATE utf8tools gedcom gedcom_gom)
