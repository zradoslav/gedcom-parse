/* LDS spouse sealing sub-structure in the gedcom object model.
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
#include "lds_event.h"
#include "family.h"
#include "individual.h"
#include "note_sub.h"
#include "source_citation.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_lds_event_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct lds_event *lds_evt
      = (struct lds_event*)malloc(sizeof(struct lds_event));
    if (! lds_evt)
      MEMORY_ERROR;
    else {
      memset (lds_evt, 0, sizeof(struct lds_event));
      lds_evt->event = parsed_tag;
      lds_evt->event_name = strdup(tag);
      if (! lds_evt->event_name) {
	MEMORY_ERROR;
	free(lds_evt);
      }
      else {
	switch (ctxt->ctxt_type) {
	  case REC_FAM:
	    family_add_lss(ctxt, lds_evt); break;
	  case REC_INDI:
	    individual_add_lio(ctxt, lds_evt); break;
	  default:
	    UNEXPECTED_CONTEXT(ctxt->ctxt_type);
	}
	result = MAKE_GOM_CTXT(elt, lds_event, lds_evt);
      }
    }
  }

  return (Gedcom_ctxt)result;
}

STRING_CB(lds_event, sub_lds_event_stat_start, date_status)
DATE_CB(lds_event, sub_lds_event_date_start, date)
STRING_CB(lds_event, sub_lds_event_temp_start, temple_code)
STRING_CB(lds_event, sub_lds_event_plac_start, place_living_ordinance)
XREF_CB(lds_event, sub_lds_event_famc_start, family, make_family_record)
     
void lds_event_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_LSS_SLGS,
			      sub_lds_event_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LIO_BAPL,
			      sub_lds_event_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LIO_SLGC,
			      sub_lds_event_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LSS_SLGS_STAT,
			      sub_lds_event_stat_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LIO_BAPL_STAT,
			      sub_lds_event_stat_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LSS_SLGS_DATE,
			      sub_lds_event_date_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LIO_BAPL_DATE,
			      sub_lds_event_date_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LSS_SLGS_TEMP,
			      sub_lds_event_temp_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LIO_BAPL_TEMP,
			      sub_lds_event_temp_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LSS_SLGS_PLAC,
			      sub_lds_event_plac_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LIO_BAPL_PLAC,
			      sub_lds_event_plac_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_LIO_SLGC_FAMC,
			      sub_lds_event_famc_start, def_elt_end);
}

void lds_event_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct lds_event *lds = SAFE_CTXT_CAST(lds_event, ctxt);
  if (lds)
    LINK_CHAIN_ELT(note_sub, lds->note, note);    
}

void lds_event_add_citation(Gom_ctxt ctxt, struct source_citation* cit)
{
  struct lds_event *lds = SAFE_CTXT_CAST(lds_event, ctxt);
  if (lds)
    LINK_CHAIN_ELT(source_citation, lds->citation, cit);    
}

void lds_event_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct lds_event *obj = SAFE_CTXT_CAST(lds_event, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void lds_event_cleanup(struct lds_event* lds)
{
  if (lds) {
    SAFE_FREE(lds->event_name);
    SAFE_FREE(lds->date_status);
    SAFE_FREE(lds->date);
    SAFE_FREE(lds->temple_code);
    SAFE_FREE(lds->place_living_ordinance);
    DESTROY_CHAIN_ELTS(source_citation, lds->citation, citation_cleanup);  
    DESTROY_CHAIN_ELTS(note_sub, lds->note, note_sub_cleanup);
    DESTROY_CHAIN_ELTS(user_data, lds->extra, user_data_cleanup);
  }
}

static int get_gedcom_elt(int parsed_tag)
{
  int obj_elt = 0;
  switch (parsed_tag) {
    case TAG_BAPL: case TAG_CONL: case TAG_ENDL:
      obj_elt = ELT_SUB_LIO_BAPL; break;
    case TAG_SLGC:
      obj_elt = ELT_SUB_LIO_SLGC; break;
    default:
      gedcom_warning(_("Internal error: unknown evt tag %d"), parsed_tag);
  }
  return obj_elt;
}
  
int write_lds_events(Gedcom_write_hndl hndl, int parent, struct lds_event *lds)
{
  int result = 0;
  struct lds_event* obj;

  if (!lds) return 1;

  for (obj = lds; obj; obj = obj->next) {
    int obj_elt = get_gedcom_elt(obj->event);
    result |= gedcom_write_element_str(hndl, obj_elt, obj->event,
				       parent, NULL);
    if (obj->date_status)
      result |= gedcom_write_element_str(hndl, ELT_SUB_LIO_BAPL_STAT, 0,
					 parent, obj->date_status);
    if (obj->temple_code)
      result |= gedcom_write_element_str(hndl, ELT_SUB_LIO_BAPL_TEMP, 0,
					 parent, obj->temple_code);
    if (obj->place_living_ordinance)
      result |= gedcom_write_element_str(hndl, ELT_SUB_LIO_BAPL_PLAC, 0,
					 parent, obj->place_living_ordinance);
    if (obj->family)
      result |= gedcom_write_element_xref(hndl, ELT_SUB_LIO_SLGC_FAMC, 0,
					  parent, obj->family);
    if (obj->citation)
      result |= write_citations(hndl, obj_elt, obj->citation);
    if (obj->note)
      result |= write_note_subs(hndl, obj_elt, obj->note);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }

  return result;
}
