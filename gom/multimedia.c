/* Multimedia object in the gedcom object model.
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
#include "multimedia.h"
#include "note_sub.h"
#include "user_ref.h"
#include "change_date.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

struct multimedia* gom_first_multimedia = NULL;

REC_CB(multimedia, obje_start, make_multimedia_record)
GET_REC_BY_XREF(multimedia, XREF_OBJE, gom_get_multimedia_by_xref)
STRING_CB(multimedia, obje_form_start, form)
STRING_CB(multimedia, obje_titl_start, title)     
NULL_CB(multimedia, obje_blob_start)     
XREF_CB(multimedia, obje_obje_start, continued, make_multimedia_record)

Gedcom_ctxt obje_blob_cont_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  struct multimedia *obj = SAFE_CTXT_CAST(multimedia, ctxt);
  char *str = GEDCOM_STRING(parsed_value);
  if (obj->data)
    obj->data = concat_strings (WITHOUT_NL, obj->data, str);
  else
    obj->data = strdup(str);
  return (Gedcom_ctxt) make_gom_ctxt(elt, ctxt->obj_type, ctxt->ctxt_ptr);
}

void multimedia_subscribe()
{
  gedcom_subscribe_to_record(REC_OBJE, obje_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_OBJE_FORM, obje_form_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_OBJE_TITL, obje_titl_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_OBJE_BLOB, obje_blob_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_OBJE_BLOB_CONT, obje_blob_cont_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_OBJE_OBJE, obje_obje_start, def_elt_end);
}

void multimedia_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct multimedia* obj = SAFE_CTXT_CAST(multimedia, ctxt);
  LINK_CHAIN_ELT(note_sub, obj->note, note)
}

void multimedia_add_user_ref(Gom_ctxt ctxt, struct user_ref_number* ref)
{
  struct multimedia *obj = SAFE_CTXT_CAST(multimedia, ctxt);
  LINK_CHAIN_ELT(user_ref_number, obj->ref, ref)
}

void multimedia_set_record_id(Gom_ctxt ctxt, char *rin)
{
  struct multimedia *obj = SAFE_CTXT_CAST(multimedia, ctxt);
  obj->record_id = strdup(rin);
}

void multimedia_set_change_date(Gom_ctxt ctxt, struct change_date* chan)
{
  struct multimedia *obj = SAFE_CTXT_CAST(multimedia, ctxt);
  obj->change_date = chan;
}

void multimedia_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct multimedia *obj = SAFE_CTXT_CAST(multimedia, ctxt);
  LINK_CHAIN_ELT(user_data, obj->extra, data)
}

void multimedia_cleanup(struct multimedia* obj)
{
  SAFE_FREE(obj->xrefstr);
  SAFE_FREE(obj->form);
  SAFE_FREE(obj->title);
  DESTROY_CHAIN_ELTS(note_sub, obj->note, note_sub_cleanup)
  SAFE_FREE(obj->data);
  DESTROY_CHAIN_ELTS(user_ref_number, obj->ref, user_ref_cleanup)
  SAFE_FREE(obj->record_id);
  change_date_cleanup(obj->change_date);
  DESTROY_CHAIN_ELTS(user_data, obj->extra, user_data_cleanup)
}

void multimedias_cleanup()
{
  DESTROY_CHAIN_ELTS(multimedia, gom_first_multimedia, multimedia_cleanup);
}

struct multimedia* gom_get_first_multimedia()
{
  return gom_first_multimedia;
}

struct multimedia* make_multimedia_record(char* xrefstr)
{
  struct multimedia* multi;
  MAKE_CHAIN_ELT(multimedia, gom_first_multimedia, multi);
  multi->xrefstr = strdup(xrefstr);
  return multi;
}
