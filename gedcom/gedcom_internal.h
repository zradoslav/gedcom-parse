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
#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif

#define _(string) gettext(string)
#define N_(string) (string)

#define MAXGEDCLEVEL    99
#define MAXGEDCLINELEN  255
#define MAXGEDCTAGLEN   31
#define MAXSTDTAGLEN    4
#define MAXGEDCPTRLEN   22
#define GEDCOMTAGOFFSET 257
#define INTERNAL_ENCODING "UTF8"

#define GEDCOM_INTERNAL 1

struct tag_struct {
  char *string;
  int value;
};

int        gedcom_parse();
int        gedcom_lex();

void       gedcom_mem_error(char *filename, int line);

#define MEMORY_ERROR gedcom_mem_error(__FILE__, __LINE__)

extern int line_no;
#endif /* __GEDCOM_INTERNAL_H */
