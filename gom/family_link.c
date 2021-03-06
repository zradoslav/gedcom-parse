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
    struct family_link *link = SUB_MAKEFUNC(family_link)();
    if (link) {
      int type = ctxt_type(ctxt);
      link->family = GEDCOM_XREF_PTR(parsed_value);
      
      switch (type) {
	case REC_INDI:
	  ADDFUNC2(individual,family_link)(ctxt, elt, link); break;
	default:
	  UNEXPECTED_CONTEXT(type);
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

DEFINE_SUB_MAKEFUNC(family_link)
DEFINE_SUB_ADDFUNC(family_link)
DEFINE_SUB_FINDFUNC(family_link)
DEFINE_SUB_REMOVEFUNC(family_link)
DEFINE_SUB_MOVEFUNC(family_link)
     
DEFINE_ADDFUNC2(family_link, note_sub, note)
DEFINE_ADDFUNC2(family_link, user_data, extra)

void family_link_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_FAMC, sub_fam_link_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_FAMS, sub_fam_link_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_FAMC_PEDI, sub_fam_link_pedi_start,
			      def_elt_end);
}

void UNREFALLFUNC(pedigree)(struct pedigree* obj)
{
  if (obj) {
    struct pedigree* runner;
    for (runner = obj; runner; runner = runner->next) {
      UNREFALLFUNC(user_data)(runner->extra);
    }
  }
}

void CLEANFUNC(pedigree)(struct pedigree* ped)
{
  if (ped) {
    SAFE_FREE(ped->pedigree);
  }
}

DEFINE_SUB_MAKEFUNC(pedigree)
DEFINE_SUB_ADDFUNC(pedigree)
DEFINE_SUB_FINDFUNC(pedigree)
DEFINE_SUB_REMOVEFUNC(pedigree)
DEFINE_SUB_MOVEFUNC(pedigree)
     
void UNREFALLFUNC(family_link)(struct family_link* obj)
{
  if (obj) {
    struct family_link* runner;
    for (runner = obj; runner; runner = runner->next) {
      unref_xref_value(runner->family);
      UNREFALLFUNC(pedigree)(runner->pedigree);
      UNREFALLFUNC(note_sub)(runner->note);
      UNREFALLFUNC(user_data)(runner->extra);
    }
  }
}

void CLEANFUNC(family_link)(struct family_link *link)
{
  if (link) {
    DESTROY_CHAIN_ELTS(pedigree, link->pedigree);
    DESTROY_CHAIN_ELTS(note_sub, link->note);
    DESTROY_CHAIN_ELTS(user_data, link->extra);
  }
}

int write_pedigrees(Gedcom_write_hndl hndl, int parent, struct pedigree* ped)
{
  int result = 0;
  struct pedigree* obj;

  if (!ped) return 1;

  for (obj = ped; obj; obj = obj->next) {
    result |= gedcom_write_element_str(hndl, ELT_SUB_FAMC_PEDI, 0, parent,
				       obj->pedigree);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);  
  }
  
  return result;
}

int write_family_links(Gedcom_write_hndl hndl, int parent, LinkType type,
		       struct family_link *link)
{
  int result = 0;
  struct family_link* obj;
  int elt = (type == LINK_TYPE_CHILD ? ELT_SUB_FAMC : ELT_SUB_FAMS);

  if (!link) return 1;

  for (obj = link; obj; obj = obj->next) {
    result |= gedcom_write_element_xref(hndl, elt, 0, parent,
					obj->family);
    if (obj->pedigree)
      result |= write_pedigrees(hndl, elt, obj->pedigree);
    if (obj->note)
      result |= write_note_subs(hndl, elt, obj->note);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);      
  }

  return result;
}
