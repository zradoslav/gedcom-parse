# $Id$
# $Name$

DMALLOC_CFLAGS=
DMALLOC_LOADLIBES=
CFLAGS=-g -W -Wall -pedantic $(DMALLOC_CFLAGS)
CPPFLAGS=-I ./include
YFLAGS=--debug --defines
LFLAGS=-8
LOADLIBES=$(DMALLOC_LOADLIBES)
LDFLAGS_GEDCOM=-Lgedcom/.libs

all:	ansel_module libgedcom gedcom_parse

gedcom_parse:	standalone.o
	$(CC) $(LDFLAGS) $(LDFLAGS_GEDCOM) -lgedcom $^ $(LOADLIBES) $(LDLIBS) -o $@

libgedcom:
	cd gedcom && $(MAKE) DMALLOC_CFLAGS=$(DMALLOC_CFLAGS) \
	DMALLOC_LOADLIBES=$(DMALLOC_LOADLIBES)

ansel_module:
	cd ansel && $(MAKE)

.PHONY:	clean
clean:
	rm -f core gedcom_parse *.o logfile
	cd gedcom && $(MAKE) clean
	cd ansel && $(MAKE) clean

# Test of parser

test:	all
	@export GCONV_PATH=./ansel; \
	export LD_LIBRARY_PATH=gedcom/.libs; \
        for file in t/*.ged; do \
	  echo "=== testing $$file"; \
	  ./gedcom_parse -2 $$file; \
	done

testmem:	DMALLOC_CFLAGS=-DMALLOC
testmem:	DMALLOC_LOADLIBES=-ldmalloc
testmem:	clean test
