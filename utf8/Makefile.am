## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS = @LCS_SUBDIRS@ .

INCLUDES = @LCS_INCLUDES@
CFLAGS   = -g -O2 -W -Wall -pedantic -Wno-long-long

noinst_LTLIBRARIES = libutf8.la
libutf8_la_SOURCES = utf8-locale.c
libutf8_la_LIBADD  = @LCS_LIBADD@ $(LIBICONV)
noinst_HEADERS = utf8-locale.h
