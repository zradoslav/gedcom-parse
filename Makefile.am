## Process this file with automake to produce Makefile.in
# $Id$
# $Name$
SUBDIRS = intl ansel gedcom include . t doc po
INCLUDES = -I $(srcdir)/include

pkgdata_DATA = gedcom.enc
noinst_PROGRAMS = testgedcom

testgedcom_SOURCES = standalone.c
testgedcom_LDFLAGS = -L gedcom/.libs -lgedcom
testgedcom_LDADD  = @INTLLIBS@

EXTRA_DIST = $(pkgdata_DATA)
VERSIONED_FILES = README

dist-hook:
	@cd $(distdir); \
	for file in $(VERSIONED_FILES); do \
          sed 's/\@VERSION\@/${VERSION}/' $$file > $$file.new; \
	  rm $$file; \
	  mv $$file.new $$file; \
	done

clean-local:
	rm -f testgedcom.out
