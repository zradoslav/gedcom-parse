## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS = images .
DOC_FILES = index.html usage.html parser.html interface.html links.html \
	    encoding.html 
GENERATED_DOC_FILES = gomxref.html
DOC_GEN = make_gom_xref
EXTRA_DIST = $(DOC_FILES) $(DOC_GEN) index.html.in $(GENERATED_DOC_FILES)
docdir = $(datadir)/doc/@PACKAGE@-@VERSION@
DISTCLEANFILES = $(GENERATED_DOC_FILES)

doc_DATA = $(DOC_FILES) $(GENERATED_DOC_FILES)

gomxref.html:	$(srcdir)/../include/gom.h
	srcdir=$(srcdir); export srcdir; \
        perl $(srcdir)/make_gom_xref
