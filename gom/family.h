/* Include file for the family object in the gedcom object model.
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

#ifndef __FAMILY_H
#define __FAMILY_H

#include "gom.h"
#include "gom_internal.h"

void family_subscribe();
void families_cleanup();
struct family* make_family_record(const char* xref);
void family_add_event(Gom_ctxt ctxt, struct event* evt);
void family_add_lss(Gom_ctxt ctxt, struct lds_event* lss);
void family_add_citation(Gom_ctxt ctxt, struct source_citation* cit);
void family_add_mm_link(Gom_ctxt ctxt, struct multimedia_link* link);
void family_add_note(Gom_ctxt ctxt, struct note_sub* note);
void family_add_user_ref(Gom_ctxt ctxt, struct user_ref_number* ref);
void family_set_record_id(Gom_ctxt ctxt, const char *rin);
void family_set_change_date(Gom_ctxt ctxt, struct change_date* chan);
void family_add_user_data(Gom_ctxt ctxt, struct user_data* data);

#endif /* __FAMILY_H */
