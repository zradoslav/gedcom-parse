/* Individual object in the gedcom object model.
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
#include "individual.h"
#include "event.h"
#include "personal_name.h"
#include "lds_event.h"
#include "family_link.h"
#include "association.h"
#include "submitter.h"
#include "source_citation.h"
#include "multimedia_link.h"
#include "note_sub.h"
#include "user_ref.h"
#include "change_date.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

struct individual* gom_first_individual = NULL;

DEFINE_MAKEFUNC(individual, gom_first_individual)
DEFINE_DESTROYFUNC(individual, gom_first_individual)
DEFINE_ADDFUNC(individual, XREF_INDI)
DEFINE_DELETEFUNC(individual)
DEFINE_GETXREFFUNC(individual, XREF_INDI)
DEFINE_MAKELINKFUNC(individual, XREF_INDI)
     
DEFINE_REC_CB(individual, indi_start)
DEFINE_STRING_CB(individual, indi_resn_start, restriction_notice)
DEFINE_STRING_CB(individual, indi_sex_start, sex)
DEFINE_XREF_LIST_CB(individual, indi_subm_start, submitters, submitter)
DEFINE_XREF_LIST_CB(individual, indi_alia_start, alias, individual)
DEFINE_XREF_LIST_CB(individual, indi_anci_start, ancestor_interest, submitter)
DEFINE_XREF_LIST_CB(individual, indi_desi_start, descendant_interest,submitter)
DEFINE_STRING_CB(individual, indi_rfn_start, record_file_nr)
DEFINE_STRING_CB(individual, indi_afn_start, ancestral_file_nr)

DEFINE_ADDFUNC2(individual, event, event)
DEFINE_ADDFUNC2_TOVAR(individual, event, attribute)
DEFINE_ADDFUNC2(individual, personal_name, name)
DEFINE_ADDFUNC2(individual, lds_event, lds_individual_ordinance)
DEFINE_ADDFUNC2(individual, association, association)
DEFINE_ADDFUNC2(individual, source_citation, citation)
DEFINE_ADDFUNC2(individual, multimedia_link, mm_link)
DEFINE_ADDFUNC2(individual, note_sub, note)
DEFINE_ADDFUNC2(individual, user_ref_number, ref)
DEFINE_ADDFUNC2(individual, user_data, extra)
DEFINE_ADDFUNC2_NOLIST(individual, change_date, change_date)
DEFINE_ADDFUNC2_STR(individual, record_id)

void individual_subscribe()
{
  gedcom_subscribe_to_record(REC_INDI, indi_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_INDI_RESN, indi_resn_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_INDI_SEX, indi_sex_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_INDI_SUBM, indi_subm_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_INDI_ALIA, indi_alia_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_INDI_ANCI, indi_anci_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_INDI_DESI, indi_desi_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_INDI_RFN, indi_rfn_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_INDI_AFN, indi_afn_start, def_elt_end);
}

void individual_add_family_link(Gom_ctxt ctxt, int ctxt_type,
				struct family_link* link)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv) {
    switch (ctxt_type) {
      case ELT_SUB_FAMC:
	LINK_CHAIN_ELT(family_link, indiv->child_to_family, link);
	break;
      case ELT_SUB_FAMS:
	LINK_CHAIN_ELT(family_link, indiv->spouse_to_family, link);
	break;
      default:
	UNEXPECTED_CONTEXT(ctxt_type);
    }
  }
}

void CLEANFUNC(individual)(struct individual* indiv)
{
  if (indiv) {
    SAFE_FREE(indiv->xrefstr);
    SAFE_FREE(indiv->restriction_notice);
    DESTROY_CHAIN_ELTS(personal_name, indiv->name);
    SAFE_FREE(indiv->sex);
    DESTROY_CHAIN_ELTS(event, indiv->event);
    DESTROY_CHAIN_ELTS(event, indiv->attribute);
    DESTROY_CHAIN_ELTS(lds_event, indiv->lds_individual_ordinance);
    DESTROY_CHAIN_ELTS(family_link, indiv->child_to_family);
    DESTROY_CHAIN_ELTS(family_link, indiv->spouse_to_family);
    DESTROY_CHAIN_ELTS(xref_list, indiv->submitters);
    DESTROY_CHAIN_ELTS(association, indiv->association);  
    DESTROY_CHAIN_ELTS(xref_list, indiv->alias);
    DESTROY_CHAIN_ELTS(xref_list, indiv->ancestor_interest);
    DESTROY_CHAIN_ELTS(xref_list, indiv->descendant_interest);
    DESTROY_CHAIN_ELTS(source_citation, indiv->citation);
    DESTROY_CHAIN_ELTS(multimedia_link, indiv->mm_link);
    DESTROY_CHAIN_ELTS(note_sub, indiv->note);
    SAFE_FREE(indiv->record_file_nr);
    SAFE_FREE(indiv->ancestral_file_nr);
    DESTROY_CHAIN_ELTS(user_ref_number, indiv->ref);
    SAFE_FREE(indiv->record_id);
    CLEANFUNC(change_date)(indiv->change_date);
    DESTROY_CHAIN_ELTS(user_data, indiv->extra);
  }
}

void individuals_cleanup()
{
  DESTROY_CHAIN_ELTS(individual, gom_first_individual);
}

struct individual* gom_get_first_individual()
{
  return gom_first_individual;
}

int write_individuals(Gedcom_write_hndl hndl)
{
  int result = 0;
  struct individual* obj;

  for (obj = gom_first_individual; obj; obj = obj->next) {
    result |= gedcom_write_record_str(hndl, REC_INDI, obj->xrefstr, NULL);
    if (obj->restriction_notice)
      result |= gedcom_write_element_str(hndl, ELT_INDI_RESN, 0,
					 REC_INDI, obj->restriction_notice);
    if (obj->name)
      result |= write_names(hndl, REC_INDI, obj->name);
    if (obj->sex)
      result |= gedcom_write_element_str(hndl, ELT_INDI_SEX, 0,
					 REC_INDI, obj->sex);
    if (obj->event)
      result |= write_events(hndl, REC_INDI, EVT_TYPE_INDIV_EVT, obj->event);
    if (obj->attribute)
      result |= write_events(hndl, REC_INDI, EVT_TYPE_INDIV_ATTR,
			     obj->attribute);
    if (obj->lds_individual_ordinance)
      result |= write_lds_events(hndl, REC_INDI,
				 obj->lds_individual_ordinance);
    if (obj->child_to_family)
      result |= write_family_links(hndl, REC_INDI, LINK_TYPE_CHILD,
				   obj->child_to_family);
    if (obj->spouse_to_family)
      result |= write_family_links(hndl, REC_INDI, LINK_TYPE_SPOUSE,
				   obj->spouse_to_family);
    result |= gom_write_xref_list(hndl, ELT_INDI_SUBM, 0,
				  REC_INDI, obj->submitters);
    if (obj->association)
      result |= write_associations(hndl, REC_INDI, obj->association);
    result |= gom_write_xref_list(hndl, ELT_INDI_ALIA, 0,
				  REC_INDI, obj->alias);
    result |= gom_write_xref_list(hndl, ELT_INDI_ANCI, 0,
				  REC_INDI, obj->ancestor_interest);
    result |= gom_write_xref_list(hndl, ELT_INDI_DESI, 0,
				  REC_INDI, obj->descendant_interest);
    if (obj->citation)
      result |= write_citations(hndl, REC_INDI, obj->citation);
    if (obj->mm_link)
      result |= write_multimedia_links(hndl, REC_INDI, obj->mm_link);
    if (obj->note)
      result |= write_note_subs(hndl, REC_INDI, obj->note);
    if (obj->record_file_nr)
      result |= gedcom_write_element_str(hndl, ELT_INDI_RFN, 0,
					 REC_INDI, obj->record_file_nr);
    if (obj->ancestral_file_nr)
      result |= gedcom_write_element_str(hndl, ELT_INDI_AFN, 0,
					 REC_INDI, obj->ancestral_file_nr);
    if (obj->ref)
      result |= write_user_refs(hndl, REC_INDI, obj->ref);
    if (obj->record_id)
      result |= gedcom_write_element_str(hndl, ELT_SUB_IDENT_RIN, 0,
					 REC_INDI, obj->record_id);
    if (obj->change_date)
      result |= write_change_date(hndl, REC_INDI, obj->change_date);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }
  
  return result;
}

