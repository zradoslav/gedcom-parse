/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

#ifndef __GEDCOM_INTERNAL_H
#define __GEDCOM_INTERNAL_H
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif

#define MAXGEDCLEVEL    99
#define MAXGEDCLINELEN  255
#define MAXGEDCTAGLEN   31
#define MAXSTDTAGLEN    4
#define MAXGEDCPTRLEN   22
#define GEDCOMTAGOFFSET 257

int        gedcom_error(char* s, ...);
int        gedcom_warning(char* s, ...);
int        gedcom_message(char* s, ...);
int        gedcom_debug_print(char* s, ...);

int        gedcom_parse();
int        gedcom_lex();

extern int line_no;
#endif /* __GEDCOM_INTERNAL_H */
