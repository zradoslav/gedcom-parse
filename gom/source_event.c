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
    struct source_event *evt = SUB_MAKEFUNC(source_event)();
    if (evt) {
      evt->recorded_events = strdup(GEDCOM_STRING(parsed_value));

      if (! evt->recorded_events) {
	MEMORY_ERROR;
	free(evt);
      }
      else {
	int type = ctxt_type(ctxt);
	switch (type) {
	  case ELT_SOUR_DATA:
	    ADDFUNC2(source,source_event)(ctxt, evt); break;
	  default:
	    UNEXPECTED_CONTEXT(type);
	}
	result = MAKE_GOM_CTXT(elt, source_event, evt);
      }
    }
  }

  return (Gedcom_ctxt)result;
}

DEFINE_SUB_MAKEFUNC(source_event)
DEFINE_SUB_ADDFUNC(source_event)
DEFINE_SUB_FINDFUNC(source_event)
DEFINE_SUB_REMOVEFUNC(source_event)
DEFINE_SUB_MOVEFUNC(source_event)
     
DEFINE_DATE_CB(source_event, sub_sour_even_date_start, date_period)
DEFINE_STRING_CB(source_event, sub_sour_even_plac_start, jurisdiction)

DEFINE_ADDFUNC2(source_event, user_data, extra)
     
void source_event_subscribe()
{
  gedcom_subscribe_to_element(ELT_SOUR_DATA_EVEN, sub_sour_even_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SOUR_DATA_EVEN_DATE,
			      sub_sour_even_date_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SOUR_DATA_EVEN_PLAC,
			      sub_sour_even_plac_start, def_elt_end);
}

void UNREFALLFUNC(source_event)(struct source_event* obj)
{
  if (obj) {
    struct source_event* runner;
    for (runner = obj; runner; runner = runner->next) {
      UNREFALLFUNC(user_data)(runner->extra);
    }
  }
}

void CLEANFUNC(source_event)(struct source_event* evt)
{
  if (evt) {
    SAFE_FREE(evt->recorded_events);
    SAFE_FREE(evt->date_period);
    SAFE_FREE(evt->jurisdiction);
    DESTROY_CHAIN_ELTS(user_data, evt->extra);
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
    if (obj->date_period)
      result |= gedcom_write_element_date(hndl, ELT_SOUR_DATA_EVEN_DATE, 0,
					 ELT_SOUR_DATA_EVEN, obj->date_period);
    if (obj->jurisdiction)
      result |= gedcom_write_element_str(hndl, ELT_SOUR_DATA_EVEN_PLAC, 0,
					ELT_SOUR_DATA_EVEN, obj->jurisdiction);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);    
  }

  return result;
}
