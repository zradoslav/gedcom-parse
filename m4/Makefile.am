## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
M4_FILES = gedcom.m4
EXTRA_DIST = $(M4_FILES)

install-m4:
	@dir=`aclocal --print-ac-dir`; \
	echo "Installing m4 macros in $$dir"; \
	for file in $(M4_FILES); do \
	  $(INSTALL) $(srcdir)/$$file $$dir; \
	done

