## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
include_HEADERS = gedcom.h \
		  gedcom-tags.h
BUILT_SOURCES = gedcom-tags.h

gedcom-tags.h:	$(srcdir)/../gedcom/gedcom.tab.h
	cat $(srcdir)/../gedcom/gedcom.tab.h | grep "TAG_\|USERTAG" > gedcom-tags.h

