## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" -I $(srcdir)/../../include \
           -I $(srcdir)/../../utf8
CFLAGS   = -g -O2

noinst_PROGRAMS = testgedcom pathtest gomtest updatetest @EXTRA_PROGS@
EXTRA_PROGRAMS  = gomtest_static
noinst_HEADERS = output.h dump_gom.h

testgedcom_SOURCES = standalone.c output.c
testgedcom_LDFLAGS = -L ../../gedcom/.libs -lgedcom
testgedcom_LDADD  = ../../utf8/libutf8.la

pathtest_SOURCES = pathtest.c
pathtest_LDFLAGS = -L ../../gedcom/.libs -lgedcom

gomtest_SOURCES = gomtest.c output.c dump_gom.c
gomtest_LDFLAGS = -L ../../gedcom/.libs -L ../../gom/.libs -lgedcom_gom -lgedcom

gomtest_static_SOURCES = gomtest.c output.c
gomtest_static_LDADD = ../../gedcom/.libs/libgedcom.a \
		       ../../gom/.libs/libgedcom_gom.a

updatetest_SOURCES = update.c output.c
updatetest_LDFLAGS = -L ../../gedcom/.libs -lgedcom

TEST_SCRIPT=test_script test_gom test_update

EXTRA_DIST=$(TEST_SCRIPT)
