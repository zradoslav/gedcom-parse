# $Id$
# $Name$

LIBTOOL=libtool
YACC=bison
LEX=flex

LIBPATH=/usr/local/lib
DMALLOC_CFLAGS=
DMALLOC_LOADLIBES=
CFLAGS=-g -W -Wall -pedantic $(DMALLOC_CFLAGS)
YFLAGS=--debug --defines
LFLAGS=-8
LOADLIBES=$(DMALLOC_LOADLIBES)
LDFLAGS_GEDCOM=-L.libs

all:	ansel_module libgedcom.so gedcom_parse

gedcom_parse:	standalone.o
	$(CC) $(LDFLAGS) $(LDFLAGS_GEDCOM) -lgedcom $^ $(LOADLIBES) $(LDLIBS) -o $@

libgedcom.so:	lex.gedcom_1byte_.lo lex.gedcom_hilo_.lo lex.gedcom_lohi_.lo \
		gedcom.tab.lo message.lo multilex.lo encoding.lo interface.lo
	$(LIBTOOL) $(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o libgedcom.la -rpath $(LIBPATH)
	rm -f libgedcom.so
	ln -s .libs/libgedcom.so.0.0.0 libgedcom.so

%.lo:	%.c
	$(LIBTOOL) $(CC) -c $(CPPFLAGS) $(CFLAGS) $^

ansel_module:
	cd ansel && $(MAKE)

lex.gedcom_1byte_.c:	gedcom_1byte.lex gedcom.tab.h gedcom_internal.h multilex.h \
			gedcom_lex_common.c encoding.h
	$(LEX) $(LFLAGS) -Pgedcom_1byte_ gedcom_1byte.lex

lex.gedcom_hilo_.c:	gedcom_hilo.lex gedcom.tab.h gedcom_internal.h multilex.h \
			gedcom_lex_common.c encoding.h
	$(LEX) $(LFLAGS) -Pgedcom_hilo_ gedcom_hilo.lex

lex.gedcom_lohi_.c:	gedcom_lohi.lex gedcom.tab.h gedcom_internal.h multilex.h \
			gedcom_lex_common.c encoding.h
	$(LEX) $(LFLAGS) -Pgedcom_lohi_ gedcom_lohi.lex

gedcom.tab.c gedcom.tab.h:	gedcom.y gedcom_internal.h
	$(YACC) $(YFLAGS) --name-prefix=gedcom_ gedcom.y

.PHONY:	clean
clean:
	rm -f core gedcom_parse lexer_* *.o *.lo *.la .libs/* lex.gedcom_* \
        gedcom.tab.* gedcom.output libgedcom.so
	cd ansel && $(MAKE) clean

# Lexer test programs

lexer_1byte:	lex.gedcom_1byte_.test.o message.o encoding.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

lex.gedcom_1byte_.test.o:	lex.gedcom_1byte_.c
	$(CC) -DLEXER_TEST -c $(CPPFLAGS) $(CFLAGS) $^ -o $@

test_1byte:	lexer_1byte
	cat t/allged.ged | ./lexer_1byte

lexer_hilo:	lex.gedcom_hilo_.test.o message.o encoding.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

lex.gedcom_hilo_.test.o:	lex.gedcom_hilo_.c
	$(CC) -DLEXER_TEST -c $(CPPFLAGS) $(CFLAGS) $^ -o $@

test_hilo:	lexer_hilo
	cat t/uhlcl.ged | ./lexer_hilo

lexer_lohi:	lex.gedcom_lohi_.test.o message.o encoding.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

lex.gedcom_lohi_.test.o:	lex.gedcom_lohi_.c
	$(CC) -DLEXER_TEST -c $(CPPFLAGS) $(CFLAGS) $^ -o $@

test_lohi:	lexer_lohi
	cat t/ulhcl.ged | ./lexer_lohi

# Test of parser

test:	all
	@export GCONV_PATH=./ansel; \
	export LD_LIBRARY_PATH=.libs; \
        for file in t/*.ged; do \
	  echo "=== testing $$file"; \
	  ./gedcom_parse -2 $$file; \
	done

testmem:	DMALLOC_CFLAGS=-DMALLOC
testmem:	DMALLOC_LOADLIBES=-ldmalloc
testmem:	clean test
