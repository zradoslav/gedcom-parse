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

REC_CB(individual, indi_start, make_individual_record)
GET_REC_BY_XREF(individual, XREF_INDI, gom_get_individual_by_xref)
STRING_CB(individual, indi_resn_start, restriction_notice)
STRING_CB(individual, indi_sex_start, sex)
XREF_LIST_CB(individual, indi_subm_start, submitters, make_submitter_record)
XREF_LIST_CB(individual, indi_alia_start, alias, make_individual_record)
XREF_LIST_CB(individual, indi_anci_start, ancestor_interest,
	     make_submitter_record)
XREF_LIST_CB(individual, indi_desi_start, descendant_interest,
	     make_submitter_record)
STRING_CB(individual, indi_rfn_start, record_file_nr)
STRING_CB(individual, indi_afn_start, ancestral_file_nr)

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

void individual_add_event(Gom_ctxt ctxt, struct event* evt)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv)
    LINK_CHAIN_ELT(event, indiv->event, evt);
}

void individual_add_attribute(Gom_ctxt ctxt, struct event* evt)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv)
    LINK_CHAIN_ELT(event, indiv->attribute, evt);
}

void individual_add_name(Gom_ctxt ctxt, struct personal_name* name)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv)
    LINK_CHAIN_ELT(personal_name, indiv->name, name);
}

void individual_add_lio(Gom_ctxt ctxt, struct lds_event* evt)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv)
    LINK_CHAIN_ELT(lds_event, indiv->lds_individual_ordinance, evt);
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

void individual_add_association(Gom_ctxt ctxt, struct association* assoc)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv)
    LINK_CHAIN_ELT(association, indiv->association, assoc);
}

void individual_add_citation(Gom_ctxt ctxt, struct source_citation* cit)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv)
    LINK_CHAIN_ELT(source_citation, indiv->citation, cit);
}

void individual_add_mm_link(Gom_ctxt ctxt, struct multimedia_link* link)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv)
    LINK_CHAIN_ELT(multimedia_link, indiv->mm_link, link);
}

void individual_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv)
    LINK_CHAIN_ELT(note_sub, indiv->note, note);
}

void individual_add_user_ref(Gom_ctxt ctxt, struct user_ref_number* ref)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv)
    LINK_CHAIN_ELT(user_ref_number, indiv->ref, ref);
}

void individual_set_record_id(Gom_ctxt ctxt, char *rin)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv) {
    indiv->record_id = strdup(rin);
    if (! indiv->record_id) MEMORY_ERROR;
  }
}

void individual_set_change_date(Gom_ctxt ctxt, struct change_date* chan)
{
  struct individual *indiv = SAFE_CTXT_CAST(individual, ctxt);
  if (indiv)
    indiv->change_date = chan;
}

void individual_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct individual *obj = SAFE_CTXT_CAST(individual, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void individual_cleanup(struct individual* indiv)
{
  if (indiv) {
    SAFE_FREE(indiv->xrefstr);
    SAFE_FREE(indiv->restriction_notice);
    DESTROY_CHAIN_ELTS(personal_name, indiv->name, name_cleanup);
    SAFE_FREE(indiv->sex);
    DESTROY_CHAIN_ELTS(event, indiv->event, event_cleanup);
    DESTROY_CHAIN_ELTS(event, indiv->attribute, event_cleanup);
    DESTROY_CHAIN_ELTS(lds_event, indiv->lds_individual_ordinance,
		       lds_event_cleanup);
    DESTROY_CHAIN_ELTS(family_link,indiv->child_to_family,family_link_cleanup);
    DESTROY_CHAIN_ELTS(family_link,indiv->spouse_to_family,
		       family_link_cleanup);
    DESTROY_CHAIN_ELTS(xref_list, indiv->submitters, NULL_DESTROY);
    DESTROY_CHAIN_ELTS(association, indiv->association, association_cleanup);  
    DESTROY_CHAIN_ELTS(xref_list, indiv->alias, NULL_DESTROY);
    DESTROY_CHAIN_ELTS(xref_list, indiv->ancestor_interest, NULL_DESTROY);
    DESTROY_CHAIN_ELTS(xref_list, indiv->descendant_interest, NULL_DESTROY);
    DESTROY_CHAIN_ELTS(source_citation, indiv->citation, citation_cleanup);
    DESTROY_CHAIN_ELTS(multimedia_link,indiv->mm_link,multimedia_link_cleanup);
    DESTROY_CHAIN_ELTS(note_sub, indiv->note, note_sub_cleanup);
    SAFE_FREE(indiv->record_file_nr);
    SAFE_FREE(indiv->ancestral_file_nr);
    DESTROY_CHAIN_ELTS(user_ref_number, indiv->ref, user_ref_cleanup);
    SAFE_FREE(indiv->record_id);
    change_date_cleanup(indiv->change_date);
    DESTROY_CHAIN_ELTS(user_data, indiv->extra, user_data_cleanup);
  }
}

void individuals_cleanup()
{
  DESTROY_CHAIN_ELTS(individual, gom_first_individual, individual_cleanup);
}

struct individual* gom_get_first_individual()
{
  return gom_first_individual;
}

struct individual* make_individual_record(char* xrefstr)
{
  struct individual* indiv = NULL;
  MAKE_CHAIN_ELT(individual, gom_first_individual, indiv);
  if (indiv) {
    indiv->xrefstr = strdup(xrefstr);
    if (! indiv->xrefstr) MEMORY_ERROR;
  }
  return indiv;
}
