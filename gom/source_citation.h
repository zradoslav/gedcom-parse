/* Include file for the source citation substructure in the gedcom object
   model.
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

#ifndef __SOURCE_CITATION_H
#define __SOURCE_CITATION_H

#include "gom.h"
#include "gom_internal.h"

void citation_subscribe();
void citation_cleanup(struct source_citation* cit);
void citation_add_note(Gom_ctxt ctxt, struct note_sub* note);
void citation_add_mm_link(Gom_ctxt ctxt, struct multimedia_link* mm);
void citation_add_to_desc(NL_TYPE type, Gom_ctxt ctxt, const char* str);
void citation_add_to_text(NL_TYPE type, Gom_ctxt ctxt, const char* str);
void citation_add_user_data(Gom_ctxt ctxt, struct user_data* data);

#endif /* __SOURCE_CITATION_H */
