/* Include file for the place substructure in the gedcom object model.
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

#ifndef __PLACE_H
#define __PLACE_H

#include "gom.h"
#include "gom_internal.h"

void place_subscribe();
void place_cleanup(struct place* place);
void place_add_citation(Gom_ctxt ctxt, struct source_citation* cit);
void place_add_note(Gom_ctxt ctxt, struct note_sub* note);
void place_add_user_data(Gom_ctxt ctxt, struct user_data* data);
int write_place(Gedcom_write_hndl hndl, int parent, struct place* place);

#endif /* __PLACE_H */
