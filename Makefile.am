## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
SUBDIRS = intl ansel gedcom include . t doc po
INCLUDES = -I $(srcdir)/include
CFLAGS   = -g -O2

pkgdata_DATA = gedcom.enc
noinst_PROGRAMS = testgedcom

testgedcom_SOURCES = standalone.c utf8-locale.c
noinst_HEADERS     = utf8-locale.h
testgedcom_LDFLAGS = -L gedcom/.libs -lgedcom
testgedcom_LDADD  = @INTLLIBS@

EXTRA_DIST = $(pkgdata_DATA)
VERSIONED_FILES = README

include $(top_srcdir)/Makefile.inc

dist-hook:
	$(replace-VERSION)

clean-local:
	rm -f testgedcom.out
