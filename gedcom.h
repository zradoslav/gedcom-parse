/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

#ifndef __GEDCOM_H
#define __GEDCOM_H
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define MAXGEDCLEVEL    99
#define MAXGEDCLINELEN  255
#define MAXGEDCTAGLEN   31
#define MAXSTDTAGLEN    4
#define MAXGEDCPTRLEN   22
#define GEDCOMTAGOFFSET 257

/* Error handling mechanisms */
typedef enum _MECH {
  IMMED_FAIL,
  DEFER_FAIL,
  IGNORE_ERRORS
} MECHANISM;


int        gedcom_error(char* s, ...);
int        gedcom_warning(char* s, ...);
int        gedcom_message(char* s, ...);
int        gedcom_debug_print(char* s, ...);
void       gedcom_set_debug_level(int level);
void       gedcom_set_error_handling(MECHANISM mechanism);
void       gedcom_set_compat_handling(int enable_compat);

int        gedcom_parse();

int        gedcom_lex();

extern int line_no;
#endif /* __GEDCOM_H */
