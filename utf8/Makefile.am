## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS = @LCS_SUBDIRS@ .

INCLUDES = @LCS_INCLUDES@
CFLAGS   = -O2 -W -Wall -pedantic -Wno-long-long @EXTRA_CFLAGS@

lib_LTLIBRARIES = libutf8tools.la
libutf8tools_la_LDFLAGS = -export-dynamic -version-info $(LIBVERSION)
libutf8tools_la_SOURCES = utf8-locale.c utf8.c utf8-convert.c
libutf8tools_la_LIBADD  = @LCS_LIBADD@ $(LIBICONV)
include_HEADERS = utf8tools.h
