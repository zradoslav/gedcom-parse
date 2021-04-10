add_library(utf8tools SHARED)

target_include_directories(utf8tools PRIVATE libcharset)
target_sources(utf8tools
	PUBLIC utf8tools.h
	PRIVATE utf8.c utf8-convert.c utf8-locale.c libcharset/localcharset.c)

set_target_properties(utf8tools
	PROPERTIES VERSION 0.2.0)
