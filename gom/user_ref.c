/* User reference number sub-structure in the gedcom object model.
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
#include "user_ref.h"
#include "family.h"
#include "individual.h"
#include "multimedia.h"
#include "note.h"
#include "repository.h"
#include "source.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_user_ref_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct user_ref_number *refn = SUB_MAKEFUNC(user_ref_number)();
    if (refn) {
      refn->value = strdup(GEDCOM_STRING(parsed_value));
      if (! refn->value) {
	MEMORY_ERROR;
	free(refn);
      }
      else {
	switch (ctxt->ctxt_type) {
	  case REC_FAM:
	    ADDFUNC2(family,user_ref_number)(ctxt, refn); break;
	  case REC_INDI:
	    ADDFUNC2(individual,user_ref_number)(ctxt, refn); break;
	  case REC_OBJE:
	    ADDFUNC2(multimedia,user_ref_number)(ctxt, refn); break;
	  case REC_NOTE:
	    ADDFUNC2(note,user_ref_number)(ctxt, refn); break;
	  case REC_REPO:
	    ADDFUNC2(repository,user_ref_number)(ctxt, refn); break;
	  case REC_SOUR:
	    ADDFUNC2(source,user_ref_number)(ctxt, refn); break;
	  default:
	    UNEXPECTED_CONTEXT(ctxt->ctxt_type);
	}
	
	result = MAKE_GOM_CTXT(elt, user_ref_number, refn);
      }
    }
  }

  return (Gedcom_ctxt)result;
}

DEFINE_SUB_MAKEFUNC(user_ref_number)
DEFINE_SUB_ADDFUNC(user_ref_number)
DEFINE_SUB_FINDFUNC(user_ref_number)
DEFINE_SUB_REMOVEFUNC(user_ref_number)
DEFINE_SUB_MOVEFUNC(user_ref_number)
     
DEFINE_STRING_CB(user_ref_number, sub_user_ref_type_start, type)

DEFINE_ADDFUNC2(user_ref_number, user_data, extra)
     
Gedcom_ctxt sub_user_rin_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;
  
  if (! ctxt)
    NO_CONTEXT;
  else {
    char *str = GEDCOM_STRING(parsed_value);

    switch (ctxt->ctxt_type) {
      case REC_FAM:
	ADDFUNC2_STR(family,record_id)(ctxt, str); break;
      case REC_INDI:
	ADDFUNC2_STR(individual,record_id)(ctxt, str); break;
      case REC_OBJE:
	ADDFUNC2_STR(multimedia,record_id)(ctxt, str); break;
      case REC_NOTE:
	ADDFUNC2_STR(note,record_id)(ctxt, str); break;
      case REC_REPO:
	ADDFUNC2_STR(repository,record_id)(ctxt, str); break;
      case REC_SOUR:
	ADDFUNC2_STR(source,record_id)(ctxt, str); break;
      default:
	UNEXPECTED_CONTEXT(ctxt->ctxt_type);
    }
    result = make_gom_ctxt(elt, ctxt->obj_type, ctxt->ctxt_ptr);
  }
  return (Gedcom_ctxt)result;
}

void user_ref_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_IDENT_REFN, sub_user_ref_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_IDENT_REFN_TYPE, sub_user_ref_type_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_IDENT_RIN, sub_user_rin_start,
			      def_elt_end);
}

void UNREFALLFUNC(user_ref_number)(struct user_ref_number* obj)
{
  if (obj) {
    struct user_ref_number* runner;
    for (runner = obj; runner; runner = runner->next)
      UNREFALLFUNC(user_data)(runner->extra);
  }
}

void CLEANFUNC(user_ref_number)(struct user_ref_number* refn)
{
  if (refn) {
    SAFE_FREE(refn->value);
    SAFE_FREE(refn->type);
    DESTROY_CHAIN_ELTS(user_data, refn->extra);
  }
}

int write_user_refs(Gedcom_write_hndl hndl, int parent,
		    struct user_ref_number *refn)
{
  int result = 0;
  struct user_ref_number* obj;

  if (!refn) return 1;

  for (obj = refn; obj; obj = obj->next) {
    result |= gedcom_write_element_str(hndl, ELT_SUB_IDENT_REFN, 0,
				       parent, obj->value);
    if (obj->type)
      result |= gedcom_write_element_str(hndl, ELT_SUB_IDENT_REFN_TYPE, 0,
					 ELT_SUB_IDENT_REFN, obj->type);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);    
  }

  return result;
}
