/* Family link sub-structure in the gedcom object model.
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
#include "family_link.h"
#include "individual.h"
#include "note_sub.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_fam_link_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct family_link *link
      = (struct family_link *)malloc(sizeof(struct family_link));
    if (! link)
      MEMORY_ERROR;
    else {
      memset (link, 0, sizeof(struct family_link));
      link->family = GEDCOM_XREF_PTR(parsed_value);
      
      switch (ctxt->ctxt_type) {
	case REC_INDI:
	  individual_add_family_link(ctxt, elt, link); break;
	default:
	  UNEXPECTED_CONTEXT(ctxt->ctxt_type);
      }
      result = MAKE_GOM_CTXT(elt, family_link, link);
    }
  }

  return (Gedcom_ctxt)result;
}

Gedcom_ctxt sub_fam_link_pedi_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct family_link *link = SAFE_CTXT_CAST(family_link, ctxt);
    if (link) {
      int err = 0;
      struct pedigree *ped = NULL;
      MAKE_CHAIN_ELT(pedigree, link->pedigree, ped);
      if (ped) {
	ped->pedigree = strdup(GEDCOM_STRING(parsed_value));
	if (! ped->pedigree) {
	  MEMORY_ERROR;
	  err = 1;
	}
      }
      if (! err)
	result = MAKE_GOM_CTXT(elt, pedigree, ped);
    }
  }
  return (Gedcom_ctxt)result;
}

void family_link_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_FAMC, sub_fam_link_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_FAMS, sub_fam_link_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_FAMC_PEDI, sub_fam_link_pedi_start,
			      def_elt_end);
}

void family_link_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct family_link *link = SAFE_CTXT_CAST(family_link, ctxt);
  if (link)
    LINK_CHAIN_ELT(note_sub, link->note, note);
}

void family_link_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct family_link *obj = SAFE_CTXT_CAST(family_link, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void pedigree_cleanup(struct pedigree* ped)
{
  if (ped) {
    SAFE_FREE(ped->pedigree);
  }
}

void family_link_cleanup(struct family_link *link)
{
  if (link) {
    DESTROY_CHAIN_ELTS(pedigree, link->pedigree, pedigree_cleanup);
    DESTROY_CHAIN_ELTS(note_sub, link->note, note_sub_cleanup);
    DESTROY_CHAIN_ELTS(user_data, link->extra, user_data_cleanup);
  }
}
