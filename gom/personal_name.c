/* Personal name sub-structure in the gedcom object model.
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
#include "personal_name.h"
#include "source_citation.h"
#include "note_sub.h"
#include "individual.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_name_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (ctxt) {
    struct personal_name *name = SUB_MAKEFUNC(personal_name)();
    if (name) {
      name->name = strdup(GEDCOM_STRING(parsed_value));

      if (! name->name) {
	MEMORY_ERROR;
	free(name);
      }
      else {
	switch (ctxt->ctxt_type) {
	  case REC_INDI:
	    ADDFUNC2(individual,personal_name)(ctxt, name); break;
	  default:
	    UNEXPECTED_CONTEXT(ctxt->ctxt_type);
	}
	result = MAKE_GOM_CTXT(elt, personal_name, name);
      }
    }
  }

  return (Gedcom_ctxt)result;
}

DEFINE_SUB_MAKEFUNC(personal_name)
DEFINE_SUB_ADDFUNC(personal_name)
DEFINE_SUB_FINDFUNC(personal_name)
DEFINE_SUB_REMOVEFUNC(personal_name)
DEFINE_SUB_MOVEFUNC(personal_name)
     
DEFINE_STRING_CB(personal_name, sub_name_npfx_start, prefix)
DEFINE_STRING_CB(personal_name, sub_name_givn_start, given)
DEFINE_STRING_CB(personal_name, sub_name_nick_start, nickname)
DEFINE_STRING_CB(personal_name, sub_name_spfx_start, surname_prefix)
DEFINE_STRING_CB(personal_name, sub_name_surn_start, surname)
DEFINE_STRING_CB(personal_name, sub_name_nsfx_start, suffix)

DEFINE_ADDFUNC2(personal_name, source_citation, citation)
DEFINE_ADDFUNC2(personal_name, note_sub, note)
DEFINE_ADDFUNC2(personal_name, user_data, extra)

void name_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_PERS_NAME, sub_name_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_PERS_NAME_NPFX, sub_name_npfx_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_PERS_NAME_GIVN, sub_name_givn_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_PERS_NAME_NICK, sub_name_nick_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_PERS_NAME_SPFX, sub_name_spfx_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_PERS_NAME_SURN, sub_name_surn_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_PERS_NAME_NSFX, sub_name_nsfx_start,
			      def_elt_end);
}

void UNREFALLFUNC(personal_name)(struct personal_name* obj)
{
  if (obj) {
    struct personal_name* runner;
    for (runner = obj; runner; runner = runner->next) {
      UNREFALLFUNC(source_citation)(runner->citation);
      UNREFALLFUNC(note_sub)(runner->note);
      UNREFALLFUNC(user_data)(runner->extra);
    }
  }
}

void CLEANFUNC(personal_name)(struct personal_name* name)
{
  if (name) {
    SAFE_FREE(name->name);
    SAFE_FREE(name->prefix);
    SAFE_FREE(name->given);
    SAFE_FREE(name->nickname);
    SAFE_FREE(name->surname_prefix);
    SAFE_FREE(name->surname);
    SAFE_FREE(name->suffix);
    DESTROY_CHAIN_ELTS(source_citation, name->citation);
    DESTROY_CHAIN_ELTS(note_sub, name->note);
    DESTROY_CHAIN_ELTS(user_data, name->extra);
  }
}

int write_names(Gedcom_write_hndl hndl, int parent,
		struct personal_name *name)
{
  int result = 0;
  struct personal_name* obj;

  if (!name) return 1;

  for (obj = name; obj; obj = obj->next) {
    result |= gedcom_write_element_str(hndl, ELT_SUB_PERS_NAME, 0,
				       parent, obj->name);
    if (obj->prefix)
      result |= gedcom_write_element_str(hndl, ELT_SUB_PERS_NAME_NPFX, 0,
					 ELT_SUB_PERS_NAME, obj->prefix);
    if (obj->given)
      result |= gedcom_write_element_str(hndl, ELT_SUB_PERS_NAME_GIVN, 0,
					 ELT_SUB_PERS_NAME, obj->given);
    if (obj->nickname)
      result |= gedcom_write_element_str(hndl, ELT_SUB_PERS_NAME_NICK, 0,
					 ELT_SUB_PERS_NAME, obj->nickname);
    if (obj->surname_prefix)
      result |= gedcom_write_element_str(hndl, ELT_SUB_PERS_NAME_SPFX, 0,
					 ELT_SUB_PERS_NAME,
					 obj->surname_prefix);
    if (obj->surname)
      result |= gedcom_write_element_str(hndl, ELT_SUB_PERS_NAME_SURN, 0,
					 ELT_SUB_PERS_NAME, obj->surname);
    if (obj->suffix)
      result |= gedcom_write_element_str(hndl, ELT_SUB_PERS_NAME_NSFX, 0,
					 ELT_SUB_PERS_NAME, obj->suffix);
    if (obj->citation)
      result |= write_citations(hndl, ELT_SUB_PERS_NAME, obj->citation);
    if (obj->note)
      result |= write_note_subs(hndl, ELT_SUB_PERS_NAME, obj->note);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }

  return result;
}
