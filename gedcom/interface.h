/* Header for interface.c
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

#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "gedcom_internal.h"
#include "gedcom.h"

Gedcom_ctxt start_record(Gedcom_rec rec,
			 int level, Gedcom_val xref, struct tag_struct tag,
			 char *raw_value, Gedcom_val parsed_value);
void        end_record(Gedcom_rec rec, Gedcom_ctxt self,
		       Gedcom_val parsed_value);

Gedcom_ctxt start_element(Gedcom_elt elt, Gedcom_ctxt parent,
			  int level, struct tag_struct tag, char *raw_value,
			  Gedcom_val parsed_value);
void        end_element(Gedcom_elt elt, Gedcom_ctxt parent, Gedcom_ctxt self,
			Gedcom_val parsed_value);

extern Gedcom_val_struct val1;
extern Gedcom_val_struct val2;

#define GEDCOM_MAKE(VAR, VALUE, TYPE, MEMBER) \
   (VAR.type = TYPE, VAR.value.MEMBER = VALUE, &VAR)

#define GEDCOM_MAKE_NULL(VAR) \
   GEDCOM_MAKE(VAR, NULL, GV_NULL, string_val)

#define GEDCOM_MAKE_STRING(VAR, STRING) \
   GEDCOM_MAKE(VAR, STRING, GV_CHAR_PTR, string_val)

#define GEDCOM_MAKE_NULL_OR_STRING(VAR, STRING) \
   (STRING == NULL ? \
    GEDCOM_MAKE_NULL(VAR) : \
    GEDCOM_MAKE_STRING(VAR, STRING))


#endif /* __INTERFACE_H */
