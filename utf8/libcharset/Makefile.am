## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

noinst_LTLIBRARIES = libcharset.la
libcharset_la_SOURCES = localcharset.c
noinst_HEADERS = libcharset.h
INCLUDES = -DLIBDIR=\"$(libdir)\"

EXTRA_DIST = config.charset ref-add.sin ref-del.sin

all-local: charset.alias ref-add.sed ref-del.sed

charset_alias = $(DESTDIR)$(libdir)/charset.alias
charset_tmp = $(DESTDIR)$(libdir)/charset.tmp
install-exec-local: all-local
	test @GLIBC21@ != no || $(mkinstalldirs) $(DESTDIR)$(libdir)
	if test -f $(charset_alias); then \
	  sed -f ref-add.sed $(charset_alias) > $(charset_tmp) ; \
	  $(INSTALL_DATA) $(charset_tmp) $(charset_alias) ; \
	  rm -f $(charset_tmp) ; \
	else \
	  if test @GLIBC21@ = no; then \
	    sed -f ref-add.sed charset.alias > $(charset_tmp) ; \
	    $(INSTALL_DATA) $(charset_tmp) $(charset_alias) ; \
	    rm -f $(charset_tmp) ; \
	  fi ; \
	fi

uninstall-local: all-local
	if test -f $(charset_alias); then \
	  sed -f ref-del.sed $(charset_alias) > $(charset_tmp); \
	  if grep '^# Packages using this file: $$' $(charset_tmp) \
	      > /dev/null; then \
	    rm -f $(charset_alias); \
	  else \
	    $(INSTALL_DATA) $(charset_tmp) $(charset_alias); \
	  fi; \
	  rm -f $(charset_tmp); \
	fi

charset.alias: config.charset
	$(SHELL) $(srcdir)/config.charset '@host@' > t-$@
	mv t-$@ $@

SUFFIXES = .sed .sin
.sin.sed:
	sed -e '/^#/d' -e 's/@''PACKAGE''@/@PACKAGE@/g' $< > t-$@
	mv t-$@ $@

CLEANFILES = charset.alias ref-add.sed ref-del.sed
