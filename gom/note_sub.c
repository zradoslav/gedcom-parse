/* Note sub-structure in the gedcom object model.
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
#include "note.h"
#include "note_sub.h"
#include "place.h"
#include "event.h"
#include "source_citation.h"
#include "multimedia_link.h"
#include "lds_event.h"
#include "family.h"
#include "change_date.h"
#include "personal_name.h"
#include "family_link.h"
#include "association.h"
#include "individual.h"
#include "multimedia.h"
#include "repository.h"
#include "source.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_note_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct note_sub *note = (struct note_sub *)malloc(sizeof(struct note_sub));
    if (! note)
      MEMORY_ERROR;
    else {
      memset (note, 0, sizeof(struct note_sub));
      if (GEDCOM_IS_XREF_PTR(parsed_value))
	note->reference = GEDCOM_XREF_PTR(parsed_value);

      switch (ctxt->ctxt_type) {
	case ELT_SUB_PLAC:
	  place_add_note(ctxt, note); break;
	case ELT_SUB_FAM_EVT:
	case ELT_SUB_FAM_EVT_EVEN:
	case ELT_SUB_INDIV_ATTR:
	case ELT_SUB_INDIV_RESI:
	case ELT_SUB_INDIV_BIRT:
	case ELT_SUB_INDIV_GEN:
	case ELT_SUB_INDIV_ADOP:
	case ELT_SUB_INDIV_EVEN:
	  event_add_note(ctxt, note); break;
	case ELT_SUB_SOUR:
	  citation_add_note(ctxt, note); break;
	case ELT_SUB_MULTIM_OBJE:
	  multimedia_link_add_note(ctxt, note); break;
	case ELT_SUB_LSS_SLGS:
	case ELT_SUB_LIO_BAPL:
	case ELT_SUB_LIO_SLGC:
	  lds_event_add_note(ctxt, note); break;
	case REC_FAM:
	  family_add_note(ctxt, note); break;
	case ELT_SUB_CHAN:
	  change_date_add_note(ctxt, note); break;
	case ELT_SUB_PERS_NAME:
	  name_add_note(ctxt, note); break;
	case ELT_SUB_FAMC: 
	case ELT_SUB_FAMS:
	  family_link_add_note(ctxt, note); break;
	case ELT_SUB_ASSO:
	  association_add_note(ctxt, note); break;
	case REC_INDI:
	  individual_add_note(ctxt, note); break;
	case REC_OBJE:
	  multimedia_add_note(ctxt, note); break;
	case REC_REPO:
	  repository_add_note(ctxt, note); break;
	case ELT_SOUR_DATA:
	  source_add_note_to_data(ctxt, note); break;
	case ELT_SUB_REPO:
	  source_add_note_to_repo(ctxt, note); break;
	case REC_SOUR:
	  source_add_note(ctxt, note); break;
	default:
	  UNEXPECTED_CONTEXT(ctxt->ctxt_type);
      }
      result = MAKE_GOM_CTXT(elt, note_sub, note);
    }
  }

  return (Gedcom_ctxt)result;
}

STRING_END_CB(note_sub, sub_note_end, text)
     
void note_sub_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_NOTE, sub_note_start, sub_note_end);
}

void note_sub_add_citation(Gom_ctxt ctxt, struct source_citation* cit)
{
  struct note_sub *note = SAFE_CTXT_CAST(note_sub, ctxt);
  if (note)
    LINK_CHAIN_ELT(source_citation, note->citation, cit);    
}

void note_sub_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct note_sub *obj = SAFE_CTXT_CAST(note_sub, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);;
}

void note_sub_cleanup(struct note_sub* note)
{
  if (note) {
    SAFE_FREE(note->text);
    DESTROY_CHAIN_ELTS(source_citation, note->citation, citation_cleanup);
    DESTROY_CHAIN_ELTS(user_data, note->extra, user_data_cleanup);
  }
}

int write_note_subs(Gedcom_write_hndl hndl, int parent, struct note_sub* note)
{
  int result = 0;
  struct note_sub* obj;

  if (!note) return 1;

  for (obj = note; obj; obj = obj->next) {
    if (obj->reference) {
      result |= gedcom_write_element_xref(hndl, ELT_SUB_NOTE, 0,
					  parent, obj->reference);
    }
    else {
      result |= gedcom_write_element_str(hndl, ELT_SUB_NOTE, 0,
					 parent, obj->text);
    }
    if (obj->citation)
      result |= write_citations(hndl, ELT_SUB_NOTE, obj->citation);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }

  return result;
}
