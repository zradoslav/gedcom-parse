/* Source event sub-structure in the gedcom object model.
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
#include "source_event.h"
#include "source.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_sour_even_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct source_event *evt
      = (struct source_event *)malloc(sizeof(struct source_event));
    if (! evt)
      MEMORY_ERROR;
    else {
      memset (evt, 0, sizeof(struct source_event));
      evt->recorded_events = strdup(GEDCOM_STRING(parsed_value));

      if (! evt->recorded_events) {
	MEMORY_ERROR;
	free(evt);
      }
      else {
	switch (ctxt->ctxt_type) {
	  case ELT_SOUR_DATA:
	    source_add_event(ctxt, evt); break;
	  default:
	    UNEXPECTED_CONTEXT(ctxt->ctxt_type);
	}
	result = MAKE_GOM_CTXT(elt, source_event, evt);
      }
    }
  }

  return (Gedcom_ctxt)result;
}

DATE_CB(source_event, sub_sour_even_date_start, date_period)
STRING_CB(source_event, sub_sour_even_plac_start, jurisdiction)
     
void source_event_subscribe()
{
  gedcom_subscribe_to_element(ELT_SOUR_DATA_EVEN, sub_sour_even_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SOUR_DATA_EVEN_DATE,
			      sub_sour_even_date_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SOUR_DATA_EVEN_PLAC,
			      sub_sour_even_plac_start, def_elt_end);
}

void source_event_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct source_event *obj = SAFE_CTXT_CAST(source_event, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void source_event_cleanup(struct source_event* evt)
{
  if (evt) {
    SAFE_FREE(evt->recorded_events);
    SAFE_FREE(evt->date_period);
    SAFE_FREE(evt->jurisdiction);
    DESTROY_CHAIN_ELTS(user_data, evt->extra, user_data_cleanup);
  }
}

int write_source_events(Gedcom_write_hndl hndl, int parent,
			struct source_event *evt)
{
  int result = 0;
  struct source_event* obj;

  if (!evt) return 1;

  for (obj = evt; obj; obj = obj->next) {
    result |= gedcom_write_element_str(hndl, ELT_SOUR_DATA_EVEN, 0,
				       parent, obj->recorded_events);
    if (obj->jurisdiction)
      result |= gedcom_write_element_str(hndl, ELT_SOUR_DATA_EVEN_PLAC, 0,
					ELT_SOUR_DATA_EVEN, obj->jurisdiction);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);    
  }

  return result;
}
