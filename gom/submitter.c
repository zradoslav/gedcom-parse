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

DEFINE_MAKEFUNC(submitter, gom_first_submitter)
DEFINE_DESTROYFUNC(submitter, gom_first_submitter)
DEFINE_ADDFUNC(submitter, XREF_SUBM)
DEFINE_DELETEFUNC(submitter)
DEFINE_GETXREFFUNC(submitter, XREF_SUBM)
DEFINE_MAKELINKFUNC(submitter, XREF_SUBM)
     
DEFINE_REC_CB(submitter, subm_start)
DEFINE_STRING_CB(submitter, subm_name_start, name)
DEFINE_STRING_CB(submitter, subm_rfn_start, record_file_nr)
DEFINE_STRING_CB(submitter, subm_rin_start, record_id)

DEFINE_ADDFUNC2(submitter, multimedia_link, mm_link)
DEFINE_ADDFUNC2(submitter, user_data, extra)
DEFINE_ADDFUNC2_NOLIST(submitter, address, address)
DEFINE_ADDFUNC2_NOLIST(submitter, change_date, change_date)
DEFINE_ADDFUNC2_STRN(submitter, phone, 3)

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

void CLEANFUNC(submitter)(struct submitter* rec)
{
  if (rec) {
    SAFE_FREE(rec->xrefstr);
    SAFE_FREE(rec->name);
    CLEANFUNC(address)(rec->address);
    SAFE_FREE(rec->phone[0]);
    SAFE_FREE(rec->phone[1]);
    SAFE_FREE(rec->phone[2]);
    DESTROY_CHAIN_ELTS(multimedia_link, rec->mm_link);
    SAFE_FREE(rec->language[0]);
    SAFE_FREE(rec->language[1]);
    SAFE_FREE(rec->language[2]);
    SAFE_FREE(rec->record_file_nr);
    SAFE_FREE(rec->record_id);
    CLEANFUNC(change_date)(rec->change_date);
    DESTROY_CHAIN_ELTS(user_data, rec->extra);
  }
}

void submitters_cleanup()
{
  DESTROY_CHAIN_ELTS(submitter, gom_first_submitter);
}

struct submitter* gom_get_first_submitter()
{
  return gom_first_submitter;
}

int write_submitters(Gedcom_write_hndl hndl)
{
  int result = 0;
  int i;
  struct submitter* obj;

  for (obj = gom_first_submitter; obj; obj = obj->next) {
    result |= gedcom_write_record_str(hndl, REC_SUBM, obj->xrefstr, NULL);
    if (obj->name)
      result |= gedcom_write_element_str(hndl, ELT_SUBM_NAME, 0, REC_SUBM,
					 obj->name);
    if (obj->address)
      result |= write_address(hndl, REC_SUBM, obj->address);
    for (i = 0; i < 3 && obj->phone[i]; i++)
      result |= gedcom_write_element_str(hndl, ELT_SUB_PHON, 0, REC_SUBM,
					 obj->phone[i]);
    if (obj->mm_link)
      result |= write_multimedia_links(hndl, REC_SUBM, obj->mm_link);
    for (i = 0; i < 3 && obj->language[i]; i++)
      result |= gedcom_write_element_str(hndl, ELT_SUBM_LANG, 0, REC_SUBM,
					 obj->language[i]);
    if (obj->record_file_nr)
      result |= gedcom_write_element_str(hndl, ELT_SUBM_RFN, 0, REC_SUBM,
					 obj->record_file_nr);
    if (obj->record_id)
      result |= gedcom_write_element_str(hndl, ELT_SUBM_RIN, 0, REC_SUBM,
					 obj->record_id);
    if (obj->change_date)
      result |= write_change_date(hndl, REC_SUBM, obj->change_date);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }
  
  return result;
}

