## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS=input
INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" -I $(srcdir)/../include
CFLAGS   = -g -O2

noinst_PROGRAMS = testgedcom pathtest gomtest @EXTRA_PROGS@
EXTRA_PROGRAMS  = gomtest_static

testgedcom_SOURCES = standalone.c utf8-locale.c
noinst_HEADERS     = utf8-locale.h
testgedcom_LDFLAGS = -L ../gedcom/.libs -lgedcom
testgedcom_LDADD  = @INTLLIBS@

pathtest_SOURCES = pathtest.c
pathtest_LDFLAGS = -L ../gedcom/.libs -lgedcom

gomtest_SOURCES = gomtest.c
gomtest_LDFLAGS = -L ../gedcom/.libs -L ../gom/.libs -lgedcom_gom -lgedcom

gomtest_static_SOURCES = gomtest.c
gomtest_static_LDADD = ../gedcom/.libs/libgedcom.a ../gom/.libs/libgedcom_gom.a
DMALLOC_LOG = dmalloc.log

TEST_SCRIPT=test_script test_gom
TESTS := $(wildcard $(srcdir)/*.test)

EXTRA_DIST=$(TEST_SCRIPT) $(TESTS)

all-local:
	@rm -f testgedcom.out $(DMALLOC_LOG)

check-% :	%.test all-local
	@srcdir=$(srcdir); export srcdir; \
        if $(TESTS_ENVIRONMENT) ./$<; then \
	  echo "Test succeeded"; \
	else \
	  echo "Test failed"; \
	fi

check-dmalloc-%:	%.test all-local
	@if [ "@EXTRA_PROGS@" = "" ]; then \
	  echo "Please run './configure --with-dmalloc' first"; \
	  exit; \
	fi; \
	srcdir=$(srcdir); export srcdir; \
	GOM_DMALLOC_TEST=$(DMALLOC_LOG); export GOM_DMALLOC_TEST; \
        if $(TESTS_ENVIRONMENT) ./$<; then \
	  echo "Test succeeded"; \
	else \
	  echo "Test failed"; \
	fi

