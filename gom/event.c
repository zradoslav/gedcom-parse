/* Event sub-structure in the gedcom object model.
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
#include "event.h"
#include "place.h"
#include "address.h"
#include "source_citation.h"
#include "multimedia_link.h"
#include "note_sub.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_evt_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct event *evt = (struct event *)malloc(sizeof(struct event));
    if (! evt)
      MEMORY_ERROR;
    else {
      memset (evt, 0, sizeof(struct event));
      evt->event = parsed_tag;
      evt->event_name = strdup(tag);
      if (! evt->event_name) {
	MEMORY_ERROR;
	free(evt);
      }
      else {
	int err = 0;
	if (GEDCOM_IS_STRING(parsed_value)) {
	  evt->val = strdup(GEDCOM_STRING(parsed_value));
	  if (! evt->val) {
	    MEMORY_ERROR;
	    free(evt->event_name);
	    free(evt);
	    err = 1;
	  }
	}

	if (! err) {
	  switch (ctxt->ctxt_type) {
	    case REC_FAM:
	      family_add_event(ctxt, evt); break;
	    case REC_INDI:
	      individual_add_event(ctxt, evt); break;
	    default:
	      UNEXPECTED_CONTEXT(ctxt->ctxt_type);
	  }
	  result = MAKE_GOM_CTXT(elt, event, evt);
	}
      }
    }
  }

  return (Gedcom_ctxt)result;
}

Gedcom_ctxt sub_attr_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct event *evt = (struct event *)malloc(sizeof(struct event));
    if (! evt)
      MEMORY_ERROR;
    else {
      memset (evt, 0, sizeof(struct event));
      evt->event = parsed_tag;
      evt->event_name = strdup(tag);
      if (! evt->event_name) {
	MEMORY_ERROR;
	free(evt);
      }
      else {
	int err = 0;
	if (GEDCOM_IS_STRING(parsed_value)) {
	  evt->val = strdup(GEDCOM_STRING(parsed_value));
	  if (! evt->val) {
	    MEMORY_ERROR;
	    free(evt->event_name);
	    free(evt);
	    err = 1;
	  }
	}

	if (! err) {
	  switch (ctxt->ctxt_type) {
	    case REC_INDI:
	      individual_add_attribute(ctxt, evt); break;
	    default:
	      UNEXPECTED_CONTEXT(ctxt->ctxt_type);
	  }
	  result = MAKE_GOM_CTXT(elt, event, evt);
	}
      }
    }
  }
  return (Gedcom_ctxt)result;
}

STRING_CB(event, sub_evt_type_start, type)
DATE_CB(event, sub_evt_date_start, date)
AGE_CB(event, sub_evt_age_start, age)
STRING_CB(event, sub_evt_agnc_start, agency)
STRING_CB(event, sub_evt_caus_start, cause)
NULL_CB(event, sub_fam_evt_husb_wife_start)
XREF_CB(event, sub_evt_famc_start, family, make_family_record)
STRING_CB(event, sub_evt_famc_adop_start, adoption_parent)
     
Gedcom_ctxt sub_fam_evt_age_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;
  
  if (! ctxt)
    NO_CONTEXT;
  else {
    struct event *evt = SAFE_CTXT_CAST(event, ctxt);
    if (evt) {
      int err = 0;
      struct age_value age = GEDCOM_AGE(parsed_value);
      switch (ctxt->ctxt_type) {
	case ELT_SUB_FAM_EVT_HUSB:
	  evt->husband_age = dup_age(age);
	  if (! evt->husband_age) {
	    MEMORY_ERROR;
	    err = 1;
	  }
	  break;
	case ELT_SUB_FAM_EVT_WIFE:
	  evt->wife_age = dup_age(age);
	  if (! evt->wife_age) {
	    MEMORY_ERROR;
	    err = 1;
	  }
	  break;
	default:
	  UNEXPECTED_CONTEXT(ctxt->ctxt_type);
      }
      if (! err)
	result = MAKE_GOM_CTXT(elt, event, evt);
    }
  }
  return (Gedcom_ctxt)result;
}

void event_add_place(Gom_ctxt ctxt, struct place* place)
{
  struct event *evt = SAFE_CTXT_CAST(event, ctxt);
  if (evt)
    evt->place = place;
}

void event_add_address(Gom_ctxt ctxt, struct address* address)
{
  struct event *evt = SAFE_CTXT_CAST(event, ctxt);
  if (evt)
    evt->address = address;
}

void event_add_phone(Gom_ctxt ctxt, char *phone)
{
  struct event *evt = SAFE_CTXT_CAST(event, ctxt);
  if (evt) {
    int i = 0;
    while (i<2 && evt->phone[i]) i++;
    if (! evt->phone[i]) {
      evt->phone[i] = strdup(phone);
      if (! evt->phone[i]) MEMORY_ERROR;
    }
  }
}

void event_add_citation(Gom_ctxt ctxt, struct source_citation* cit)
{
  struct event *evt = SAFE_CTXT_CAST(event, ctxt);
  if (evt)
    LINK_CHAIN_ELT(source_citation, evt->citation, cit);  
}

void event_add_mm_link(Gom_ctxt ctxt, struct multimedia_link* mm)
{
  struct event *evt = SAFE_CTXT_CAST(event, ctxt);
  if (evt)
    LINK_CHAIN_ELT(multimedia_link, evt->mm_link, mm);
}

void event_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct event *evt = SAFE_CTXT_CAST(event, ctxt);
  if (evt)
    LINK_CHAIN_ELT(note_sub, evt->note, note);
}

void event_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct event *obj = SAFE_CTXT_CAST(event, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void event_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_FAM_EVT, sub_evt_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_FAM_EVT_EVEN,
			      sub_evt_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_INDIV_ATTR,
			      sub_attr_start, def_elt_end);  
  gedcom_subscribe_to_element(ELT_SUB_INDIV_RESI,
			      sub_attr_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_INDIV_BIRT,
			      sub_evt_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_INDIV_BIRT_FAMC,
			      sub_evt_famc_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_INDIV_GEN,
			      sub_evt_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_INDIV_ADOP,
			      sub_evt_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_INDIV_ADOP_FAMC,
			      sub_evt_famc_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_INDIV_ADOP_FAMC_ADOP,
			      sub_evt_famc_adop_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_INDIV_EVEN,
			      sub_evt_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_FAM_EVT_HUSB,
			      sub_fam_evt_husb_wife_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_FAM_EVT_WIFE,
			      sub_fam_evt_husb_wife_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_FAM_EVT_AGE,
			      sub_fam_evt_age_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_EVT_TYPE,
			      sub_evt_type_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_EVT_DATE,
			      sub_evt_date_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_EVT_AGE,
			      sub_evt_age_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_EVT_AGNC,
			      sub_evt_agnc_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_EVT_CAUS,
			      sub_evt_caus_start, def_elt_end);
}

void event_cleanup(struct event* evt)
{
  if (evt) {
    SAFE_FREE(evt->event_name);
    SAFE_FREE(evt->val);
    SAFE_FREE(evt->type);
    SAFE_FREE(evt->date);
    place_cleanup(evt->place);
    address_cleanup(evt->address);
    SAFE_FREE(evt->phone[0]);
    SAFE_FREE(evt->phone[1]);
    SAFE_FREE(evt->phone[2]);
    SAFE_FREE(evt->age);
    SAFE_FREE(evt->agency);
    SAFE_FREE(evt->cause);
    DESTROY_CHAIN_ELTS(source_citation, evt->citation, citation_cleanup);
    DESTROY_CHAIN_ELTS(multimedia_link, evt->mm_link, multimedia_link_cleanup);
    DESTROY_CHAIN_ELTS(note_sub, evt->note, note_sub_cleanup);
    SAFE_FREE(evt->husband_age);
    SAFE_FREE(evt->wife_age);
    SAFE_FREE(evt->adoption_parent);
    DESTROY_CHAIN_ELTS(user_data, evt->extra, user_data_cleanup);
  }
}

static int get_gedcom_elt(EventType evt_type, int parsed_tag)
{
  int obj_elt = 0;
  switch (evt_type) {
    case EVT_TYPE_FAMILY:
      obj_elt = (parsed_tag == TAG_EVEN ? ELT_SUB_FAM_EVT_EVEN :
		 ELT_SUB_FAM_EVT);
      break;
    case EVT_TYPE_INDIV_ATTR:
      obj_elt = (parsed_tag == TAG_RESI ? ELT_SUB_INDIV_RESI :
		 ELT_SUB_INDIV_ATTR);
      break;
    case EVT_TYPE_INDIV_EVT:
      switch (parsed_tag) {
	case TAG_BIRT: case TAG_CHR:
	  obj_elt = ELT_SUB_INDIV_BIRT; break;
	case TAG_DEAT: case TAG_BURI: case TAG_CREM: case TAG_BAPM:
	case TAG_BARM: case TAG_BASM: case TAG_BLES: case TAG_CHRA:
	case TAG_CONF: case TAG_FCOM: case TAG_ORDN: case TAG_NATU:
	case TAG_EMIG: case TAG_IMMI: case TAG_CENS: case TAG_PROB:
	case TAG_WILL: case TAG_GRAD: case TAG_RETI:
	  obj_elt = ELT_SUB_INDIV_GEN; break;
	case TAG_ADOP:
	  obj_elt = ELT_SUB_INDIV_ADOP; break;
	case TAG_EVEN:
	  obj_elt = ELT_SUB_INDIV_EVEN; break;
	default:
	  gedcom_warning(_("Internal error: unknown evt tag %d"), parsed_tag);
      }
      break;
    default:
      gedcom_warning(_("Internal error: unknown evt type %d"), evt_type);
  }
  return obj_elt;
}

int get_gedcom_fam_elt(int elt)
{
  int fam_obj_elt = 0;
  switch (elt) {
    case ELT_SUB_INDIV_BIRT:
      fam_obj_elt = ELT_SUB_INDIV_BIRT_FAMC;
      break;
    case ELT_SUB_INDIV_ADOP:
      fam_obj_elt = ELT_SUB_INDIV_ADOP_FAMC;
      break;
    default:
      gedcom_warning(_("Internal error: wrong parent for evt->family"));
  }
  return fam_obj_elt;
}

int write_events(Gedcom_write_hndl hndl, int parent, EventType evt_type,
		 struct event* evt)
{
  int result = 0;
  int i;
  struct event* obj;

  if (!evt) return 1;

  for (obj = evt; obj; obj = obj->next) {
    int obj_elt = get_gedcom_elt(evt_type, obj->event);
    result |= gedcom_write_element_str(hndl, obj_elt, obj->event,
				       parent, obj->val);
    if (obj->type)
      result |= gedcom_write_element_str(hndl, ELT_SUB_EVT_TYPE, 0,
					 obj_elt, obj->type);
    if (obj->place)
      result |= write_place(hndl, obj_elt, obj->place);
    if (obj->address)
      result |= write_address(hndl, obj_elt, obj->address);
    for (i = 0; i < 3 && obj->phone[i]; i++)
      result |= gedcom_write_element_str(hndl, ELT_SUB_PHON, 0, obj_elt,
					 obj->phone[i]);
    if (obj->agency)
      result |= gedcom_write_element_str(hndl, ELT_SUB_EVT_AGNC, 0,
					 obj_elt, obj->agency);
    if (obj->cause)
      result |= gedcom_write_element_str(hndl, ELT_SUB_EVT_CAUS, 0,
					 obj_elt, obj->cause);
    if (obj->citation)
      result |= write_citations(hndl, obj_elt, obj->citation);
    if (obj->mm_link)
      result |= write_multimedia_links(hndl, obj_elt, obj->mm_link);
    if (obj->note)
      result |= write_note_subs(hndl, obj_elt, obj->note);
    if (obj->family) {
      int fam_obj_elt = get_gedcom_fam_elt(obj_elt);
      result |= gedcom_write_element_xref(hndl, fam_obj_elt, 0,
					  obj_elt, obj->family);
      if (obj->adoption_parent) {
	result |= gedcom_write_element_str(hndl, ELT_SUB_INDIV_ADOP_FAMC_ADOP,
					   0, fam_obj_elt,
					   obj->adoption_parent);
      }
    }
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }
  
  return result;
}
  
