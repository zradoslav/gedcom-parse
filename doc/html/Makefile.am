## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS = images .
DOC_FILES = encoding.html
DOC_FILES_PREFIXED := $(addprefix $(srcdir)/, $(DOC_FILES))
ALL_HTML_FILES := $(wildcard $(srcdir)/*.html)
GENERATED_DOC_FILES := $(filter-out $(DOC_FILES_PREFIXED), $(ALL_HTML_FILES))
GENERATED_EXTRA_FILES = doxygen.css doxygen.png

EXTRA_DIST = $(ALL_HTML_FILES) $(GENERATED_EXTRA_FILES)
docdir = $(datadir)/doc/@PACKAGE@-@VERSION@
DISTCLEANFILES = $(GENERATED_DOC_FILES) $(GENERATED_EXTRA_FILES)

# doc_DATA = $(ALL_HTML_FILES) $(GENERATED_EXTRA_FILES)
