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
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct source_citation *cit
      = (struct source_citation *)malloc(sizeof(struct source_citation));

    if (! cit)
      MEMORY_ERROR;
    else {
      memset (cit, 0, sizeof(struct source_citation));
      if (GEDCOM_IS_XREF_PTR(parsed_value))
	cit->reference = GEDCOM_XREF_PTR(parsed_value);

      switch (ctxt->ctxt_type) {
	case ELT_SUB_PLAC:
	  ADDFUNC2(place,source_citation)(ctxt, cit); break;
	case ELT_SUB_FAM_EVT:
	case ELT_SUB_FAM_EVT_EVEN:
	case ELT_SUB_INDIV_ATTR:
	case ELT_SUB_INDIV_RESI:
	case ELT_SUB_INDIV_BIRT:
	case ELT_SUB_INDIV_GEN:
	case ELT_SUB_INDIV_ADOP:
	case ELT_SUB_INDIV_EVEN:
	  ADDFUNC2(event,source_citation)(ctxt, cit); break;
	case ELT_SUB_NOTE:
	  ADDFUNC2(note_sub,source_citation)(ctxt, cit); break;
	case ELT_SUB_LSS_SLGS:
	case ELT_SUB_LIO_BAPL:
	case ELT_SUB_LIO_SLGC:
	  ADDFUNC2(lds_event,source_citation)(ctxt, cit); break;
	case REC_FAM:
	  ADDFUNC2(family,source_citation)(ctxt, cit); break;
	case ELT_SUB_PERS_NAME:
	  ADDFUNC2(personal_name,source_citation)(ctxt, cit); break;
	case REC_INDI:
	  ADDFUNC2(individual,source_citation)(ctxt, cit); break;
	case ELT_SUB_ASSO:
	  ADDFUNC2(association,source_citation)(ctxt, cit); break;
	case REC_NOTE:
	  ADDFUNC2(note,source_citation)(ctxt, cit); break;
	default:
	  UNEXPECTED_CONTEXT(ctxt->ctxt_type);
      }
      result = MAKE_GOM_CTXT(elt, source_citation, cit);
    }
  }

  return (Gedcom_ctxt)result;
}

void sub_citation_end(_ELT_END_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)self;

  if (! ctxt)
    NO_CONTEXT;
  else {
    if (GEDCOM_IS_STRING(parsed_value)) {
      struct source_citation *cit = SAFE_CTXT_CAST(source_citation, ctxt);
      if (cit) {
	char *str = GEDCOM_STRING(parsed_value);
	char *newvalue = strdup(str);
	if (! newvalue)
	  MEMORY_ERROR;
	else
	  cit->description = newvalue;
      }
    }
    destroy_gom_ctxt(ctxt);
  }
}

Gedcom_ctxt sub_cit_text_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct source_citation *cit = SAFE_CTXT_CAST(source_citation, ctxt);
    if (cit) {
      struct text *t = NULL;
      MAKE_CHAIN_ELT(text, cit->text, t);
      if (t)
	result = MAKE_GOM_CTXT(elt, text, t);
    }
  }
  
  return (Gedcom_ctxt)result;
}

DEFINE_STRING_CB(source_citation, sub_cit_page_start, page)
DEFINE_STRING_CB(source_citation, sub_cit_even_start, event)
DEFINE_STRING_CB(source_citation, sub_cit_even_role_start, role)
DEFINE_NULL_CB(source_citation, sub_cit_data_start)
DEFINE_DATE_CB(source_citation, sub_cit_data_date_start, date)
DEFINE_STRING_CB(source_citation, sub_cit_quay_start, quality)
DEFINE_STRING_END_CB(text, sub_cit_text_end, text)

DEFINE_ADDFUNC2(source_citation, note_sub, note)
DEFINE_ADDFUNC2(source_citation, multimedia_link, mm_link)
DEFINE_ADDFUNC2(source_citation, user_data, extra)
     
void citation_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_SOUR, sub_citation_start,
			      sub_citation_end);
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
			      sub_cit_text_end);
  gedcom_subscribe_to_element(ELT_SUB_SOUR_QUAY, sub_cit_quay_start,
			      def_elt_end);
}

void CLEANFUNC(text)(struct text* t)
{
  if (t) {
    SAFE_FREE(t->text);
  }
}

void CLEANFUNC(source_citation)(struct source_citation* cit)
{
  if (cit) {
    SAFE_FREE(cit->description);
    SAFE_FREE(cit->page);
    SAFE_FREE(cit->event);
    SAFE_FREE(cit->role);
    SAFE_FREE(cit->date);
    DESTROY_CHAIN_ELTS(text, cit->text);
    SAFE_FREE(cit->quality);
    DESTROY_CHAIN_ELTS(multimedia_link, cit->mm_link);
    DESTROY_CHAIN_ELTS(note_sub, cit->note);
    DESTROY_CHAIN_ELTS(user_data, cit->extra);
  }
}

int write_texts(Gedcom_write_hndl hndl, int parent, struct text* t)
{
  int result = 0;
  struct text* obj;
  
  if (!t) return 1;

  for (obj = t; obj; obj = obj->next) {
    result |= gedcom_write_element_str(hndl, ELT_SUB_SOUR_TEXT, 0, parent,
				       obj->text);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);    
  }
  
  return result;
}

int write_citations(Gedcom_write_hndl hndl, int parent,
		    struct source_citation* cit)
{
  int result = 0;
  struct source_citation* obj;
  
  if (!cit) return 1;

  for (obj = cit; obj; obj = obj->next) {
    if (obj->reference) {
      result |= gedcom_write_element_xref(hndl, ELT_SUB_SOUR, 0, parent,
					  obj->reference);
      if (obj->page)
	result |= gedcom_write_element_str(hndl, ELT_SUB_SOUR_PAGE, 0,
					   ELT_SUB_SOUR, obj->page);
      if (obj->event)
	result |= gedcom_write_element_str(hndl, ELT_SUB_SOUR_EVEN, 0,
					   ELT_SUB_SOUR, obj->event);
      if (obj->role)
	result |= gedcom_write_element_str(hndl, ELT_SUB_SOUR_EVEN_ROLE, 0,
					   ELT_SUB_SOUR_EVEN, obj->role);
      if (obj->date || obj->text)
	result |= gedcom_write_element_str(hndl, ELT_SUB_SOUR_DATA, 0,
					   ELT_SUB_SOUR, NULL);
      if (obj->date)
	result |= gedcom_write_element_date(hndl, ELT_SUB_SOUR_DATA_DATE, 0,
					    ELT_SUB_SOUR_DATA, obj->date);
      if (obj->text)
	result |= write_texts(hndl, ELT_SUB_SOUR_DATA, obj->text);
      if (obj->quality)
	result |= gedcom_write_element_str(hndl, ELT_SUB_SOUR_QUAY, 0,
					   ELT_SUB_SOUR, obj->quality);
      if (obj->mm_link)
	result |= write_multimedia_links(hndl, ELT_SUB_SOUR, obj->mm_link);
    }
    else {
      result |= gedcom_write_element_str(hndl, ELT_SUB_SOUR, 0, parent,
					 obj->description);
      if (obj->text)
	result |= write_texts(hndl, ELT_SUB_SOUR, obj->text);
    }
    if (obj->note)
      result |= write_note_subs(hndl, ELT_SUB_SOUR, obj->note);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);    
  }
  
  return result;
}
