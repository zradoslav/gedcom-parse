## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

localedir = $(datadir)/locale
INCLUDES = -I$(srcdir)/../intl -I$(srcdir)/../include -I$(srcdir)/../utf8
CFLAGS   = -O2 -W -Wall -pedantic -Wno-long-long @EXTRA_CFLAGS@

EXTRA_DIST = gedcom-check.sh gedcom-sanitize.sh

bin_PROGRAMS = gedcom-check gedcom-sanitize
gedcom_check_SOURCES = gedcom-check.c
gedcom_check_LDFLAGS = -L../gedcom/.libs -L../utf8/.libs @ICONV_LIBPATH@
gedcom_check_LDADD   = $(LIBICONV) -lgedcom -lutf8tools $(LIBICONV)

gedcom_sanitize_SOURCES = gedcom-sanitize.c
gedcom_sanitize_LDFLAGS = -L../gedcom/.libs -L../gom/.libs -L../utf8/.libs \
                          @ICONV_LIBPATH@
gedcom_sanitize_LDADD   = $(LIBICONV) -lgedcom_gom -lgedcom -lutf8tools \
                          $(LIBICONV)
