## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS = libcharset .

INCLUDES = -I $(srcdir)/libcharset

noinst_LTLIBRARIES = libutf8.la
libutf8_la_SOURCES = utf8-locale.c
libutf8_la_LDFLAGS = $(LIBICONV)
libutf8_la_LIBADD  = libcharset/libcharset.la
noinst_HEADERS = utf8-locale.h
