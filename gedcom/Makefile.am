## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

SUBDIRS = calendar .

localedir = $(datadir)/locale
INCLUDES = -I $(srcdir)/../intl -I $(srcdir)/../include -I $(srcdir)/calendar \
           -DPKGDATADIR=\"$(pkgdatadir)\" -DLOCALEDIR=\"$(localedir)\"
YFLAGS   = --debug --defines
LFLAGS   = -8
CFLAGS   = -g -O2 -W -Wall -pedantic

lib_LTLIBRARIES = libgedcom.la
libgedcom_la_SOURCES = lex.gedcom_1byte_.c \
		       lex.gedcom_hilo_.c \
		       lex.gedcom_lohi_.c \
		       lex.gedcom_date_.c \
		       gedcom.tab.c \
		       gedcom_date.tab.c \
                       message.c \
                       multilex.c \
                       encoding.c \
                       interface.c \
		       date.c \
		       hash.c \
		       xref.c \
		       age.c \
		       compat.c
libgedcom_la_LDFLAGS = -export-dynamic -version-info $(LIBVERSION) $(LIBICONV)
libgedcom_la_LIBADD  = calendar/libcalendar.la
BUILT_SOURCES = lex.gedcom_1byte_.c \
	 	lex.gedcom_hilo_.c \
		lex.gedcom_lohi_.c \
		lex.gedcom_date_.c \
		gedcom.tab.c \
		gedcom.tab.h \
		gedcom.tabgen.h \
		gedcom_date.tab.c \
		gedcom_date.tab.h \
		gedcom_date.tabgen.h
noinst_HEADERS = encoding.h \
		 gedcom_internal.h \
		 interface.h \
		 multilex.h \
		 date.h \
		 hash.h \
		 xref.h \
		 age.h \
		 compat.h
EXTRA_DIST = gedcom.y \
	     gedcom_date.y \
	     gedcom_1byte.lex \
	     gedcom_hilo.lex \
	     gedcom_lohi.lex \
	     gedcom_date.lex \
	     gedcom_lex_common.c

gedcom.tab.c gedcom.tab.h:	gedcom.y
	$(YACC) $(YFLAGS) --name-prefix=gedcom_ $<

gedcom_date.tab.c gedcom_date.tab.h:	gedcom_date.y
	$(YACC) $(YFLAGS) --name-prefix=gedcom_date_ $<

lex.gedcom_1byte_.c:	gedcom_1byte.lex
	$(LEX) $(LFLAGS) -Pgedcom_1byte_ $<

lex.gedcom_hilo_.c:	gedcom_hilo.lex
	$(LEX) $(LFLAGS) -Pgedcom_hilo_ $<

lex.gedcom_lohi_.c:	gedcom_lohi.lex
	$(LEX) $(LFLAGS) -Pgedcom_lohi_ $<

lex.gedcom_date_.c:	gedcom_date.lex
	$(LEX) $(LFLAGS) -Pgedcom_date_ $<

lex.gedcom_1byte_.c:	gedcom.tabgen.h gedcom_lex_common.c
lex.gedcom_hilo_.c:	gedcom.tabgen.h gedcom_lex_common.c
lex.gedcom_lohi_.c:	gedcom.tabgen.h gedcom_lex_common.c
lex.gedcom_date_.c:	gedcom_date.tabgen.h

# Lexer test programs

EXTRA_PROGRAMS = lexer_1byte lexer_hilo lexer_lohi
lexer_1byte_SOURCES =
lexer_1byte_LDADD = lex.gedcom_1byte_.test.o message.o encoding.o hash.o
lexer_hilo_SOURCES =
lexer_hilo_LDADD = lex.gedcom_hilo_.test.o message.o encoding.o hash.o
lexer_lohi_SOURCES =
lexer_lohi_LDADD = lex.gedcom_lohi_.test.o message.o encoding.o hash.o

lex.gedcom_1byte_.test.o:	lex.gedcom_1byte_.c
	$(COMPILE) -DLEXER_TEST -c $(CPPFLAGS) $(CFLAGS) $< -o $@
lex.gedcom_hilo_.test.o:	lex.gedcom_hilo_.c
	$(COMPILE) -DLEXER_TEST -c $(CPPFLAGS) $(CFLAGS) $< -o $@
lex.gedcom_lohi_.test.o:	lex.gedcom_lohi_.c
	$(COMPILE) -DLEXER_TEST -c $(CPPFLAGS) $(CFLAGS) $< -o $@

clean-local:
	rm -f $(EXTRA_PROGRAMS)

%.tabgen.h:	%.tab.h
	if [ ! -r $(srcdir)/$@ ] ; \
	then \
	  touch $(srcdir)/$@; \
	fi
	if ! diff -q $< $(srcdir)/$@ > /dev/null; \
	then \
	  rm $(srcdir)/$@; \
	  cp $(srcdir)/$< $(srcdir)/$@; \
	fi
