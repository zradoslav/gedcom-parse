## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" -I $(srcdir)/../../include \
           -I $(srcdir)/../../utf8
CFLAGS   = -g -O2 @EXTRA_CFLAGS@

noinst_PROGRAMS = testgedcom pathtest gomtest updatetest @EXTRA_PROGS@
EXTRA_PROGRAMS  = gomtest_static
noinst_HEADERS = output.h dump_gom.h portability.h

testgedcom_SOURCES = standalone.c output.c portability.c
testgedcom_LDFLAGS = -L../../gedcom/.libs
testgedcom_LDADD  = -lgedcom ../../utf8/libutf8.la @INTLLIBS@

pathtest_SOURCES = pathtest.c
pathtest_LDFLAGS = -L../../gedcom/.libs
pathtest_LDADD = -lgedcom @INTLLIBS@

gomtest_SOURCES = gomtest.c output.c dump_gom.c portability.c
gomtest_LDFLAGS = -L../../gedcom/.libs -L../../gom/.libs
gomtest_LDADD = -lgedcom_gom -lgedcom @INTLLIBS@

gomtest_static_SOURCES = gomtest.c output.c dump_gom.c portability.c
gomtest_static_LDADD = ../../gedcom/.libs/libgedcom.a \
		       ../../gom/.libs/libgedcom_gom.a

updatetest_SOURCES = update.c output.c portability.c
updatetest_LDFLAGS = -L../../gedcom/.libs
updatetest_LDADD = -lgedcom @INTLLIBS@

TEST_SCRIPT=test_script test_gom test_update

EXTRA_DIST=$(TEST_SCRIPT)
