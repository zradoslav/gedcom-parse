## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
SUBDIRS = intl ansel utf8 gedcom gom include . t doc data po

EXTRA_DIST = Makefile.inc
VERSIONED_FILES = README

include $(top_srcdir)/Makefile.inc

dist-hook:
	$(replace-VERSION)
