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
  struct personal_name *name = NULL;

  if (ctxt) {
    name = (struct personal_name *)malloc(sizeof(struct personal_name));
    memset (name, 0, sizeof(struct personal_name));
    name->name = strdup(GEDCOM_STRING(parsed_value));

    switch (ctxt->ctxt_type) {
      case REC_INDI:
	individual_add_name(ctxt, name); break;
      default:
	UNEXPECTED_CONTEXT(ctxt->ctxt_type);
    }
  }

  return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, personal_name, name);
}

STRING_CB(personal_name, sub_name_npfx_start, prefix)
STRING_CB(personal_name, sub_name_givn_start, given)
STRING_CB(personal_name, sub_name_nick_start, nickname)
STRING_CB(personal_name, sub_name_spfx_start, surname_prefix)
STRING_CB(personal_name, sub_name_surn_start, surname)
STRING_CB(personal_name, sub_name_nsfx_start, suffix)

void name_add_citation(Gom_ctxt ctxt, struct source_citation* cit)
{
  struct personal_name *name = SAFE_CTXT_CAST(personal_name, ctxt);
  LINK_CHAIN_ELT(source_citation, name->citation, cit)  
}

void name_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct personal_name *name = SAFE_CTXT_CAST(personal_name, ctxt);
  LINK_CHAIN_ELT(note_sub, name->note, note)  
}

void name_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct personal_name *obj = SAFE_CTXT_CAST(personal_name, ctxt);
  LINK_CHAIN_ELT(user_data, obj->extra, data)
}

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

void name_cleanup(struct personal_name* name)
{
  if (name) {
    SAFE_FREE(name->name);
    SAFE_FREE(name->prefix);
    SAFE_FREE(name->given);
    SAFE_FREE(name->nickname);
    SAFE_FREE(name->surname_prefix);
    SAFE_FREE(name->surname);
    SAFE_FREE(name->suffix);
    DESTROY_CHAIN_ELTS(source_citation, name->citation, citation_cleanup)
    DESTROY_CHAIN_ELTS(note_sub, name->note, note_sub_cleanup)
    DESTROY_CHAIN_ELTS(user_data, name->extra, user_data_cleanup)
  }
}
