/* Include file for the association substructure in the gedcom object model.
   Copyright (C) 2002 The Genes Development Team
   This file is part of the Gedcom parser library.
   Contributed by Peter Verthez <Peter.Verthez@advalvas.be>, 2002.

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

#ifndef __ASSOCIATION_H
#define __ASSOCIATION_H

#include "gom.h"
#include "gom_internal.h"

void association_subscribe();
int write_associations(Gedcom_write_hndl hndl, int parent,
		       struct association *assoc);

DECLARE_CLEANFUNC(association);
DECLARE_ADDFUNC2(association, note_sub);
DECLARE_ADDFUNC2(association, source_citation);
DECLARE_ADDFUNC2(association, user_data);

#endif /* __ASSOCIATION_H */
