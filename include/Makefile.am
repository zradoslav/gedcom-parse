## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
include_HEADERS = gedcom.h \
		  gedcom-tags.h \
		  gom.h
BUILT_SOURCES = gedcom.h \
		gedcom-tags.h
EXTRA_DIST = gedcom.h.in \
	     gedcom-tags.h

gedcom-tags.h:	$(srcdir)/../gedcom/gedcom.tabgen.h
	grep "TAG_" $< > $@
	grep "USERTAG" $< >> $@

