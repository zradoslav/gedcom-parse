/* Source citation sub-structure in the gedcom object model.
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
#include "source_citation.h"
#include "place.h"
#include "event.h"
#include "note_sub.h"
#include "multimedia_link.h"
#include "lds_event.h"
#include "family.h"
#include "personal_name.h"
#include "individual.h"
#include "association.h"
#include "note.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_citation_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  struct source_citation *cit = NULL;

  if (ctxt) {
    cit = (struct source_citation *)malloc(sizeof(struct source_citation));
    memset (cit, 0, sizeof(struct source_citation));
    if (GEDCOM_IS_STRING(parsed_value))
      cit->description = strdup(GEDCOM_STRING(parsed_value));
    else if (GEDCOM_IS_XREF_PTR(parsed_value))
      cit->reference = GEDCOM_XREF_PTR(parsed_value);

    switch (ctxt->ctxt_type) {
      case ELT_SUB_PLAC:
	place_add_citation(ctxt, cit); break;
      case ELT_SUB_FAM_EVT:
      case ELT_SUB_FAM_EVT_EVEN:
      case ELT_SUB_INDIV_ATTR:
      case ELT_SUB_INDIV_RESI:
      case ELT_SUB_INDIV_BIRT:
      case ELT_SUB_INDIV_GEN:
      case ELT_SUB_INDIV_ADOP:
      case ELT_SUB_INDIV_EVEN:
	event_add_citation(ctxt, cit); break;
      case ELT_SUB_NOTE:
	note_sub_add_citation(ctxt, cit); break;
      case ELT_SUB_LSS_SLGS:
      case ELT_SUB_LIO_BAPL:
      case ELT_SUB_LIO_SLGC:
	lds_event_add_citation(ctxt, cit); break;
      case REC_FAM:
	family_add_citation(ctxt, cit); break;
      case ELT_SUB_PERS_NAME:
	name_add_citation(ctxt, cit); break;
      case REC_INDI:
	individual_add_citation(ctxt, cit); break;
      case ELT_SUB_ASSO:
	association_add_citation(ctxt, cit); break;
      case REC_NOTE:
	note_add_citation(ctxt, cit); break;
      default:
	UNEXPECTED_CONTEXT(ctxt->ctxt_type);
    }
  }

  return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, source_citation, cit);
}

Gedcom_ctxt sub_cit_text_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  struct source_citation *cit = SAFE_CTXT_CAST(source_citation, ctxt);
  struct text *t;
  MAKE_CHAIN_ELT(text, cit->text, t);
  t->text = strdup(GEDCOM_STRING(parsed_value));
  return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, text, t);
}

STRING_CB(source_citation, sub_cit_page_start, page)
STRING_CB(source_citation, sub_cit_even_start, event)
STRING_CB(source_citation, sub_cit_even_role_start, role)
NULL_CB(source_citation, sub_cit_data_start)
DATE_CB(source_citation, sub_cit_data_date_start, date)
STRING_CB(source_citation, sub_cit_quay_start, quality)
     
void citation_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_SOUR, sub_citation_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_SOUR_PAGE, sub_cit_page_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_SOUR_EVEN, sub_cit_even_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_SOUR_EVEN_ROLE, sub_cit_even_role_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_SOUR_DATA, sub_cit_data_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_SOUR_DATA_DATE, sub_cit_data_date_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_SOUR_TEXT, sub_cit_text_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_SOUR_QUAY, sub_cit_quay_start,
			      def_elt_end);
}

void citation_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct source_citation *cit = SAFE_CTXT_CAST(source_citation, ctxt);
  LINK_CHAIN_ELT(note_sub, cit->note, note)    
}

void citation_add_mm_link(Gom_ctxt ctxt, struct multimedia_link* mm)
{
  struct source_citation *cit = SAFE_CTXT_CAST(source_citation, ctxt);
  LINK_CHAIN_ELT(multimedia_link, cit->mm_link, mm)    
}

void citation_add_to_desc(NL_TYPE type, Gom_ctxt ctxt, char* str)
{
  struct source_citation *cit = SAFE_CTXT_CAST(source_citation, ctxt);
  cit->description = concat_strings (type, cit->description, str);
}

void citation_add_to_text(NL_TYPE type, Gom_ctxt ctxt, char* str)
{
  struct text *t = SAFE_CTXT_CAST(text, ctxt);
  t->text = concat_strings (type, t->text, str);
}

void citation_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct source_citation *obj = SAFE_CTXT_CAST(source_citation, ctxt);
  LINK_CHAIN_ELT(user_data, obj->extra, data)
}

void text_cleanup(struct text* t)
{
  if (t) {
    SAFE_FREE(t->text);
  }
}

void citation_cleanup(struct source_citation* cit)
{
  if (cit) {
    SAFE_FREE(cit->description);
    SAFE_FREE(cit->page);
    SAFE_FREE(cit->event);
    SAFE_FREE(cit->role);
    SAFE_FREE(cit->date);
    DESTROY_CHAIN_ELTS(text, cit->text, text_cleanup)
    SAFE_FREE(cit->quality);
    DESTROY_CHAIN_ELTS(multimedia_link, cit->mm_link, multimedia_link_cleanup)
    DESTROY_CHAIN_ELTS(note_sub, cit->note, note_sub_cleanup)
    DESTROY_CHAIN_ELTS(user_data, cit->extra, user_data_cleanup)
  }
}
