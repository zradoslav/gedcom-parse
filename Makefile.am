## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
SUBDIRS = intl ansel gedcom . t doc include po
INCLUDES = -I $(srcdir)/include

pkgdata_DATA = gedcom.enc
noinst_PROGRAMS = testgedcom

testgedcom_SOURCES = standalone.c
testgedcom_LDFLAGS = -L gedcom/.libs -lgedcom
testgedcom_LDADD  = @INTLLIBS@

EXTRA_DIST = $(pkgdata_DATA)

clean-local:
	rm -f testgedcom.out
