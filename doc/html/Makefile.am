## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS = images .
DOC_FILES = encoding.html interface.html
GENERATED_DOC_FILES = utf8tools.html gomxref.html
DOC_FILES_PREFIXED := $(addprefix $(srcdir)/, $(DOC_FILES) $(GENERATED_DOC_FILES))

ALL_HTML_FILES := $(wildcard $(srcdir)/*.html)
DOXYGEN_FILES := $(filter-out $(DOC_FILES_PREFIXED), $(ALL_HTML_FILES))
DOXYGEN_EXTRA_FILES = doxygen.css $(wildcard $(srcdir)/*.png)

EXTRA_DIST = $(ALL_HTML_FILES) $(DOXYGEN_EXTRA_FILES)
docdir = $(datadir)/doc/@PACKAGE@-@VERSION@
DISTCLEANFILES = $(DOXYGEN_FILES) $(DOXYGEN_EXTRA_FILES) $(GENERATED_DOC_FILES)

doc_DATA = $(DOXYGEN_FILES) $(DOXYGEN_EXTRA_FILES) $(DOC_FILES) \
           $(GENERATED_DOC_FILES)

gomxref.html:	$(srcdir)/../../include/gom.h $(srcdir)/../make_gom_xref
	srcdir=$(srcdir); export srcdir; \
        perl $(srcdir)/../make_gom_xref $(srcdir)/../../include/gom.h

utf8tools.html:	$(srcdir)/../../utf8/doc/utf8tools.html
	cp $< $@
