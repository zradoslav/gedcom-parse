/* User record object in the gedcom object model.
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
#include "header.h"
#include "submission.h"
#include "submitter.h"
#include "family.h"
#include "individual.h"
#include "multimedia.h"
#include "note.h"
#include "repository.h"
#include "source.h"
#include "address.h"
#include "event.h"
#include "place.h"
#include "source_citation.h"
#include "note_sub.h"
#include "multimedia_link.h"
#include "lds_event.h"
#include "user_ref.h"
#include "change_date.h"
#include "personal_name.h"
#include "family_link.h"
#include "association.h"
#include "source_event.h"
#include "source_description.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

struct user_rec* gom_first_user_rec = NULL;

Gedcom_ctxt user_rec_start(_REC_PARAMS_)
{
  struct user_rec* user = NULL;
  struct xref_value* xr = NULL;
  Gom_ctxt result  = NULL;
  Gedcom_ctxt ctxt = NULL;
  int err = 0;
  
  if (GEDCOM_IS_XREF_PTR(xref))
    xr = GEDCOM_XREF_PTR(xref);
  if (xr) {
    if (! xr->object) {
      user = make_user_record(xr->string);
      xr->object = (Gedcom_ctxt) user;
    }
    ctxt = xr->object;
    if (ctxt) {
      user = (struct user_rec*) ctxt;
    }
  }
  else {
    user = make_user_record(NULL);
    ctxt = (Gedcom_ctxt) user;
  }

  if (user) {
    user->tag = strdup(tag);
    if (! user->tag) {
      MEMORY_ERROR;
      err = 1;
    }
    else if (GEDCOM_IS_STRING(parsed_value)) {
      user->str_value = strdup(GEDCOM_STRING(parsed_value));
      if (!user->str_value) {
	MEMORY_ERROR;
	err = 1;
      }
    }
    else if (GEDCOM_IS_XREF_PTR(parsed_value))
      user->xref_value = GEDCOM_XREF_PTR(parsed_value);
    
    if (! err)
      result = MAKE_GOM_CTXT(rec, user_rec, ctxt);
  }
  
  return (Gedcom_ctxt)result;
}

GET_REC_BY_XREF(user_rec, XREF_USER, gom_get_user_rec_by_xref)

Gedcom_ctxt user_elt_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;
  int err = 0;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct user_data *data
      = (struct user_data *)malloc(sizeof(struct user_data));

    if (! data)
      MEMORY_ERROR;
    else {
      memset (data, 0, sizeof(struct user_data));
      
      data->level = level;
      data->tag = strdup(tag);
      if (! data->tag) {
	MEMORY_ERROR;
	free(data);
	err = 1;
      }
      else if (GEDCOM_IS_STRING(parsed_value)) {
	data->str_value = strdup(GEDCOM_STRING(parsed_value));
	if (! data->str_value) {
	  MEMORY_ERROR;
	  free(data->tag);
	  free(data->str_value);
	  err = 1;
	}
      }
      else if (GEDCOM_IS_XREF_PTR(parsed_value))
	data->xref_value = GEDCOM_XREF_PTR(parsed_value);

      if (! err) {
	switch (ctxt->obj_type) {
	  case T_header:
	    header_add_user_data(ctxt, data); break;
	  case T_submission:
	    submission_add_user_data(ctxt, data); break;
	  case T_submitter:
	    submitter_add_user_data(ctxt, data); break;
	  case T_family:
	    family_add_user_data(ctxt, data); break;
	  case T_individual:
	    individual_add_user_data(ctxt, data); break;
	  case T_multimedia:
	    multimedia_add_user_data(ctxt, data); break;
	  case T_note:
	    note_add_user_data(ctxt, data); break;
	  case T_repository:
	    repository_add_user_data(ctxt, data); break;
	  case T_source:
	    source_add_user_data(ctxt, data); break;
	  case T_user_rec:
	    user_rec_add_user_data(ctxt, data); break;
	  case T_address:
	    address_add_user_data(ctxt, data); break;
	  case T_event:
	    event_add_user_data(ctxt, data); break;
	  case T_place:
	    place_add_user_data(ctxt, data); break;
	  case T_source_citation:
	    citation_add_user_data(ctxt, data); break;
	  case T_note_sub:
	    note_sub_add_user_data(ctxt, data); break;
	  case T_multimedia_link:
	    multimedia_link_add_user_data(ctxt, data); break;
	  case T_lds_event:
	    lds_event_add_user_data(ctxt, data); break;
	  case T_user_ref_number:
	    user_ref_add_user_data(ctxt, data); break;
	  case T_change_date:
	    change_date_add_user_data(ctxt, data); break;
	  case T_personal_name:
	    name_add_user_data(ctxt, data); break;
	  case T_family_link:
	    family_link_add_user_data(ctxt, data); break;
	  case T_association:
	    association_add_user_data(ctxt, data); break;
	  case T_source_event:
	    source_event_add_user_data(ctxt, data); break;
	  case T_source_description:
	    source_description_add_user_data(ctxt, data); break;
	  default:
	    UNEXPECTED_CONTEXT(ctxt->ctxt_type);
	}
	result = make_gom_ctxt(elt, ctxt->obj_type, ctxt->ctxt_ptr);
      }
    }
  }
  
  return (Gedcom_ctxt)result;
}

void user_rec_subscribe()
{
  gedcom_subscribe_to_record(REC_USER, user_rec_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_USER, user_elt_start, def_elt_end);
}

void user_rec_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct user_rec *obj = SAFE_CTXT_CAST(user_rec, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void user_data_cleanup(struct user_data* data)
{
  if (data) {
    SAFE_FREE(data->tag);
    SAFE_FREE(data->str_value);
  }
}

void user_rec_cleanup(struct user_rec* rec)
{
  if (rec) {
    SAFE_FREE(rec->xrefstr);
    SAFE_FREE(rec->tag);
    SAFE_FREE(rec->str_value);
    DESTROY_CHAIN_ELTS(user_data, rec->extra, user_data_cleanup);
  }
}

void user_recs_cleanup()
{
  DESTROY_CHAIN_ELTS(user_rec, gom_first_user_rec, user_rec_cleanup);
}

struct user_rec* gom_get_first_user_rec()
{
  return gom_first_user_rec;
}

struct user_rec* make_user_record(char* xrefstr)
{
  struct user_rec* rec = NULL;
  MAKE_CHAIN_ELT(user_rec, gom_first_user_rec, rec);
  if (rec && xrefstr) {
    rec->xrefstr = strdup(xrefstr);
    if (! rec->xrefstr) MEMORY_ERROR;
  }
  return rec;
}