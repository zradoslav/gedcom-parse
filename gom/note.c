/* Note sub-structure in the gedcom object model.
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

#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "source_citation.h"
#include "note.h"
#include "note_sub.h"
#include "user_ref.h"
#include "change_date.h"
#include "source.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

struct note* gom_first_note = NULL;

Gedcom_ctxt note_start(_REC_PARAMS_)
{
  Gom_ctxt result = NULL;
  struct xref_value* xr = GEDCOM_XREF_PTR(xref);
  struct note* note = (struct note*) xr->object;
  if (! note) {
    note = MAKEFUNC(note)(xr->string);
    xr->object = (Gedcom_ctxt) note;
  }
  if (note)
    result = MAKE_GOM_CTXT(rec, note, xr->object);
  return (Gedcom_ctxt)result;
}

DEFINE_MAKEFUNC(note, gom_first_note)
DEFINE_DESTROYFUNC(note, gom_first_note)
DEFINE_ADDFUNC(note, XREF_NOTE)
DEFINE_DELETEFUNC(note)
DEFINE_GETXREFFUNC(note, XREF_NOTE)
DEFINE_MAKELINKFUNC(note, XREF_NOTE)
     
DEFINE_STRING_END_REC_CB(note, note_end, text)

DEFINE_ADDFUNC2(note, source_citation, citation)
DEFINE_ADDFUNC2(note, user_ref_number, ref)
DEFINE_ADDFUNC2(note, user_data, extra)
DEFINE_ADDFUNC2_NOLIST(note, change_date, change_date)
DEFINE_ADDFUNC2_STR(note, record_id)
     
Gedcom_ctxt sub_cont_conc_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else
    result = make_gom_ctxt(elt, ctxt->obj_type, ctxt->ctxt_ptr);
  
  return (Gedcom_ctxt)result;
}

void note_subscribe()
{
  gedcom_subscribe_to_record(REC_NOTE, note_start, note_end);
  gedcom_subscribe_to_element(ELT_SUB_CONT, sub_cont_conc_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_CONC, sub_cont_conc_start, def_elt_end);
}

void CLEANFUNC(note)(struct note* note)
{
  if (note) {
    SAFE_FREE(note->xrefstr);
    SAFE_FREE(note->text);
    DESTROY_CHAIN_ELTS(source_citation, note->citation);
    DESTROY_CHAIN_ELTS(user_ref_number, note->ref);
    SAFE_FREE(note->record_id);
    CLEANFUNC(change_date)(note->change_date);
    DESTROY_CHAIN_ELTS(user_data, note->extra);
  }
}

void notes_cleanup()
{
  DESTROY_CHAIN_ELTS(note, gom_first_note);
}

struct note* gom_get_first_note()
{
  return gom_first_note;
}

int write_notes(Gedcom_write_hndl hndl)
{
  int result = 0;
  struct note* obj;

  for (obj = gom_first_note; obj; obj = obj->next) {
    result |= gedcom_write_record_str(hndl, REC_NOTE, obj->xrefstr, obj->text);
    if (obj->citation)
      result |= write_citations(hndl, REC_NOTE, obj->citation);
    if (obj->ref)
      result |= write_user_refs(hndl, REC_NOTE, obj->ref);
    if (obj->record_id)
      result |= gedcom_write_element_str(hndl, ELT_SUB_IDENT_RIN, 0,
					 REC_NOTE, obj->record_id);
    if (obj->change_date)
      result |= write_change_date(hndl, REC_NOTE, obj->change_date);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }
  
  return result;
}

