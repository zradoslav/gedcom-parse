/* Header for cross-reference manipulation routines.
   Copyright (C) 2001,2002 The Genes Development Team
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

#ifndef __XREF_H
#define __XREF_H

#include "gedcom.h"

typedef enum _XREF_CTXT {
  XREF_DEFINED,
  XREF_USED
} Xref_ctxt;

void make_xref_table();
int check_xref_table();

struct xref_value *gedcom_parse_xref(const char *raw_value,
				     Xref_ctxt ctxt, Xref_type type);

#define GEDCOM_MAKE_XREF_PTR(VAR, XREF) \
   GEDCOM_MAKE(VAR, XREF, GV_XREF_PTR, xref_val)

#define GEDCOM_MAKE_NULL_OR_XREF_PTR(VAR, XREF) \
   (XREF == NULL ? \
    GEDCOM_MAKE_NULL(VAR) : \
    GEDCOM_MAKE_XREF_PTR(VAR, XREF))

#endif /* __XREF_H */
