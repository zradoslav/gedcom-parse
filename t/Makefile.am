## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS=src input output .

TESTS := $(wildcard $(srcdir)/*.test)

EXTRA_DIST=$(TESTS)

TESTS_ENVIRONMENT=ICONV_PATH=@ICONV_PATH@

all-local:
	@rm -f *.out

check-% :	%.test all-local
	@srcdir=$(srcdir); export srcdir; \
        if $(TESTS_ENVIRONMENT) ./$<; then \
	  echo "Test succeeded"; \
	else \
	  echo "Test failed"; \
	fi
