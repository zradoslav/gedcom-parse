## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS=src input .

DMALLOC_LOG = dmalloc.log
TESTS := $(wildcard $(srcdir)/*.test)

EXTRA_DIST=$(TESTS)

TESTS_ENVIRONMENT=ICONV_PATH=@ICONV_PATH@

all-local:
	@rm -f *.out $(DMALLOC_LOG)

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

