/* $Id$ */
/* $Name$ */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#define MAXGEDCOMLEVEL 99
#define MAXSTDTAGLENGTH 4
#define GEDCOMTAGOFFSET 257

/* Error handling mechanisms */
typedef enum _MECH {
  FAIL_PARSE,
  IGNORE_RECORD,
  IGNORE_LINE
} MECHANISM;

int        gedcom_error(char* s, ...);
void       gedcom_enable_debug();
void       gedcom_set_error_handling(MECHANISM mechanism);
int        gedcom_parse();
extern int line_no;
