/* Submission object in the gedcom object model.
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
#include "submission.h"
#include "submitter.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

struct submission* gom_submission = NULL;

REC_CB(submission, subn_start, make_submission_record)
XREF_CB(submission, subn_subm_start, submitter, make_submitter_record)
STRING_CB(submission, subn_famf_start, family_file)
STRING_CB(submission, subn_temp_start, temple_code)
STRING_CB(submission, subn_ance_start, nr_of_ancestor_gens)
STRING_CB(submission, subn_desc_start, nr_of_descendant_gens)
STRING_CB(submission, subn_ordi_start, ordinance_process_flag)
STRING_CB(submission, subn_rin_start, record_id)
     
void submission_subscribe()
{
  gedcom_subscribe_to_record(REC_SUBN, subn_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_SUBN_SUBM, subn_subm_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUBN_FAMF, subn_famf_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUBN_TEMP, subn_temp_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUBN_ANCE, subn_ance_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUBN_DESC, subn_desc_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUBN_ORDI, subn_ordi_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUBN_RIN, subn_rin_start, def_elt_end);  
}

void submission_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct submission *obj = SAFE_CTXT_CAST(submission, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void submission_cleanup()
{
  if (gom_submission) {
    SAFE_FREE(gom_submission->xrefstr);
    SAFE_FREE(gom_submission->family_file);
    SAFE_FREE(gom_submission->temple_code);
    SAFE_FREE(gom_submission->nr_of_ancestor_gens);
    SAFE_FREE(gom_submission->nr_of_descendant_gens);
    SAFE_FREE(gom_submission->ordinance_process_flag);
    SAFE_FREE(gom_submission->record_id);
    DESTROY_CHAIN_ELTS(user_data, gom_submission->extra, user_data_cleanup);
    SAFE_FREE(gom_submission);
  }
}

struct submission* gom_get_submission()
{
  return gom_submission;
}

struct submission* make_submission_record(const char* xref)
{
  if (! gom_submission) {
    gom_submission = (struct submission*)malloc(sizeof(struct submission));
    if (! gom_submission)
      MEMORY_ERROR;
    else {
      memset(gom_submission, 0, sizeof(struct submission));
      gom_submission->xrefstr = strdup(xref);
      if (!gom_submission->xrefstr) MEMORY_ERROR;
    }
  }
  
  return gom_submission;
}

int write_submission(Gedcom_write_hndl hndl)
{
  int result = 0;

  if (gom_submission) {
    result |= gedcom_write_record_str(hndl, REC_SUBN, 
				      gom_submission->xrefstr, NULL);
    if (gom_submission->submitter)
      result |= gedcom_write_element_xref(hndl, ELT_SUBN_SUBM, 0,
					  REC_SUBN, gom_submission->submitter);
    if (gom_submission->family_file)
      result |= gedcom_write_element_str(hndl, ELT_SUBN_FAMF, 0, REC_SUBN,
					 gom_submission->family_file);
    if (gom_submission->temple_code)
      result |= gedcom_write_element_str(hndl, ELT_SUBN_TEMP, 0, REC_SUBN,
					 gom_submission->temple_code);
    if (gom_submission->nr_of_ancestor_gens)
      result |= gedcom_write_element_str(hndl, ELT_SUBN_ANCE, 0, REC_SUBN,
					 gom_submission->nr_of_ancestor_gens);
    if (gom_submission->nr_of_descendant_gens)
      result |= gedcom_write_element_str(hndl, ELT_SUBN_DESC, 0, REC_SUBN,
					gom_submission->nr_of_descendant_gens);
    if (gom_submission->ordinance_process_flag)
      result |= gedcom_write_element_str(hndl, ELT_SUBN_ORDI, 0, REC_SUBN,
				      gom_submission->ordinance_process_flag);
    if (gom_submission->record_id)
      result |= gedcom_write_element_str(hndl, ELT_SUBN_RIN, 0, REC_SUBN,
					 gom_submission->record_id);
    if (gom_submission->extra)
      result |= write_user_data(hndl, gom_submission->extra);
  }
  
  return result;
}
