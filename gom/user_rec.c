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
      user = MAKEFUNC(user_rec)(xr->string);
      xr->object = (Gedcom_ctxt) user;
    }
    ctxt = xr->object;
    if (ctxt) {
      user = (struct user_rec*) ctxt;
    }
  }
  else {
    user = MAKEFUNC(user_rec)(NULL);
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

DEFINE_DESTROYFUNC(user_rec, gom_first_user_rec)
DEFINE_DELETEFUNC(user_rec)
DEFINE_GETXREFFUNC(user_rec, XREF_USER)

DEFINE_ADDFUNC2(user_rec, user_data, extra)

/* Specific function, because xrefstr not mandatory here */
struct user_rec* MAKEFUNC(user_rec)(const char* xrefstr)
{
  struct user_rec* rec = NULL;
  MAKE_CHAIN_ELT(user_rec, gom_first_user_rec, rec);
  if (rec && xrefstr) {
    rec->xrefstr = strdup(xrefstr);
    if (! rec->xrefstr) MEMORY_ERROR;
  }
  return rec;
}

struct user_rec* ADDFUNC(user_rec)(const char* xrefstr, const char* tag)
{
  struct user_rec *obj = NULL;
  struct xref_value* xrv = gedcom_get_by_xref(xrefstr);
  if (tag && tag[0] == '_') {
    if (xrv)
      gom_xref_already_in_use(xrefstr);
    else {
      obj = MAKEFUNC(user_rec)(xrefstr);
      if (obj) {
	obj->tag = strdup(tag);
	if (! obj->tag)
	  MEMORY_ERROR;
	else
	  xrv = gedcom_add_xref(XREF_USER, xrefstr, (Gedcom_ctxt)obj);
	if (!xrv) {
	  DESTROYFUNC(user_rec)(obj);
	  obj = NULL;
	}
      }
    }
  }
  return obj;
}

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
	    ADDFUNC2(header,user_data)(ctxt, data); break;
	  case T_submission:
	    ADDFUNC2(submission,user_data)(ctxt, data); break;
	  case T_submitter:
	    ADDFUNC2(submitter,user_data)(ctxt, data); break;
	  case T_family:
	    ADDFUNC2(family,user_data)(ctxt, data); break;
	  case T_individual:
	    ADDFUNC2(individual,user_data)(ctxt, data); break;
	  case T_multimedia:
	    ADDFUNC2(multimedia,user_data)(ctxt, data); break;
	  case T_note:
	    ADDFUNC2(note,user_data)(ctxt, data); break;
	  case T_repository:
	    ADDFUNC2(repository,user_data)(ctxt, data); break;
	  case T_source:
	    ADDFUNC2(source,user_data)(ctxt, data); break;
	  case T_user_rec:
	    ADDFUNC2(user_rec,user_data)(ctxt, data); break;
	  case T_address:
	    ADDFUNC2(address,user_data)(ctxt, data); break;
	  case T_event:
	    ADDFUNC2(event,user_data)(ctxt, data); break;
	  case T_place:
	    ADDFUNC2(place,user_data)(ctxt, data); break;
	  case T_source_citation:
	    ADDFUNC2(source_citation,user_data)(ctxt, data); break;
	  case T_note_sub:
	    ADDFUNC2(note_sub,user_data)(ctxt, data); break;
	  case T_multimedia_link:
	    ADDFUNC2(multimedia_link,user_data)(ctxt, data); break;
	  case T_lds_event:
	    ADDFUNC2(lds_event,user_data)(ctxt, data); break;
	  case T_user_ref_number:
	    ADDFUNC2(user_ref_number,user_data)(ctxt, data); break;
	  case T_change_date:
	    ADDFUNC2(change_date,user_data)(ctxt, data); break;
	  case T_personal_name:
	    ADDFUNC2(personal_name,user_data)(ctxt, data); break;
	  case T_family_link:
	    ADDFUNC2(family_link,user_data)(ctxt, data); break;
	  case T_association:
	    ADDFUNC2(association,user_data)(ctxt, data); break;
	  case T_source_event:
	    ADDFUNC2(source_event,user_data)(ctxt, data); break;
	  case T_source_description:
	    ADDFUNC2(source_description,user_data)(ctxt, data); break;
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

void UNREFALLFUNC(user_data)(struct user_data *obj)
{
  if (obj) {
    struct user_data* runner;
    for (runner = obj; runner; runner = runner->next)
      unref_xref_value(runner->xref_value);
  }
}

void CLEANFUNC(user_data)(struct user_data* data)
{
  if (data) {
    SAFE_FREE(data->tag);
    SAFE_FREE(data->str_value);
  }
}

void UNREFALLFUNC(user_rec)(struct user_rec *obj)
{
  if (obj) {
    unref_xref_value(obj->xref_value);
    UNREFALLFUNC(user_data)(obj->extra);
  }
}

void CLEANFUNC(user_rec)(struct user_rec* rec)
{
  if (rec) {
    SAFE_FREE(rec->xrefstr);
    SAFE_FREE(rec->tag);
    SAFE_FREE(rec->str_value);
    DESTROY_CHAIN_ELTS(user_data, rec->extra);
  }
}

void user_recs_cleanup()
{
  DESTROY_CHAIN_ELTS(user_rec, gom_first_user_rec);
}

struct user_rec* gom_get_first_user_rec()
{
  return gom_first_user_rec;
}

int write_user_recs(Gedcom_write_hndl hndl)
{
  int result = 0;
  struct user_rec* obj;

  for (obj = gom_first_user_rec; obj; obj = obj->next) {
    if (obj->xref_value)
      result |= gedcom_write_user_xref(hndl, 0, obj->tag, obj->xrefstr,
				       obj->xref_value);
    else
      result |= gedcom_write_user_str(hndl, 0, obj->tag, obj->xrefstr,
				      obj->str_value);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }
  return result;  
}

int write_user_data(Gedcom_write_hndl hndl, struct user_data* data)
{
  int result = 0;
  struct user_data* obj;

  if (!data) return 1;

  for (obj = data; data; data = data->next) {
    if (obj->xref_value)
      result |= gedcom_write_user_xref(hndl, obj->level, obj->tag, NULL,
				       obj->xref_value);
    else
      result |= gedcom_write_user_str(hndl, obj->level, obj->tag, NULL,
				      obj->str_value);
  }
  return result;
}
