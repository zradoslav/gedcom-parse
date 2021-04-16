add_library(utf8tools SHARED)

target_sources(utf8tools
	PUBLIC utf8tools.h
	PRIVATE utf8.c utf8-convert.c utf8-locale.c)
target_link_libraries(utf8tools PRIVATE unistring)

set_target_properties(utf8tools
	PROPERTIES VERSION 0.2.0)
