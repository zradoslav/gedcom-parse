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
  struct user_ref_number *refn = NULL;

  if (ctxt) {
    refn = (struct user_ref_number *)malloc(sizeof(struct user_ref_number));
    memset (refn, 0, sizeof(struct user_ref_number));
    refn->value = strdup(GEDCOM_STRING(parsed_value));

    switch (ctxt->ctxt_type) {
      case REC_FAM:
	family_add_user_ref(ctxt, refn); break;
      case REC_INDI:
	individual_add_user_ref(ctxt, refn); break;
      case REC_OBJE:
	multimedia_add_user_ref(ctxt, refn); break;
      case REC_NOTE:
	note_add_user_ref(ctxt, refn); break;
      case REC_REPO:
	repository_add_user_ref(ctxt, refn); break;
      case REC_SOUR:
	source_add_user_ref(ctxt, refn); break;
      default:
	UNEXPECTED_CONTEXT(ctxt->ctxt_type);
    }
  }

  return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, user_ref_number, refn);
}

STRING_CB(user_ref_number, sub_user_ref_type_start, type)
     
Gedcom_ctxt sub_user_rin_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  if (ctxt) {
    char *str = GEDCOM_STRING(parsed_value);

    switch (ctxt->ctxt_type) {
      case REC_FAM:
	family_set_record_id(ctxt, str); break;
      case REC_INDI:
	individual_set_record_id(ctxt, str); break;
      case REC_OBJE:
	multimedia_set_record_id(ctxt, str); break;
      case REC_NOTE:
	note_set_record_id(ctxt, str); break;
      case REC_REPO:
	repository_set_record_id(ctxt, str); break;
      case REC_SOUR:
	source_set_record_id(ctxt, str); break;
      default:
	UNEXPECTED_CONTEXT(ctxt->ctxt_type);
    }
  }
  return (Gedcom_ctxt) make_gom_ctxt(elt, ctxt->obj_type, ctxt->ctxt_ptr);
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

void user_ref_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct user_ref_number *obj = SAFE_CTXT_CAST(user_ref_number, ctxt);
  LINK_CHAIN_ELT(user_data, obj->extra, data)
}

void user_ref_cleanup(struct user_ref_number* refn)
{
  if (refn) {
    SAFE_FREE(refn->value);
    SAFE_FREE(refn->type);
    DESTROY_CHAIN_ELTS(user_data, refn->extra, user_data_cleanup)
  }
}
