/* General header for the Gedcom parser.
   Copyright (C) 2001, 2002 The Genes Development Team
   This file is part of the Gedcom parser library.
   Contributed by Peter Verthez <Peter.Verthez@advalvas.be>, 2001.

   The Gedcom parser library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The Gedcom parser library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Gedcom parser library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* $Id$ */
/* $Name$ */

#ifndef __GEDCOM_INTERNAL_H
#define __GEDCOM_INTERNAL_H
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <libintl.h>

#define _(string) dgettext(PACKAGE, string)
#define N_(string) (string)

#define MAXGEDCLEVEL    99
#define MAXGEDCLINELEN  255
#define MAXGEDCTAGLEN   31
#define MAXSTDTAGLEN    4
#define MAXGEDCPTRLEN   22
#define GEDCOMTAGOFFSET 257
#define INTERNAL_ENCODING "UTF-8"

#define GEDCOM_INTERNAL 1

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

struct tag_struct {
  char *string;
  int value;
};

typedef enum _PARSE_STATE {
  STATE_NORMAL,
  STATE_INITIAL,
  STATE_EXPECT_TAG
} ParseState;

int  gedcom_parse();
int  gedcom_lex();
int  gedcom_check_token(const char* str, ParseState state, int check_token);
void gedcom_enable_internal_debug();

void gedcom_mem_error(const char *filename, int line);

#define MEMORY_ERROR gedcom_mem_error(__FILE__, __LINE__)
#define VALUE_IF_MISSING "-" 

extern int line_no;
extern int init_called;
extern int gedcom_high_level_debug; 
extern FILE* trace_output;


#endif /* __GEDCOM_INTERNAL_H */
