## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

EXTRA_DIST = index.html usage.html parser.html interface.html links.html
VERSIONED_FILES = index.html

dist-hook:
	@cd $(distdir); \
	mkdir images
	cp -p $(srcdir)/images/schema.obj $(srcdir)/images/schema.png images \
	for file in $(VERSIONED_FILES); do \
          sed 's/\@VERSION\@/${VERSION}/' $$file > $$file.new; \
	  rm $$file; \
	  mv $$file.new $$file; \
	done
