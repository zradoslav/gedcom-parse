/* Multimedia link sub-structure in the gedcom object model.
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
#include "multimedia_link.h"
#include "event.h"
#include "source_citation.h"
#include "note_sub.h"
#include "family.h"
#include "individual.h"
#include "source.h"
#include "submitter.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_obje_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  struct multimedia_link *mm = NULL;

  if (ctxt) {
    mm = (struct multimedia_link *)malloc(sizeof(struct multimedia_link));
    memset (mm, 0, sizeof(struct multimedia_link));
    if (GEDCOM_IS_XREF_PTR(parsed_value))
      mm->reference = GEDCOM_XREF_PTR(parsed_value);

    switch (ctxt->ctxt_type) {
      case ELT_SUB_FAM_EVT:
      case ELT_SUB_FAM_EVT_EVEN:
      case ELT_SUB_INDIV_ATTR:
      case ELT_SUB_INDIV_RESI:
      case ELT_SUB_INDIV_BIRT:
      case ELT_SUB_INDIV_GEN:
      case ELT_SUB_INDIV_ADOP:
      case ELT_SUB_INDIV_EVEN:
	event_add_mm_link(ctxt, mm); break;
      case ELT_SUB_SOUR:
	citation_add_mm_link(ctxt, mm); break;
      case REC_FAM:
	family_add_mm_link(ctxt, mm); break;
      case REC_INDI:
	individual_add_mm_link(ctxt, mm); break;
      case REC_SOUR:
	source_add_mm_link(ctxt, mm); break;
      case REC_SUBM:
	submitter_add_mm_link(ctxt, mm); break;
      default:
	UNEXPECTED_CONTEXT(ctxt->ctxt_type);
    }
  }

  return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, multimedia_link, mm);
}

STRING_CB(multimedia_link, sub_obje_form_start, form)
STRING_CB(multimedia_link, sub_obje_titl_start, title)
STRING_CB(multimedia_link, sub_obje_file_start, file)
     
void multimedia_link_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_MULTIM_OBJE,
			      sub_obje_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_MULTIM_OBJE_FORM,
			      sub_obje_form_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_MULTIM_OBJE_TITL,
			      sub_obje_titl_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_MULTIM_OBJE_FILE,
			      sub_obje_file_start, def_elt_end);
}

void multimedia_link_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct multimedia_link *mm = SAFE_CTXT_CAST(multimedia_link, ctxt);
  LINK_CHAIN_ELT(note_sub, mm->note, note)    
}

void multimedia_link_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct multimedia_link *obj = SAFE_CTXT_CAST(multimedia_link, ctxt);
  LINK_CHAIN_ELT(user_data, obj->extra, data)
}

void multimedia_link_cleanup(struct multimedia_link* mm)
{
  if (mm) {
    SAFE_FREE(mm->form);
    SAFE_FREE(mm->title);
    SAFE_FREE(mm->file);
    DESTROY_CHAIN_ELTS(note_sub, mm->note, note_sub_cleanup)
    DESTROY_CHAIN_ELTS(user_data, mm->extra, user_data_cleanup)
  }
}
