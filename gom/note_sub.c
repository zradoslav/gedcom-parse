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
    struct note_sub *note = SUB_MAKEFUNC(note_sub)();
    if (note) {
      if (GEDCOM_IS_XREF_PTR(parsed_value))
	note->reference = GEDCOM_XREF_PTR(parsed_value);

      switch (ctxt->ctxt_type) {
	case ELT_SUB_PLAC:
	  ADDFUNC2(place,note_sub)(ctxt, note); break;
	case ELT_SUB_FAM_EVT:
	case ELT_SUB_FAM_EVT_EVEN:
	case ELT_SUB_INDIV_ATTR:
	case ELT_SUB_INDIV_RESI:
	case ELT_SUB_INDIV_BIRT:
	case ELT_SUB_INDIV_GEN:
	case ELT_SUB_INDIV_ADOP:
	case ELT_SUB_INDIV_EVEN:
	  ADDFUNC2(event,note_sub)(ctxt, note); break;
	case ELT_SUB_SOUR:
	  ADDFUNC2(source_citation,note_sub)(ctxt, note); break;
	case ELT_SUB_MULTIM_OBJE:
	  ADDFUNC2(multimedia_link,note_sub)(ctxt, note); break;
	case ELT_SUB_LSS_SLGS:
	case ELT_SUB_LIO_BAPL:
	case ELT_SUB_LIO_SLGC:
	  ADDFUNC2(lds_event,note_sub)(ctxt, note); break;
	case REC_FAM:
	  ADDFUNC2(family,note_sub)(ctxt, note); break;
	case ELT_SUB_CHAN:
	  ADDFUNC2(change_date,note_sub)(ctxt, note); break;
	case ELT_SUB_PERS_NAME:
	  ADDFUNC2(personal_name,note_sub)(ctxt, note); break;
	case ELT_SUB_FAMC: 
	case ELT_SUB_FAMS:
	  ADDFUNC2(family_link,note_sub)(ctxt, note); break;
	case ELT_SUB_ASSO:
	  ADDFUNC2(association,note_sub)(ctxt, note); break;
	case REC_INDI:
	  ADDFUNC2(individual,note_sub)(ctxt, note); break;
	case REC_OBJE:
	  ADDFUNC2(multimedia,note_sub)(ctxt, note); break;
	case REC_REPO:
	  ADDFUNC2(repository,note_sub)(ctxt, note); break;
	case ELT_SOUR_DATA:
	  source_add_note_to_data(ctxt, note); break;
	case ELT_SUB_REPO:
	  source_add_note_to_repo(ctxt, note); break;
	case REC_SOUR:
	  ADDFUNC2(source,note_sub)(ctxt, note); break;
	default:
	  UNEXPECTED_CONTEXT(ctxt->ctxt_type);
      }
      result = MAKE_GOM_CTXT(elt, note_sub, note);
    }
  }

  return (Gedcom_ctxt)result;
}

DEFINE_SUB_MAKEFUNC(note_sub)
     
DEFINE_STRING_END_CB(note_sub, sub_note_end, text)

DEFINE_ADDFUNC2(note_sub, source_citation, citation)
DEFINE_ADDFUNC2(note_sub, user_data, extra)
     
void note_sub_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_NOTE, sub_note_start, sub_note_end);
}

void CLEANFUNC(note_sub)(struct note_sub* note)
{
  if (note) {
    SAFE_FREE(note->text);
    DESTROY_CHAIN_ELTS(source_citation, note->citation);
    DESTROY_CHAIN_ELTS(user_data, note->extra);
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
