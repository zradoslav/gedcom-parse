## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
SUBDIRS = ansel gedcom . t doc include po intl
INCLUDES = -I $(srcdir)/include

pkgdata_DATA = gedcom.enc
noinst_PROGRAMS = testgedcom

testgedcom_SOURCES = standalone.c
testgedcom_LDFLAGS = -L gedcom/.libs -lgedcom

EXTRA_DIST = $(pkgdata_DATA)
