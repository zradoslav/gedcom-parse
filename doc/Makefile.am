## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS = images .
DOC_FILES = index.html usage.html parser.html interface.html links.html \
	    encoding.html gomxref.html
VERSIONED_FILES = index.html
EXTRA_DIST = $(DOC_FILES)
docdir = $(datadir)/doc/@PACKAGE@-@VERSION@

include $(top_srcdir)/Makefile.inc

doc_DATA = $(DOC_FILES)

dist-hook:
	$(replace-VERSION)

gomxref.html:	$(srcdir)/../include/gom.h
	./make_gom_xref
