/* Association sub-structure in the gedcom object model.
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
#include "association.h"
#include "individual.h"
#include "note_sub.h"
#include "source_citation.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_assoc_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (!ctxt)
    NO_CONTEXT;
  else {
    struct association *assoc;
    assoc = (struct association *)malloc(sizeof(struct association));
    if (! assoc)
      MEMORY_ERROR;
    else {
      memset (assoc, 0, sizeof(struct association));
      assoc->to = GEDCOM_XREF_PTR(parsed_value);
      
      switch (ctxt->ctxt_type) {
	case REC_INDI:
	  individual_add_association(ctxt, assoc);
	default:
	  UNEXPECTED_CONTEXT(ctxt->ctxt_type);
      }
      result = MAKE_GOM_CTXT(elt, association, assoc);
    }
  }

  return (Gedcom_ctxt)result;
}

STRING_CB(association, sub_assoc_rela_start, relation)
     
Gedcom_ctxt sub_assoc_type_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;
  
  if (! ctxt)
    NO_CONTEXT;
  else {
    struct association *obj = SAFE_CTXT_CAST(association, ctxt);
    char *str = GEDCOM_STRING(parsed_value);
    if (obj) {
      obj->type = strdup(str);
      if (! obj->type)
	MEMORY_ERROR;
      else {
	set_xref_type(obj->to, str);
	result = MAKE_GOM_CTXT(elt, association, obj);
      }
    }
  }
  return (Gedcom_ctxt)result;
}

void association_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_ASSO, sub_assoc_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_ASSO_TYPE, sub_assoc_type_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_ASSO_RELA, sub_assoc_rela_start,
			      def_elt_end);
}

void association_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct association *assoc = SAFE_CTXT_CAST(association, ctxt);
  if (assoc)
    LINK_CHAIN_ELT(note_sub, assoc->note, note);
}

void association_add_citation(Gom_ctxt ctxt, struct source_citation* cit)
{
  struct association *assoc = SAFE_CTXT_CAST(association, ctxt);
  if (assoc)
    LINK_CHAIN_ELT(source_citation, assoc->citation, cit);
}

void association_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct association *obj = SAFE_CTXT_CAST(association, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void association_cleanup(struct association* assoc)
{
  if (assoc) {
    SAFE_FREE(assoc->type);
    SAFE_FREE(assoc->relation);
    DESTROY_CHAIN_ELTS(note_sub, assoc->note, note_sub_cleanup);
    DESTROY_CHAIN_ELTS(source_citation, assoc->citation, citation_cleanup);
    DESTROY_CHAIN_ELTS(user_data, assoc->extra, user_data_cleanup);
  }
}

int write_associations(Gedcom_write_hndl hndl, int parent,
		       struct association *assoc)
{
  int result = 0;
  struct association* obj;

  if (!assoc) return 1;

  for (obj = assoc; obj; obj = obj->next) {
    result |= gedcom_write_element_xref(hndl, ELT_SUB_ASSO, 0, parent,
					obj->to);
    if (obj->type)
      result |= gedcom_write_element_str(hndl, ELT_SUB_ASSO_TYPE, 0, parent,
					 obj->type);
    if (obj->relation)
      result |= gedcom_write_element_str(hndl, ELT_SUB_ASSO_RELA, 0, parent,
					 obj->relation);
    if (obj->citation)
      result |= write_citations(hndl, ELT_SUB_ASSO, obj->citation);
    if (obj->note)
      result |= write_note_subs(hndl, ELT_SUB_ASSO, obj->note);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }

  return result;
}
