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
  struct place *place = NULL;

  if (ctxt) {
    place = (struct place *)malloc(sizeof(struct place));
    memset (place, 0, sizeof(struct place));
    place->value = strdup(GEDCOM_STRING(parsed_value));

    switch (ctxt->ctxt_type) {
      case ELT_SUB_FAM_EVT:
      case ELT_SUB_FAM_EVT_EVEN:
      case ELT_SUB_INDIV_ATTR:
      case ELT_SUB_INDIV_RESI:
      case ELT_SUB_INDIV_BIRT:
      case ELT_SUB_INDIV_GEN:
      case ELT_SUB_INDIV_ADOP:
      case ELT_SUB_INDIV_EVEN:
	event_add_place(ctxt, place); break;
      default:
	UNEXPECTED_CONTEXT(ctxt->ctxt_type);
    }
  }

  return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, place, place);
}

STRING_CB(place, sub_place_form_start, place_hierarchy)

void place_add_citation(Gom_ctxt ctxt, struct source_citation* cit)
{
  struct place *place = SAFE_CTXT_CAST(place, ctxt);
  LINK_CHAIN_ELT(source_citation, place->citation, cit)  
}

void place_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct place *place = SAFE_CTXT_CAST(place, ctxt);
  LINK_CHAIN_ELT(note_sub, place->note, note)  
}

void place_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct place *obj = SAFE_CTXT_CAST(place, ctxt);
  LINK_CHAIN_ELT(user_data, obj->extra, data)
}

void place_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_PLAC, sub_place_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_PLAC_FORM,
			      sub_place_form_start, def_elt_end);
}

void place_cleanup(struct place* place)
{
  if (place) {
    SAFE_FREE(place->value);
    SAFE_FREE(place->place_hierarchy);
    DESTROY_CHAIN_ELTS(source_citation, place->citation, citation_cleanup)  
    DESTROY_CHAIN_ELTS(note_sub, place->note, note_sub_cleanup)
    DESTROY_CHAIN_ELTS(user_data, place->extra, user_data_cleanup)
  }
  SAFE_FREE(place);
}
