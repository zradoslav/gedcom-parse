/* $Id$ */
/* $Name$ */

#ifndef __MULTILEX_H
#define __MULTILEX_H
#include <stdio.h>

int        gedcom_parse_file(char* file_name);

int        gedcom_1byte_lex();
extern FILE *gedcom_1byte_in;

int        gedcom_hilo_lex();
extern FILE *gedcom_hilo_in;

int        gedcom_lohi_lex();
extern FILE *gedcom_lohi_in;
#endif /* __MULTILEX_H */
