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

#include <time.h>
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

DEFINE_STRING_CB(header, head_sour_start, source.id)
DEFINE_STRING_CB(header, head_sour_name_start, source.name)
DEFINE_STRING_CB(header, head_sour_vers_start, source.version)
DEFINE_STRING_CB(header, head_sour_corp_start, source.corporation.name)
DEFINE_STRING_CB(header, head_sour_data_start, source.data.name)
DEFINE_DATE_CB(header, head_sour_data_date_start, source.data.date)
DEFINE_STRING_CB(header, head_sour_data_copr_start, source.data.copyright)
DEFINE_STRING_CB(header, head_dest_start, destination)
DEFINE_XREF_CB(header, head_subm_start, submitter, submitter)
DEFINE_XREF_CB(header, head_subn_start, submission, submission)
DEFINE_DATE_CB(header, head_date_start, date)
DEFINE_STRING_CB(header, head_date_time_start, time)
DEFINE_STRING_CB(header, head_file_start, filename)
DEFINE_STRING_CB(header, head_copr_start, copyright)
DEFINE_NULL_CB(header, head_gedc_start)
DEFINE_STRING_CB(header, head_gedc_vers_start, gedcom.version)
DEFINE_STRING_CB(header, head_gedc_form_start, gedcom.form)
DEFINE_STRING_CB(header, head_char_start, charset.name)
DEFINE_STRING_CB(header, head_char_vers_start, charset.version)
DEFINE_STRING_CB(header, head_lang_start, language)
DEFINE_NULL_CB(header, head_plac_start)
DEFINE_STRING_CB(header, head_plac_form_start, place_hierarchy)
DEFINE_NULL_CB(header, head_note_start) /* the end cb will fill the value */
DEFINE_STRING_END_CB(header, head_note_end, note)

DEFINE_ADDFUNC2_NOLIST(header, address, source.corporation.address)
DEFINE_ADDFUNC2(header, user_data, extra)
     
void header_add_phone(Gom_ctxt ctxt, const char* phone)
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
  gedcom_subscribe_to_element(ELT_HEAD_NOTE, head_note_start, head_note_end);
}

void header_cleanup()
{
  SAFE_FREE(gom_header.source.id);
  SAFE_FREE(gom_header.source.name);
  SAFE_FREE(gom_header.source.version);
  SAFE_FREE(gom_header.source.corporation.name);
  CLEANFUNC(address)(gom_header.source.corporation.address);
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
  DESTROY_CHAIN_ELTS(user_data, gom_header.extra);
}

struct header* gom_get_header()
{
  return &gom_header;
}

int gom_header_update_timestamp(time_t t)
{
  int result = 0;
  struct tm *tm_ptr = localtime(&t);
  result |= update_date(&gom_header.date, tm_ptr);
  result |= update_time(&gom_header.time, tm_ptr);
  return result;
}

int write_header(Gedcom_write_hndl hndl)
{
  int result = 0;
  int i;
  
  result |= gedcom_write_record_str(hndl, REC_HEAD, NULL, NULL);
  if (gom_header.charset.name)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_CHAR, 0, REC_HEAD,
				       gom_header.charset.name);
  if (gom_header.charset.version)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_CHAR_VERS, 0,
				       ELT_HEAD_CHAR,
				       gom_header.charset.version);
  if (gom_header.source.id)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_SOUR, 0, REC_HEAD,
				       gom_header.source.id);
  if (gom_header.source.name)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_SOUR_NAME, 0,
				       ELT_HEAD_SOUR,
				       gom_header.source.name);
  if (gom_header.source.version)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_SOUR_VERS, 0,
				       ELT_HEAD_SOUR,
				       gom_header.source.version);
  if (gom_header.source.corporation.name)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_SOUR_CORP, 0,
				       ELT_HEAD_SOUR,
				       gom_header.source.corporation.name);
  if (gom_header.source.corporation.address)
    result |= write_address(hndl, ELT_HEAD_SOUR_CORP,
			    gom_header.source.corporation.address);
  for (i = 0; i < 3 && gom_header.source.corporation.phone[i]; i++)
    result |= gedcom_write_element_str(hndl, ELT_SUB_PHON, 0, ELT_HEAD_SOUR_CORP,
				       gom_header.source.corporation.phone[i]);
  if (gom_header.source.data.name)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_SOUR_DATA, 0,
				       ELT_HEAD_SOUR,
				       gom_header.source.data.name);
  if (gom_header.source.data.date)
    result |= gedcom_write_element_date(hndl, ELT_HEAD_SOUR_DATA_DATE, 0,
					ELT_HEAD_SOUR_DATA,
					gom_header.source.data.date);
  if (gom_header.source.data.copyright)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_SOUR_DATA_COPR, 0,
				       ELT_HEAD_SOUR_DATA,
				       gom_header.source.data.copyright);
  if (gom_header.destination)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_DEST, 0, REC_HEAD,
				       gom_header.destination);
  if (gom_header.date)
    result |= gedcom_write_element_date(hndl, ELT_HEAD_DATE, 0, REC_HEAD,
					gom_header.date);
  if (gom_header.time)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_DATE_TIME, 0,
				       ELT_HEAD_DATE, gom_header.time);
  if (gom_header.submitter)
    result |= gedcom_write_element_xref(hndl, ELT_HEAD_SUBM, 0, REC_HEAD,
					gom_header.submitter);
  if (gom_header.submission)
    result |= gedcom_write_element_xref(hndl, ELT_HEAD_SUBN, 0, REC_HEAD,
					gom_header.submission);
  if (gom_header.filename)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_FILE, 0, REC_HEAD,
				       gom_header.filename);
  if (gom_header.copyright)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_COPR, 0, REC_HEAD,
				       gom_header.copyright);
  result |= gedcom_write_element_str(hndl, ELT_HEAD_GEDC, 0, REC_HEAD,
				     NULL);
  if (gom_header.gedcom.version)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_GEDC_VERS, 0,
				       ELT_HEAD_GEDC,
				       gom_header.gedcom.version);
  if (gom_header.gedcom.form)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_GEDC_FORM, 0,
				       ELT_HEAD_GEDC,
				       gom_header.gedcom.form);
  if (gom_header.language)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_LANG, 0,
				       REC_HEAD,
				       gom_header.language);
  if (gom_header.place_hierarchy) {
    result |= gedcom_write_element_str(hndl, ELT_HEAD_PLAC, 0, REC_HEAD,
				       NULL);
    result |= gedcom_write_element_str(hndl, ELT_HEAD_PLAC_FORM, 0,
				       ELT_HEAD_PLAC,
				       gom_header.place_hierarchy);
  }
  if (gom_header.note)
    result |= gedcom_write_element_str(hndl, ELT_HEAD_NOTE, 0,
				       REC_HEAD,
				       gom_header.note);
  if (gom_header.extra)
    result |= write_user_data(hndl, gom_header.extra);
  return result;
}
