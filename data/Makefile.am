## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
pkgdata_DATA = gedcom.enc new.ged

EXTRA_DIST = $(pkgdata_DATA)
VERSIONED_FILES = new.ged

include $(top_srcdir)/Makefile.inc

dist-hook:
	$(replace-VERSION)
