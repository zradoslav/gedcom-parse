/* Family object in the gedcom object model.
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
#include "family.h"
#include "individual.h"
#include "submitter.h"
#include "event.h"
#include "lds_event.h"
#include "source_citation.h"
#include "multimedia_link.h"
#include "note_sub.h"
#include "user_ref.h"
#include "change_date.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

struct family* gom_first_family = NULL;

REC_CB(family, fam_start, make_family_record)
GET_REC_BY_XREF(family, XREF_FAM, gom_get_family_by_xref)
XREF_CB(family, fam_husb_start, husband, make_individual_record)
XREF_CB(family, fam_wife_start, wife, make_individual_record)
STRING_CB(family, fam_nchi_start, nr_of_children)
XREF_LIST_CB(family, fam_chil_start, children, make_individual_record)
XREF_LIST_CB(family, fam_subm_start, submitters, make_submitter_record)

void family_subscribe()
{
  gedcom_subscribe_to_record(REC_FAM, fam_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_FAM_HUSB, fam_husb_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_FAM_WIFE, fam_wife_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_FAM_CHIL, fam_chil_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_FAM_NCHI, fam_nchi_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_FAM_SUBM, fam_subm_start, def_elt_end);
}

void family_add_event(Gom_ctxt ctxt, struct event* evt)
{
  struct family *fam = SAFE_CTXT_CAST(family, ctxt);
  if (fam)
    LINK_CHAIN_ELT(event, fam->event, evt);
}

void family_add_lss(Gom_ctxt ctxt, struct lds_event* lss)
{
  struct family *fam = SAFE_CTXT_CAST(family, ctxt);
  if (fam)
    LINK_CHAIN_ELT(lds_event, fam->lds_spouse_sealing, lss);
}

void family_add_citation(Gom_ctxt ctxt, struct source_citation* cit)
{
  struct family *fam = SAFE_CTXT_CAST(family, ctxt);
  if (fam)
    LINK_CHAIN_ELT(source_citation, fam->citation, cit);
}

void family_add_mm_link(Gom_ctxt ctxt, struct multimedia_link* link)
{
  struct family *fam = SAFE_CTXT_CAST(family, ctxt);
  if (fam)
    LINK_CHAIN_ELT(multimedia_link, fam->mm_link, link);
}

void family_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct family *fam = SAFE_CTXT_CAST(family, ctxt);
  if (fam)
    LINK_CHAIN_ELT(note_sub, fam->note, note);
}

void family_add_user_ref(Gom_ctxt ctxt, struct user_ref_number* ref)
{
  struct family *fam = SAFE_CTXT_CAST(family, ctxt);
  if (fam)
    LINK_CHAIN_ELT(user_ref_number, fam->ref, ref);
}

void family_set_record_id(Gom_ctxt ctxt, const char *rin)
{
  struct family *fam = SAFE_CTXT_CAST(family, ctxt);
  if (fam) {
    fam->record_id = strdup(rin);
    if (! fam->record_id) MEMORY_ERROR;
  }
}

void family_set_change_date(Gom_ctxt ctxt, struct change_date* chan)
{
  struct family *fam = SAFE_CTXT_CAST(family, ctxt);
  if (fam)
    fam->change_date = chan;
}

void family_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct family *obj = SAFE_CTXT_CAST(family, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void family_cleanup(struct family* fam)
{
  if (fam) {
    SAFE_FREE(fam->xrefstr);
    DESTROY_CHAIN_ELTS(event, fam->event, event_cleanup); 
    DESTROY_CHAIN_ELTS(xref_list, fam->children, NULL_DESTROY);
    SAFE_FREE(fam->nr_of_children);
    DESTROY_CHAIN_ELTS(xref_list, fam->submitters, NULL_DESTROY);
    DESTROY_CHAIN_ELTS(lds_event, fam->lds_spouse_sealing, lds_event_cleanup);
    DESTROY_CHAIN_ELTS(source_citation, fam->citation, citation_cleanup);
    DESTROY_CHAIN_ELTS(multimedia_link, fam->mm_link, multimedia_link_cleanup);
    DESTROY_CHAIN_ELTS(note_sub, fam->note, note_sub_cleanup);
    DESTROY_CHAIN_ELTS(user_ref_number, fam->ref, user_ref_cleanup);
    SAFE_FREE(fam->record_id);
    change_date_cleanup(fam->change_date);
    DESTROY_CHAIN_ELTS(user_data, fam->extra, user_data_cleanup);
  }
}

void families_cleanup()
{
  DESTROY_CHAIN_ELTS(family, gom_first_family, family_cleanup);
}

struct family* gom_get_first_family()
{
  return gom_first_family;
}

struct family* make_family_record(const char* xrefstr)
{
  struct family* fam = NULL;
  MAKE_CHAIN_ELT(family, gom_first_family, fam);
  if (fam) {
    fam->xrefstr = strdup(xrefstr);
    if (! fam->xrefstr) MEMORY_ERROR;
  }
  return fam;
}

int write_families(Gedcom_write_hndl hndl)
{
  int result = 0;
  struct family* obj;

  for (obj = gom_first_family; obj; obj = obj->next) {
    result |= gedcom_write_record_str(hndl, REC_FAM, 0,
				      obj->xrefstr, NULL);
    if (obj->event)
      result |= write_events(hndl, REC_FAM, EVT_TYPE_FAMILY, obj->event);
    if (obj->husband)
      result |= gedcom_write_element_xref(hndl, ELT_FAM_HUSB, 0,
					  REC_FAM, obj->husband);
    if (obj->wife)
      result |= gedcom_write_element_xref(hndl, ELT_FAM_WIFE, 0,
					  REC_FAM, obj->wife);
    result |= gom_write_xref_list(hndl, ELT_FAM_CHIL, 0,
				  REC_FAM, obj->children);
    if (obj->nr_of_children)
      result |= gedcom_write_element_str(hndl, ELT_FAM_NCHI, 0,
					 REC_FAM, obj->nr_of_children);
    result |= gom_write_xref_list(hndl, ELT_FAM_SUBM, 0,
				  REC_FAM, obj->submitters);
    if (obj->lds_spouse_sealing)
      result |= write_lds_events(hndl, REC_FAM, obj->lds_spouse_sealing);
    if (obj->citation)
      result |= write_citations(hndl, REC_FAM, obj->citation);
    if (obj->mm_link)
      result |= write_multimedia_links(hndl, REC_FAM, obj->mm_link);
    if (obj->note)
      result |= write_note_subs(hndl, REC_FAM, obj->note);
    if (obj->ref)
      result |= write_user_refs(hndl, REC_FAM, obj->ref);
    if (obj->record_id)
      result |= gedcom_write_element_str(hndl, ELT_SUB_IDENT_RIN, 0,
					 REC_FAM, obj->record_id);
    if (obj->change_date)
      result |= write_change_date(hndl, REC_FAM, obj->change_date);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }
  
  return result;
}

