## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS=input

TEST_SCRIPT=test_script
TESTS := $(wildcard $(srcdir)/*.test)

EXTRA_DIST=$(TEST_SCRIPT) $(TESTS)

all-local:
	rm -f ../testgedcom.out
