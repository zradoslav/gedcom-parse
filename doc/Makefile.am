## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

EXTRA_DIST = index.html usage.html parser.html interface.html links.html
VERSIONED_FILES = index.html

dist-hook:
	mkdir -p $(distdir)/images;
	cp -p $(srcdir)/images/schema.obj $(srcdir)/images/schema.png \
	   $(distdir)/images;
	cd $(distdir); \
	for file in $(VERSIONED_FILES); do \
          sed 's/\@VERSION\@/${VERSION}/' $$file > $$file.new; \
	  rm $$file; \
	  mv $$file.new $$file; \
	done
