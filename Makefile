# $Id$
# $Name$

YACC=bison
LEX=flex

CFLAGS=-g -W -Wall -pedantic
YFLAGS=--debug --defines
LFLAGS=-8

all:	ansel_module gedcom_parse

gedcom_parse:	standalone.o lex.gedcom_1byte_.o lex.gedcom_hilo_.o \
                lex.gedcom_lohi_.o gedcom.tab.o message.o multilex.o \
		encoding.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

ansel_module:
	cd ansel && $(MAKE)

lex.gedcom_1byte_.c:	gedcom_1byte.lex gedcom.tab.h gedcom.h multilex.h
	$(LEX) $(LFLAGS) -Pgedcom_1byte_ gedcom_1byte.lex

lex.gedcom_hilo_.c:	gedcom_hilo.lex gedcom.tab.h gedcom.h multilex.h
	$(LEX) $(LFLAGS) -Pgedcom_hilo_ gedcom_hilo.lex

lex.gedcom_lohi_.c:	gedcom_lohi.lex gedcom.tab.h gedcom.h multilex.h
	$(LEX) $(LFLAGS) -Pgedcom_lohi_ gedcom_lohi.lex

gedcom.tab.c gedcom.tab.h:	gedcom.y gedcom.h
	$(YACC) $(YFLAGS) --name-prefix=gedcom_ gedcom.y

clean:
	rm -f core gedcom_parse test_* *.o lex.gedcom_* \
        gedcom.tab.* gedcom.output
	cd ansel && $(MAKE) clean

# Lexer test programs

test_1byte:	lex.gedcom_1byte_.test.o message.o encoding.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

lex.gedcom_1byte_.test.o:	lex.gedcom_1byte_.c
	$(CC) -DLEXER_TEST -c $(CPPFLAGS) $(CFLAGS) $^ -o $@

test_hilo:	lex.gedcom_hilo_.test.o message.o encoding.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

lex.gedcom_hilo_.test.o:	lex.gedcom_hilo_.c
	$(CC) -DLEXER_TEST -c $(CPPFLAGS) $(CFLAGS) $^ -o $@

test_lohi:	lex.gedcom_lohi_.test.o message.o encoding.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

lex.gedcom_lohi_.test.o:	lex.gedcom_lohi_.c
	$(CC) -DLEXER_TEST -c $(CPPFLAGS) $(CFLAGS) $^ -o $@

# Test of parser

test:	all
	@export GCONV_PATH=./ansel; \
        for file in t/*.ged; do \
	  echo "=== testing $$file"; \
	  ./gedcom_parse $$file; \
	done
