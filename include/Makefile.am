## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
include_HEADERS = gedcom.h \
		  gedcom-tags.h
BUILT_SOURCES = gedcom.h \
		gedcom-tags.h
EXTRA_DIST = gedcom.h.in

gedcom-tags.h:	$(srcdir)/../gedcom/gedcom.tab.h
	grep "TAG_\|USERTAG" $< > $@

