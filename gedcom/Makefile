# $Id$
# $Name$

LIBTOOL=libtool
YACC=bison
LEX=flex

LIBPATH=/usr/local/lib
DMALLOC_CFLAGS=
DMALLOC_LOADLIBES=
CFLAGS=-g -W -Wall -pedantic $(DMALLOC_CFLAGS)
CPPFLAGS=-I ../include
YFLAGS=--debug --defines
LFLAGS=-8
LOADLIBES=$(DMALLOC_LOADLIBES)

all:	libgedcom.so

libgedcom.so:	lex.gedcom_1byte_.lo lex.gedcom_hilo_.lo lex.gedcom_lohi_.lo \
		gedcom.tab.lo message.lo multilex.lo encoding.lo interface.lo
	$(LIBTOOL) $(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o libgedcom.la -rpath $(LIBPATH)
	rm -f libgedcom.so
	ln -s .libs/libgedcom.so.0.0.0 libgedcom.so

%.lo:	%.c
	$(LIBTOOL) $(CC) -c $(CPPFLAGS) $(CFLAGS) $^

lex.gedcom_1byte_.c:	gedcom_1byte.lex gedcom.tab.h gedcom_internal.h \
			multilex.h ../include/gedcom.h \
			gedcom_lex_common.c encoding.h
	$(LEX) $(LFLAGS) -Pgedcom_1byte_ gedcom_1byte.lex

lex.gedcom_hilo_.c:	gedcom_hilo.lex gedcom.tab.h gedcom_internal.h \
			multilex.h ../include/gedcom.h \
			gedcom_lex_common.c encoding.h
	$(LEX) $(LFLAGS) -Pgedcom_hilo_ gedcom_hilo.lex

lex.gedcom_lohi_.c:	gedcom_lohi.lex gedcom.tab.h gedcom_internal.h \
			multilex.h ../include/gedcom.h \
			gedcom_lex_common.c encoding.h
	$(LEX) $(LFLAGS) -Pgedcom_lohi_ gedcom_lohi.lex

gedcom.tab.c gedcom.tab.h:	gedcom.y gedcom_internal.h ../include/gedcom.h
	$(YACC) $(YFLAGS) --name-prefix=gedcom_ gedcom.y
.PHONY:	clean
clean:
	rm -f lexer_* *.o *.lo *.la .libs/* lex.gedcom_* \
        gedcom.tab.* gedcom.output libgedcom.so
	rm -rf .libs

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

