/* Header for date manipulation routines.
   Copyright (C) 2001 The Genes Development Team
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

#ifndef __DATE_H
#define __DATE_H

#include <stdlib.h>
#include "gedcom_internal.h"
#include "gedcom.h"

#define gedcom_date_error gedcom_error
#define MAX_DATE_TOKEN 10

extern struct date_value dv_s;
extern struct date date_s;
extern struct date def_date;
extern char* curr_line_value;

int               gedcom_date_parse();
int               gedcom_date_lex();

/* These are defined in gedcom_date.lex */
void              init_gedcom_date_lex(char* string);
void              close_gedcom_date_lex();

struct date_value make_date_value(Date_value_type t, struct date d1,
				  struct date d2, char* p);
void              copy_date(struct date *to, struct date from);

#endif /* __DATE_H */