/* Submitter object in the gedcom object model.
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
#include "submitter.h"
#include "address.h"
#include "multimedia_link.h"
#include "change_date.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

struct submitter* gom_first_submitter = NULL;

REC_CB(submitter, subm_start, make_submitter_record)
GET_REC_BY_XREF(submitter, XREF_SUBM, gom_get_submitter_by_xref)
STRING_CB(submitter, subm_name_start, name)
STRING_CB(submitter, subm_rfn_start, record_file_nr)
STRING_CB(submitter, subm_rin_start, record_id)

Gedcom_ctxt subm_lang_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct submitter *subm = SAFE_CTXT_CAST(submitter, ctxt);

    if (subm) {
      int err = 0;
      char *str = GEDCOM_STRING(parsed_value);
      int i = 0;

      while (i<2 && subm->language[i]) i++;
      if (! subm->language[i]) {
	subm->language[i] = strdup(str);
	if (! subm->language[i]) {
	  MEMORY_ERROR;
	  err = 1;
	}
      }
      if (! err)
	result = MAKE_GOM_CTXT(elt, submitter, subm);
    }
  }
  
  return (Gedcom_ctxt)result;
}

void submitter_subscribe()
{
  gedcom_subscribe_to_record(REC_SUBM, subm_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_SUBM_NAME, subm_name_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUBM_LANG, subm_lang_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUBM_RFN, subm_rfn_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUBM_RIN, subm_rin_start, def_elt_end);
}

void submitter_add_address(Gom_ctxt ctxt, struct address* address)
{
  struct submitter *subm = SAFE_CTXT_CAST(submitter, ctxt);
  if (subm)
    subm->address = address;
}

void submitter_add_phone(Gom_ctxt ctxt, char *phone)
{
  struct submitter *subm = SAFE_CTXT_CAST(submitter, ctxt);
  if (subm) {
    int i = 0;
    while (i<2 && subm->phone[i]) i++;
    if (! subm->phone[i]) {
      subm->phone[i] = strdup(phone);
      if (! subm->phone[i]) MEMORY_ERROR;
    }
  }
}

void submitter_add_mm_link(Gom_ctxt ctxt, struct multimedia_link* link)
{
  struct submitter *subm = SAFE_CTXT_CAST(submitter, ctxt);
  if (subm)
    LINK_CHAIN_ELT(multimedia_link, subm->mm_link, link);
}

void submitter_set_change_date(Gom_ctxt ctxt, struct change_date* chan)
{
  struct submitter *subm = SAFE_CTXT_CAST(submitter, ctxt);
  if (subm)
    subm->change_date = chan;
}

void submitter_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct submitter *obj = SAFE_CTXT_CAST(submitter, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void submitter_cleanup(struct submitter* rec)
{
  if (rec) {
    SAFE_FREE(rec->xrefstr);
    SAFE_FREE(rec->name);
    address_cleanup(rec->address);
    SAFE_FREE(rec->phone[0]);
    SAFE_FREE(rec->phone[1]);
    SAFE_FREE(rec->phone[2]);
    DESTROY_CHAIN_ELTS(multimedia_link, rec->mm_link, multimedia_link_cleanup);
    SAFE_FREE(rec->language[0]);
    SAFE_FREE(rec->language[1]);
    SAFE_FREE(rec->language[2]);
    SAFE_FREE(rec->record_file_nr);
    SAFE_FREE(rec->record_id);
    change_date_cleanup(rec->change_date);
    DESTROY_CHAIN_ELTS(user_data, rec->extra, user_data_cleanup);
  }
}

void submitters_cleanup()
{
  DESTROY_CHAIN_ELTS(submitter, gom_first_submitter, submitter_cleanup);
}

struct submitter* gom_get_first_submitter()
{
  return gom_first_submitter;
}

struct submitter* make_submitter_record(char* xrefstr)
{
  struct submitter* subm = NULL;
  MAKE_CHAIN_ELT(submitter, gom_first_submitter, subm);
  if (subm) {
    subm->xrefstr = strdup(xrefstr);
    if (!subm->xrefstr) MEMORY_ERROR;
  }
  return subm;
}
