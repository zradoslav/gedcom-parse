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

REC_CB(source, sour_start, make_source_record)
GET_REC_BY_XREF(source, XREF_SOUR, gom_get_source_by_xref)
NULL_CB(source, sour_data_start)
STRING_CB(source, sour_data_agnc_start, data.agency)
NULL_CB(source, sour_auth_start)  /* value set by end callback */
STRING_END_CB(source, sour_auth_end, author)
NULL_CB(source, sour_titl_start)  /* value set by end callback */
STRING_END_CB(source, sour_titl_end, title)
STRING_CB(source, sour_abbr_start, abbreviation)
NULL_CB(source, sour_publ_start)  /* value set by end callback */
STRING_END_CB(source, sour_publ_end, publication)
NULL_CB(source, sour_text_start)  /* value set by end callback */
STRING_END_CB(source, sour_text_end, text)
XREF_CB(source, sour_repo_start, repository.link, make_repository_record)

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

void source_add_event(Gom_ctxt ctxt, struct source_event* evt)
{
  struct source *sour = SAFE_CTXT_CAST(source, ctxt);
  if (sour)
    LINK_CHAIN_ELT(source_event, sour->data.event, evt);  
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

void source_add_description(Gom_ctxt ctxt, struct source_description* desc)
{
  struct source *sour = SAFE_CTXT_CAST(source, ctxt);
  if (sour)
    LINK_CHAIN_ELT(source_description, sour->repository.description, desc);  
}

void source_add_mm_link(Gom_ctxt ctxt, struct multimedia_link* link)
{
  struct source *sour = SAFE_CTXT_CAST(source, ctxt);
  if (sour)
    LINK_CHAIN_ELT(multimedia_link, sour->mm_link, link);
}

void source_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct source *sour = SAFE_CTXT_CAST(source, ctxt);
  if (sour)
    LINK_CHAIN_ELT(note_sub, sour->note, note);
}

void source_add_user_ref(Gom_ctxt ctxt, struct user_ref_number* ref)
{
  struct source *sour = SAFE_CTXT_CAST(source, ctxt);
  if (sour)
    LINK_CHAIN_ELT(user_ref_number, sour->ref, ref);
}

void source_set_record_id(Gom_ctxt ctxt, const char *rin)
{
  struct source *sour = SAFE_CTXT_CAST(source, ctxt);
  if (sour) {
    sour->record_id = strdup(rin);
    if (! sour->record_id) MEMORY_ERROR;
  }
}

void source_set_change_date(Gom_ctxt ctxt, struct change_date* chan)
{
  struct source *sour = SAFE_CTXT_CAST(source, ctxt);
  if (sour)
    sour->change_date = chan;
}

void source_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct source *obj = SAFE_CTXT_CAST(source, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void source_cleanup(struct source* sour)
{
  if (sour) {
    SAFE_FREE(sour->xrefstr);
    DESTROY_CHAIN_ELTS(source_event, sour->data.event, source_event_cleanup);
    SAFE_FREE(sour->data.agency)
    DESTROY_CHAIN_ELTS(note_sub, sour->data.note, note_sub_cleanup);
    SAFE_FREE(sour->author);
    SAFE_FREE(sour->title);
    SAFE_FREE(sour->abbreviation);
    SAFE_FREE(sour->publication);
    SAFE_FREE(sour->text);
    DESTROY_CHAIN_ELTS(note_sub, sour->repository.note, note_sub_cleanup);
    DESTROY_CHAIN_ELTS(source_description, sour->repository.description,
		       source_description_cleanup);
    DESTROY_CHAIN_ELTS(multimedia_link, sour->mm_link,multimedia_link_cleanup);
    DESTROY_CHAIN_ELTS(note_sub, sour->note, note_sub_cleanup);
    DESTROY_CHAIN_ELTS(user_ref_number, sour->ref, user_ref_cleanup);
    SAFE_FREE(sour->record_id);
    change_date_cleanup(sour->change_date);
    DESTROY_CHAIN_ELTS(user_data, sour->extra, user_data_cleanup);
  }
}

void sources_cleanup()
{
  DESTROY_CHAIN_ELTS(source, gom_first_source, source_cleanup);
}

struct source* gom_get_first_source()
{
  return gom_first_source;
}

struct source* make_source_record(const char* xrefstr)
{
  struct source* src = NULL;
  MAKE_CHAIN_ELT(source, gom_first_source, src);
  if (src) {
    src->xrefstr = strdup(xrefstr);
    if (! src->xrefstr) MEMORY_ERROR;
  }
  return src;
}

int write_sources(Gedcom_write_hndl hndl)
{
  int result = 0;
  struct source* obj;

  for (obj = gom_first_source; obj; obj = obj->next) {
    result |= gedcom_write_record_str(hndl, REC_SOUR, 0,
				      obj->xrefstr, NULL);
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

