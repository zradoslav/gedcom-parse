## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
SUBDIRS = intl ansel gedcom include . t doc po

pkgdata_DATA = gedcom.enc

EXTRA_DIST = $(pkgdata_DATA) Makefile.inc
VERSIONED_FILES = README

include $(top_srcdir)/Makefile.inc

dist-hook:
	$(replace-VERSION)

clean-local:
	rm -f testgedcom.out
