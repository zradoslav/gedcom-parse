cmake_minimum_required(VERSION 3.13)

project(gedcom-parse VERSION 0.9.0
	LANGUAGES C)

set(CMAKE_C_EXTENSIONS ON)

option(ENABLE_NLS "enable localization of libgedcom, libgedcom-gom and helpers" ON)

include(GNUInstallDirs)

configure_file(config.h.in config.h)
include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(bin)
add_subdirectory(gedcom)
add_subdirectory(gom)
add_subdirectory(utf8)
