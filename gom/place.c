/* Place sub-structure in the gedcom object model.
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
#include "place.h"
#include "address.h"
#include "event.h"
#include "source_citation.h"
#include "note_sub.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_place_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct place *place = SUB_MAKEFUNC(place)();
    if (place) {
      place->value = strdup(GEDCOM_STRING(parsed_value));
      
      if (!place->value) {
	MEMORY_ERROR;
	free(place);
      }
      else {
	switch (ctxt->ctxt_type) {
	  case ELT_SUB_FAM_EVT:
	  case ELT_SUB_FAM_EVT_EVEN:
	  case ELT_SUB_INDIV_ATTR:
	  case ELT_SUB_INDIV_RESI:
	  case ELT_SUB_INDIV_BIRT:
	  case ELT_SUB_INDIV_GEN:
	  case ELT_SUB_INDIV_ADOP:
	  case ELT_SUB_INDIV_EVEN:
	    ADDFUNC2_NOLIST(event,place)(ctxt, place); break;
	  default:
	    UNEXPECTED_CONTEXT(ctxt->ctxt_type);
	}
	result = MAKE_GOM_CTXT(elt, place, place);
      }
    }
  }

  return (Gedcom_ctxt)result;
}

DEFINE_SUB_MAKEFUNC(place)
DEFINE_SUB_SETFUNC(place)
DEFINE_SUB_DELETEFUNC(place)
     
DEFINE_STRING_CB(place, sub_place_form_start, place_hierarchy)

DEFINE_ADDFUNC2(place, source_citation, citation)
DEFINE_ADDFUNC2(place, note_sub, note)
DEFINE_ADDFUNC2(place, user_data, extra)

void place_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_PLAC, sub_place_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_PLAC_FORM,
			      sub_place_form_start, def_elt_end);
}

void UNREFALLFUNC(place)(struct place* obj)
{
  if (obj) {
    UNREFALLFUNC(source_citation)(obj->citation);
    UNREFALLFUNC(note_sub)(obj->note);
    UNREFALLFUNC(user_data)(obj->extra);
  }
}

void CLEANFUNC(place)(struct place* place)
{
  if (place) {
    SAFE_FREE(place->value);
    SAFE_FREE(place->place_hierarchy);
    DESTROY_CHAIN_ELTS(source_citation, place->citation);  
    DESTROY_CHAIN_ELTS(note_sub, place->note);
    DESTROY_CHAIN_ELTS(user_data, place->extra);
  }
  SAFE_FREE(place);
}

int write_place(Gedcom_write_hndl hndl, int parent, struct place* place)
{
  int result = 0;
  
  if (!place) return 1;
  
  result |= gedcom_write_element_str(hndl, ELT_SUB_PLAC, 0,
				     parent, place->value);
  if (place->place_hierarchy)
    result |= gedcom_write_element_str(hndl, ELT_SUB_PLAC_FORM, 0,
				       ELT_SUB_PLAC, place->place_hierarchy);
  if (place->citation)
    result |= write_citations(hndl, ELT_SUB_PLAC, place->citation);
  if (place->note)
    result |= write_note_subs(hndl, ELT_SUB_PLAC, place->note);
  if (place->extra)
    result |= write_user_data(hndl, place->extra);

  return result;
}
