## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

PATCH_FILES := $(wildcard $(srcdir)/*.patch)
SRC_FILES   := ansi_z39_47.h

EXTRA_DIST = ChangeLog.gedcom NEWS.gedcom get_patches patch_dist \
	     $(SRC_FILES) $(PATCH_FILES)

dist:
	./patch_dist
