## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS=input
INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" -I $(srcdir)/../include
CFLAGS   = -g -O2

noinst_PROGRAMS = testgedcom pathtest

testgedcom_SOURCES = standalone.c utf8-locale.c
noinst_HEADERS     = utf8-locale.h
testgedcom_LDFLAGS = -L ../gedcom/.libs -lgedcom
testgedcom_LDADD  = @INTLLIBS@

pathtest_SOURCES = pathtest.c
pathtest_LDFLAGS = -L ../gedcom/.libs -lgedcom

TEST_SCRIPT=test_script
TESTS := $(wildcard $(srcdir)/*.test)

EXTRA_DIST=$(TEST_SCRIPT) $(TESTS)

all-local:
	rm -f ../testgedcom.out

check-% :	%.test
	@srcdir=$(srcdir); export srcdir; \
        if $(TESTS_ENVIRONMENT) ./$<; then \
	  echo "Test succeeded"; \
	else \
	  echo "Test failed"; \
	fi
