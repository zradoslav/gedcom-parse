# $Id$
# $Name$

gedcom_parse:	standalone.o lex.gedcom_.o gedcom.tab.o
	cc standalone.o lex.gedcom_.o gedcom.tab.o -o gedcom_parse

lex.gedcom_.c:	gedcom.lex gedcom.tab.h gedcom.h
	flex -8 -Pgedcom_ gedcom.lex

gedcom.tab.c gedcom.tab.h:	gedcom.y gedcom.h
	bison --debug --defines --name-prefix=gedcom_ gedcom.y

clean:
	rm -f core gedcom_parse *.o lex.gedcom_.c gedcom.tab.* gedcom.output
