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
    note = make_note_record(xr->string);
    xr->object = (Gedcom_ctxt) note;
  }
  if (note) {
    note->text = strdup(GEDCOM_STRING(parsed_value));
    if (! note->text)
      MEMORY_ERROR;
    else
      result = MAKE_GOM_CTXT(rec, note, xr->object);
  }
  return (Gedcom_ctxt)result;
}

GET_REC_BY_XREF(note, XREF_NOTE, gom_get_note_by_xref)
     
Gedcom_ctxt sub_cont_conc_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    char *str = GEDCOM_STRING(parsed_value);
    NL_TYPE type = (elt == ELT_SUB_CONT ? WITH_NL : WITHOUT_NL);
    switch (ctxt->ctxt_type) {
      case ELT_HEAD_NOTE:
	header_add_to_note(type, ctxt, str); break;
      case ELT_SUB_SOUR:
	citation_add_to_desc(type, ctxt, str); break;
      case ELT_SUB_SOUR_TEXT:
	citation_add_to_text(type, ctxt, str); break;
      case ELT_SUB_NOTE:
	note_sub_add_to_note(type, ctxt, str); break;
      case REC_NOTE:
	note_add_to_note(type, ctxt, str); break;
      case ELT_SOUR_AUTH:
      case ELT_SOUR_TITL:
      case ELT_SOUR_PUBL:
      case ELT_SOUR_TEXT:
	source_add_to_value(type, ctxt, str); break;
      default:
	UNEXPECTED_CONTEXT(ctxt->ctxt_type);
    }
    result = make_gom_ctxt(elt, ctxt->obj_type, ctxt->ctxt_ptr);
  }
  return (Gedcom_ctxt)result;
}

void note_subscribe()
{
  gedcom_subscribe_to_record(REC_NOTE, note_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_SUB_CONT, sub_cont_conc_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_CONC, sub_cont_conc_start, def_elt_end);
}

void note_add_to_note(NL_TYPE type, Gom_ctxt ctxt, const char* str)
{
  struct note *note = SAFE_CTXT_CAST(note, ctxt);
  if (note) {
    char *newvalue = concat_strings (type, note->text, str);
    if (newvalue)
      note->text = newvalue;
    else
      MEMORY_ERROR;
  }
}

void note_add_citation(Gom_ctxt ctxt, struct source_citation* cit)
{
  struct note *note = SAFE_CTXT_CAST(note, ctxt);
  if (note)
    LINK_CHAIN_ELT(source_citation, note->citation, cit);    
}

void note_add_user_ref(Gom_ctxt ctxt, struct user_ref_number* ref)
{
  struct note *note = SAFE_CTXT_CAST(note, ctxt);
  if (note)
    LINK_CHAIN_ELT(user_ref_number, note->ref, ref);
}

void note_set_record_id(Gom_ctxt ctxt, const char *rin)
{
  struct note *note = SAFE_CTXT_CAST(note, ctxt);
  if (note) {
    note->record_id = strdup(rin);
    if (! note->record_id) MEMORY_ERROR;
  }
}

void note_set_change_date(Gom_ctxt ctxt, struct change_date* chan)
{
  struct note *note = SAFE_CTXT_CAST(note, ctxt);
  if (note)
    note->change_date = chan;
}

void note_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct note *obj = SAFE_CTXT_CAST(note, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void note_cleanup(struct note* note)
{
  if (note) {
    SAFE_FREE(note->xrefstr);
    SAFE_FREE(note->text);
    DESTROY_CHAIN_ELTS(source_citation, note->citation, citation_cleanup);
    DESTROY_CHAIN_ELTS(user_ref_number, note->ref, user_ref_cleanup);
    SAFE_FREE(note->record_id);
    change_date_cleanup(note->change_date);
    DESTROY_CHAIN_ELTS(user_data, note->extra, user_data_cleanup);
  }
}

void notes_cleanup()
{
  DESTROY_CHAIN_ELTS(note, gom_first_note, note_cleanup);
}

struct note* gom_get_first_note()
{
  return gom_first_note;
}

struct note* make_note_record(const char* xrefstr)
{
  struct note* note = NULL;
  MAKE_CHAIN_ELT(note, gom_first_note, note);
  if (note) {
    note->xrefstr = strdup(xrefstr);
    if (! note->xrefstr) MEMORY_ERROR;
  }
  return note;
}
