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
    struct association *assoc = SUB_MAKEFUNC(association)();
    if (assoc) {
      int type = ctxt_type(ctxt);
      assoc->to = GEDCOM_XREF_PTR(parsed_value);

      switch (type) {
	case REC_INDI:
	  ADDFUNC2(individual,association)(ctxt, assoc);
	default:
	  UNEXPECTED_CONTEXT(type);
      }
      result = MAKE_GOM_CTXT(elt, association, assoc);
    }
  }

  return (Gedcom_ctxt)result;
}

DEFINE_SUB_MAKEFUNC(association)
DEFINE_SUB_ADDFUNC(association)
DEFINE_SUB_FINDFUNC(association)
DEFINE_SUB_REMOVEFUNC(association)
DEFINE_SUB_MOVEFUNC(association)
     
DEFINE_STRING_CB(association, sub_assoc_rela_start, relation)

DEFINE_ADDFUNC2(association, note_sub, note)
DEFINE_ADDFUNC2(association, source_citation, citation)
DEFINE_ADDFUNC2(association, user_data, extra)
     
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

void UNREFALLFUNC(association)(struct association *obj)
{
  if (obj) {
    struct association* runner;
    for (runner = obj; runner; runner = runner->next) {
      unref_xref_value(runner->to);
      UNREFALLFUNC(source_citation)(runner->citation);
      UNREFALLFUNC(note_sub)(runner->note);
      UNREFALLFUNC(user_data)(runner->extra);
    }
  }
}

void CLEANFUNC(association)(struct association* assoc)
{
  if (assoc) {
    SAFE_FREE(assoc->type);
    SAFE_FREE(assoc->relation);
    DESTROY_CHAIN_ELTS(note_sub, assoc->note);
    DESTROY_CHAIN_ELTS(source_citation, assoc->citation);
    DESTROY_CHAIN_ELTS(user_data, assoc->extra);
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
