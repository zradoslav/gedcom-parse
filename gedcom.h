/* $Id$ */
/* $Name$ */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXGEDCLEVEL 99
#define MAXGEDCLINELEN 256
#define MAXGEDCTAGLEN 31
#define MAXSTDTAGLENGTH 4
#define GEDCOMTAGOFFSET 257

/* Error handling mechanisms */
typedef enum _MECH {
  IMMED_FAIL,
  DEFER_FAIL,
  IGNORE_ERRORS
} MECHANISM;

/* Basic file encoding */
typedef enum _ENC {
  ONE_BYTE,
  TWO_BYTE_HILO,
  TWO_BYTE_LOHI
} ENCODING;

int        gedcom_error(char* s, ...);
int        gedcom_warning(char* s, ...);
int        gedcom_debug_print(char* s, ...);
void       gedcom_set_debug_level(int level);
void       gedcom_set_error_handling(MECHANISM mechanism);
void       gedcom_set_compat_handling(int enable_compat);
int        gedcom_parse();
int        gedcom_lex();
extern int line_no;
extern FILE *gedcom_in;
