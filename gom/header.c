/* Header object in the gedcom object model.
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

#include "header.h"
#include "submission.h"
#include "submitter.h"
#include "address.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

struct header gom_header;

Gedcom_ctxt head_start(_REC_PARAMS_)
{
  /* Nothing special */
  return (Gedcom_ctxt) MAKE_GOM_CTXT(rec, header, &gom_header);
}

STRING_CB(header, head_sour_start, source.id)
STRING_CB(header, head_sour_name_start, source.name)
STRING_CB(header, head_sour_vers_start, source.version)
STRING_CB(header, head_sour_corp_start, source.corporation.name)
STRING_CB(header, head_sour_data_start, source.data.name)
DATE_CB(header, head_sour_data_date_start, source.data.date)
STRING_CB(header, head_sour_data_copr_start, source.data.copyright)
STRING_CB(header, head_dest_start, destination)
XREF_CB(header, head_subm_start, submitter, make_submitter_record)
XREF_CB(header, head_subn_start, submission, make_submission_record)
DATE_CB(header, head_date_start, date)
STRING_CB(header, head_date_time_start, time)
STRING_CB(header, head_file_start, filename)
STRING_CB(header, head_copr_start, copyright)
NULL_CB(header, head_gedc_start)
STRING_CB(header, head_gedc_vers_start, gedcom.version)
STRING_CB(header, head_gedc_form_start, gedcom.form)
STRING_CB(header, head_char_start, charset.name)
STRING_CB(header, head_char_vers_start, charset.version)
STRING_CB(header, head_lang_start, language)
NULL_CB(header, head_plac_start)
STRING_CB(header, head_plac_form_start, place_hierarchy)
STRING_CB(header, head_note_start, note)
     
void header_add_address(Gom_ctxt ctxt, struct address* addr)
{
  struct header *head = SAFE_CTXT_CAST(header, ctxt);
  if (head)
    head->source.corporation.address = addr;
}

void header_add_phone(Gom_ctxt ctxt, char* phone)
{
  struct header *head = SAFE_CTXT_CAST(header, ctxt);
  if (head) {
    struct header_corporation *corp = &(head->source.corporation);
    int i = 0;
    while (i<2 && corp->phone[i]) i++;
    if (! corp->phone[i]) {
      corp->phone[i] = strdup(phone);
      if (! corp->phone[i]) MEMORY_ERROR;
    }
  }
}

void header_add_to_note(NL_TYPE type, Gom_ctxt ctxt, char* str)
{
  struct header *head = SAFE_CTXT_CAST(header, ctxt);
  if (head) {
    char *newvalue = concat_strings(type, head->note, str);
    if (newvalue)
      head->note = newvalue;
    else
      MEMORY_ERROR;
  }
}

void header_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct header *head = SAFE_CTXT_CAST(header, ctxt);
  if (head)
    LINK_CHAIN_ELT(user_data, head->extra, data);
}

void header_subscribe()
{
  gedcom_subscribe_to_record(REC_HEAD, head_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_HEAD_SOUR, head_sour_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_SOUR_NAME, head_sour_name_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_SOUR_VERS, head_sour_vers_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_SOUR_CORP, head_sour_corp_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_SOUR_DATA, head_sour_data_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_SOUR_DATA_DATE,
			      head_sour_data_date_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_SOUR_DATA_COPR,
			      head_sour_data_copr_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_DEST, head_dest_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_DATE, head_date_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_DATE_TIME,
			      head_date_time_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_SUBM, head_subm_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_SUBN, head_subn_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_FILE, head_file_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_COPR, head_copr_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_GEDC, head_gedc_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_GEDC_VERS,
			      head_gedc_vers_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_GEDC_FORM,
			      head_gedc_form_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_CHAR, head_char_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_CHAR_VERS,
			      head_char_vers_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_LANG, head_lang_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_PLAC, head_plac_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_PLAC_FORM,
			      head_plac_form_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_HEAD_NOTE, head_note_start, def_elt_end);
}

void header_cleanup()
{
  SAFE_FREE(gom_header.source.id);
  SAFE_FREE(gom_header.source.name);
  SAFE_FREE(gom_header.source.version);
  SAFE_FREE(gom_header.source.corporation.name);
  address_cleanup(gom_header.source.corporation.address);
  SAFE_FREE(gom_header.source.corporation.phone[0]);
  SAFE_FREE(gom_header.source.corporation.phone[1]);
  SAFE_FREE(gom_header.source.corporation.phone[2]);
  SAFE_FREE(gom_header.source.data.name);
  SAFE_FREE(gom_header.source.data.date);
  SAFE_FREE(gom_header.source.data.copyright);
  SAFE_FREE(gom_header.destination);
  SAFE_FREE(gom_header.date);
  SAFE_FREE(gom_header.time);
  SAFE_FREE(gom_header.filename);
  SAFE_FREE(gom_header.copyright);
  SAFE_FREE(gom_header.gedcom.version);
  SAFE_FREE(gom_header.gedcom.form);
  SAFE_FREE(gom_header.charset.name);
  SAFE_FREE(gom_header.charset.version);
  SAFE_FREE(gom_header.language);
  SAFE_FREE(gom_header.place_hierarchy);
  SAFE_FREE(gom_header.note);
  DESTROY_CHAIN_ELTS(user_data, gom_header.extra, user_data_cleanup);
}

struct header* gom_get_header()
{
  return &gom_header;
}
