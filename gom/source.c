/* Source object in the gedcom object model.
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
#include "source.h"
#include "source_event.h"
#include "note_sub.h"
#include "source_description.h"
#include "repository.h"
#include "multimedia_link.h"
#include "user_ref.h"
#include "change_date.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

struct source* gom_first_source = NULL;

DEFINE_MAKEFUNC(source, gom_first_source)
DEFINE_DESTROYFUNC(source, gom_first_source)
DEFINE_ADDFUNC(source, XREF_SOUR)
DEFINE_DELETEFUNC(source)
DEFINE_GETXREFFUNC(source, XREF_SOUR)
     
DEFINE_REC_CB(source, sour_start)
DEFINE_NULL_CB(source, sour_data_start)
DEFINE_STRING_CB(source, sour_data_agnc_start, data.agency)
DEFINE_NULL_CB(source, sour_auth_start)  /* value set by end callback */
DEFINE_STRING_END_CB(source, sour_auth_end, author)
DEFINE_NULL_CB(source, sour_titl_start)  /* value set by end callback */
DEFINE_STRING_END_CB(source, sour_titl_end, title)
DEFINE_STRING_CB(source, sour_abbr_start, abbreviation)
DEFINE_NULL_CB(source, sour_publ_start)  /* value set by end callback */
DEFINE_STRING_END_CB(source, sour_publ_end, publication)
DEFINE_NULL_CB(source, sour_text_start)  /* value set by end callback */
DEFINE_STRING_END_CB(source, sour_text_end, text)
DEFINE_XREF_CB(source, sour_repo_start, repository.link, repository)

DEFINE_ADDFUNC2(source, source_event, data.event)
DEFINE_ADDFUNC2(source, source_description, repository.description)
DEFINE_ADDFUNC2(source, multimedia_link, mm_link)
DEFINE_ADDFUNC2(source, note_sub, note)
DEFINE_ADDFUNC2(source, user_ref_number, ref)
DEFINE_ADDFUNC2(source, user_data, extra)
DEFINE_ADDFUNC2_NOLIST(source, change_date, change_date)
DEFINE_ADDFUNC2_STR(source, record_id)

void source_subscribe()
{
  gedcom_subscribe_to_record(REC_SOUR, sour_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_SOUR_DATA, sour_data_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SOUR_DATA_AGNC, sour_data_agnc_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SOUR_AUTH, sour_auth_start, sour_auth_end);
  gedcom_subscribe_to_element(ELT_SOUR_TITL, sour_titl_start, sour_titl_end);
  gedcom_subscribe_to_element(ELT_SOUR_ABBR, sour_abbr_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SOUR_PUBL, sour_publ_start, sour_publ_end);
  gedcom_subscribe_to_element(ELT_SOUR_TEXT, sour_text_start, sour_text_end);
  gedcom_subscribe_to_element(ELT_SUB_REPO, sour_repo_start, def_elt_end);
}

void source_add_note_to_data(Gom_ctxt ctxt, struct note_sub* note)
{
  struct source *sour = SAFE_CTXT_CAST(source, ctxt);
  if (sour)
    LINK_CHAIN_ELT(note_sub, sour->data.note, note);  
}

void source_add_note_to_repo(Gom_ctxt ctxt, struct note_sub* note)
{
  struct source *sour = SAFE_CTXT_CAST(source, ctxt);
  if (sour)
    LINK_CHAIN_ELT(note_sub, sour->repository.note, note);  
}

void CLEANFUNC(source)(struct source* sour)
{
  if (sour) {
    SAFE_FREE(sour->xrefstr);
    DESTROY_CHAIN_ELTS(source_event, sour->data.event);
    SAFE_FREE(sour->data.agency)
    DESTROY_CHAIN_ELTS(note_sub, sour->data.note);
    SAFE_FREE(sour->author);
    SAFE_FREE(sour->title);
    SAFE_FREE(sour->abbreviation);
    SAFE_FREE(sour->publication);
    SAFE_FREE(sour->text);
    DESTROY_CHAIN_ELTS(note_sub, sour->repository.note);
    DESTROY_CHAIN_ELTS(source_description, sour->repository.description);
    DESTROY_CHAIN_ELTS(multimedia_link, sour->mm_link);
    DESTROY_CHAIN_ELTS(note_sub, sour->note);
    DESTROY_CHAIN_ELTS(user_ref_number, sour->ref);
    SAFE_FREE(sour->record_id);
    CLEANFUNC(change_date)(sour->change_date);
    DESTROY_CHAIN_ELTS(user_data, sour->extra);
  }
}

void sources_cleanup()
{
  DESTROY_CHAIN_ELTS(source, gom_first_source);
}

struct source* gom_get_first_source()
{
  return gom_first_source;
}

int write_sources(Gedcom_write_hndl hndl)
{
  int result = 0;
  struct source* obj;

  for (obj = gom_first_source; obj; obj = obj->next) {
    result |= gedcom_write_record_str(hndl, REC_SOUR, obj->xrefstr, NULL);
    if (obj->data.event || obj->data.agency || obj->data.note)
      result |= gedcom_write_element_str(hndl, ELT_SOUR_DATA, 0,
					 REC_SOUR, NULL);
    if (obj->data.event)
      result |= write_source_events(hndl, ELT_SOUR_DATA, obj->data.event);
    if (obj->data.agency)
      result |= gedcom_write_element_str(hndl, ELT_SOUR_DATA_AGNC, 0,
					 ELT_SOUR_DATA, obj->data.agency);
    if (obj->data.note)
      result |= write_note_subs(hndl, ELT_SOUR_DATA, obj->data.note);
    if (obj->author)
      result |= gedcom_write_element_str(hndl, ELT_SOUR_AUTH, 0,
					 REC_SOUR, obj->author);
    if (obj->title)
      result |= gedcom_write_element_str(hndl, ELT_SOUR_TITL, 0,
					 REC_SOUR, obj->title);
    if (obj->abbreviation)
      result |= gedcom_write_element_str(hndl, ELT_SOUR_ABBR, 0,
					 REC_SOUR, obj->abbreviation);
    if (obj->publication)
      result |= gedcom_write_element_str(hndl, ELT_SOUR_PUBL, 0,
					 REC_SOUR, obj->publication);
    if (obj->text)
      result |= gedcom_write_element_str(hndl, ELT_SOUR_TEXT, 0,
					 REC_SOUR, obj->text);
    if (obj->repository.link || obj->repository.note
	|| obj->repository.description) {
      result |= gedcom_write_element_xref(hndl, ELT_SUB_REPO, 0,
					  REC_SOUR, obj->repository.link);
    }
    if (obj->repository.note)
      result |= write_note_subs(hndl, ELT_SUB_REPO, obj->repository.note);
    if (obj->repository.description)
      result |= write_source_descriptions(hndl, ELT_SUB_REPO,
					  obj->repository.description);
    if (obj->mm_link)
      result |= write_multimedia_links(hndl, REC_SOUR, obj->mm_link);
    if (obj->note)
      result |= write_note_subs(hndl, REC_SOUR, obj->note);
    if (obj->ref)
      result |= write_user_refs(hndl, REC_SOUR, obj->ref);
    if (obj->record_id)
      result |= gedcom_write_element_str(hndl, ELT_SUB_IDENT_RIN, 0,
					 REC_SOUR, obj->record_id);
    if (obj->change_date)
      result |= write_change_date(hndl, REC_SOUR, obj->change_date);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }
  
  return result;
}

