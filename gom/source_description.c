/* Source description sub-structure in the gedcom object model.
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
#include "source_description.h"
#include "source.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_sour_caln_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct source_description *desc
      = (struct source_description *)malloc(sizeof(struct source_description));
    if (! desc)
      MEMORY_ERROR;
    else {
      memset (desc, 0, sizeof(struct source_description));
      desc->call_number = strdup(GEDCOM_STRING(parsed_value));

      if (! desc->call_number) {
	MEMORY_ERROR;
	free(desc);
      }
      else {
	switch (ctxt->ctxt_type) {
	  case ELT_SUB_REPO:
	    source_add_description(ctxt, desc); break;
	  default:
	    UNEXPECTED_CONTEXT(ctxt->ctxt_type);
	}
	result = MAKE_GOM_CTXT(elt, source_description, desc);
      }
    }
  }

  return (Gedcom_ctxt)result;
}

STRING_CB(source_description, sub_sour_caln_medi_start, media)
     
void source_description_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_REPO_CALN, sub_sour_caln_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_REPO_CALN_MEDI,
			      sub_sour_caln_medi_start, def_elt_end);
}

void source_description_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct source_description *obj = SAFE_CTXT_CAST(source_description, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void source_description_cleanup(struct source_description* desc)
{
  if (desc) {
    SAFE_FREE(desc->call_number);
    SAFE_FREE(desc->media);
    DESTROY_CHAIN_ELTS(user_data, desc->extra, user_data_cleanup);
  }
}

int write_source_descriptions(Gedcom_write_hndl hndl, int parent,
			      struct source_description *desc)
{
  int result = 0;
  struct source_description* obj;

  if (!desc) return 1;

  for (obj = desc; obj; obj = obj->next) {
    result |= gedcom_write_element_str(hndl, ELT_SUB_REPO_CALN, 0,
				       parent, obj->call_number);
    if (obj->media)
      result |= gedcom_write_element_str(hndl, ELT_SUB_REPO_CALN_MEDI, 0,
					 ELT_SUB_REPO_CALN, obj->media);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);    
  }

  return result;
}
