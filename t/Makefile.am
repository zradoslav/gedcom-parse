## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

TEST_SCRIPT=test_script

TESTS_ENVIRONMENT=$(srcdir)/$(TEST_SCRIPT)
TESTS = allged.ged \
	ansel.ged \
        uhlbomcl.ged \
	uhlcl.ged \
	ulhbomcl.ged \
	ulhc.ged \
	ulhcl.ged \
	ulhl.ged \
	ulhlc.ged

EXTRA_DIST=$(TEST_SCRIPT) $(TESTS)

all-local:
	rm -f ../testgedcom.out
