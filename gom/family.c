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

DEFINE_MAKEFUNC(family, gom_first_family)
DEFINE_DESTROYFUNC(family, gom_first_family)
DEFINE_ADDFUNC(family, XREF_FAM)
DEFINE_DELETEFUNC(family)
DEFINE_GETXREFFUNC(family, XREF_FAM)
     
DEFINE_REC_CB(family, fam_start)
DEFINE_XREF_CB(family, fam_husb_start, husband, individual)
DEFINE_XREF_CB(family, fam_wife_start, wife, individual)
DEFINE_STRING_CB(family, fam_nchi_start, nr_of_children)
DEFINE_XREF_LIST_CB(family, fam_chil_start, children, individual)
DEFINE_XREF_LIST_CB(family, fam_subm_start, submitters, submitter)

DEFINE_ADDFUNC2(family, event, event)
DEFINE_ADDFUNC2(family, lds_event, lds_spouse_sealing)
DEFINE_ADDFUNC2(family, source_citation, citation)
DEFINE_ADDFUNC2(family, multimedia_link, mm_link)
DEFINE_ADDFUNC2(family, note_sub, note)
DEFINE_ADDFUNC2(family, user_ref_number, ref)
DEFINE_ADDFUNC2(family, user_data, extra)
DEFINE_ADDFUNC2_NOLIST(family, change_date, change_date)
DEFINE_ADDFUNC2_STR(family, record_id)

void family_subscribe()
{
  gedcom_subscribe_to_record(REC_FAM, fam_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_FAM_HUSB, fam_husb_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_FAM_WIFE, fam_wife_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_FAM_CHIL, fam_chil_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_FAM_NCHI, fam_nchi_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_FAM_SUBM, fam_subm_start, def_elt_end);
}

void CLEANFUNC(family)(struct family* fam)
{
  if (fam) {
    SAFE_FREE(fam->xrefstr);
    DESTROY_CHAIN_ELTS(event, fam->event); 
    DESTROY_CHAIN_ELTS(xref_list, fam->children);
    SAFE_FREE(fam->nr_of_children);
    DESTROY_CHAIN_ELTS(xref_list, fam->submitters);
    DESTROY_CHAIN_ELTS(lds_event, fam->lds_spouse_sealing);
    DESTROY_CHAIN_ELTS(source_citation, fam->citation);
    DESTROY_CHAIN_ELTS(multimedia_link, fam->mm_link);
    DESTROY_CHAIN_ELTS(note_sub, fam->note);
    DESTROY_CHAIN_ELTS(user_ref_number, fam->ref);
    SAFE_FREE(fam->record_id);
    CLEANFUNC(change_date)(fam->change_date);
    DESTROY_CHAIN_ELTS(user_data, fam->extra);
  }
}

void families_cleanup()
{
  DESTROY_CHAIN_ELTS(family, gom_first_family);
}

struct family* gom_get_first_family()
{
  return gom_first_family;
}

int write_families(Gedcom_write_hndl hndl)
{
  int result = 0;
  struct family* obj;

  for (obj = gom_first_family; obj; obj = obj->next) {
    result |= gedcom_write_record_str(hndl, REC_FAM, obj->xrefstr, NULL);
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

