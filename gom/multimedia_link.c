/* Multimedia link sub-structure in the gedcom object model.
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
#include "multimedia_link.h"
#include "event.h"
#include "source_citation.h"
#include "note_sub.h"
#include "family.h"
#include "individual.h"
#include "source.h"
#include "submitter.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

Gedcom_ctxt sub_obje_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct multimedia_link *mm = SUB_MAKEFUNC(multimedia_link)();
    if (mm) {
      int type = ctxt_type(ctxt);
      if (GEDCOM_IS_XREF_PTR(parsed_value))
	mm->reference = GEDCOM_XREF_PTR(parsed_value);
      
      switch (type) {
	case ELT_SUB_FAM_EVT:
	case ELT_SUB_FAM_EVT_EVEN:
	case ELT_SUB_INDIV_ATTR:
	case ELT_SUB_INDIV_RESI:
	case ELT_SUB_INDIV_BIRT:
	case ELT_SUB_INDIV_GEN:
	case ELT_SUB_INDIV_ADOP:
	case ELT_SUB_INDIV_EVEN:
	  ADDFUNC2(event,multimedia_link)(ctxt, mm); break;
	case ELT_SUB_SOUR:
	  ADDFUNC2(source_citation,multimedia_link)(ctxt, mm); break;
	case REC_FAM:
	  ADDFUNC2(family,multimedia_link)(ctxt, mm); break;
	case REC_INDI:
	  ADDFUNC2(individual,multimedia_link)(ctxt, mm); break;
	case REC_SOUR:
	  ADDFUNC2(source,multimedia_link)(ctxt, mm); break;
	case REC_SUBM:
	  ADDFUNC2(submitter,multimedia_link)(ctxt, mm); break;
	default:
	  UNEXPECTED_CONTEXT(type);
      }
      result = MAKE_GOM_CTXT(elt, multimedia_link, mm);
    }
  }

  return (Gedcom_ctxt)result;
}

DEFINE_SUB_MAKEFUNC(multimedia_link)
DEFINE_SUB_ADDFUNC(multimedia_link)
DEFINE_SUB_FINDFUNC(multimedia_link)
DEFINE_SUB_REMOVEFUNC(multimedia_link)
DEFINE_SUB_MOVEFUNC(multimedia_link)
     
DEFINE_STRING_CB(multimedia_link, sub_obje_form_start, form)
DEFINE_STRING_CB(multimedia_link, sub_obje_titl_start, title)
DEFINE_STRING_CB(multimedia_link, sub_obje_file_start, file)

DEFINE_ADDFUNC2(multimedia_link, note_sub, note)
DEFINE_ADDFUNC2(multimedia_link, user_data, extra)
     
void multimedia_link_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_MULTIM_OBJE,
			      sub_obje_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_MULTIM_OBJE_FORM,
			      sub_obje_form_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_MULTIM_OBJE_TITL,
			      sub_obje_titl_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_MULTIM_OBJE_FILE,
			      sub_obje_file_start, def_elt_end);
}

void UNREFALLFUNC(multimedia_link)(struct multimedia_link* obj)
{
  if (obj) {
    struct multimedia_link* runner;
    for (runner = obj; runner; runner = runner->next) {
      unref_xref_value(runner->reference);
      UNREFALLFUNC(note_sub)(runner->note);
      UNREFALLFUNC(user_data)(runner->extra);
    }
  }
}

void CLEANFUNC(multimedia_link)(struct multimedia_link* mm)
{
  if (mm) {
    SAFE_FREE(mm->form);
    SAFE_FREE(mm->title);
    SAFE_FREE(mm->file);
    DESTROY_CHAIN_ELTS(note_sub, mm->note);
    DESTROY_CHAIN_ELTS(user_data, mm->extra);
  }
}

int write_multimedia_links(Gedcom_write_hndl hndl, int parent,
			   struct multimedia_link* mm)
{
  int result = 0;
  struct multimedia_link* obj;

  if (!mm) return 1;

  for (obj = mm; obj; obj = obj->next) {
    if (obj->reference) {
      result |= gedcom_write_element_xref(hndl, ELT_SUB_MULTIM_OBJE, 0,
					  parent, obj->reference);
    }
    else {
      result |= gedcom_write_element_str(hndl, ELT_SUB_MULTIM_OBJE, 0,
					 parent, NULL);
      if (obj->form)
	result |= gedcom_write_element_str(hndl, ELT_SUB_MULTIM_OBJE_FORM, 0,
					   ELT_SUB_MULTIM_OBJE, obj->form);
      if (obj->title)
	result |= gedcom_write_element_str(hndl, ELT_SUB_MULTIM_OBJE_TITL, 0,
					   ELT_SUB_MULTIM_OBJE, obj->title);
      if (obj->file)
	result |= gedcom_write_element_str(hndl, ELT_SUB_MULTIM_OBJE_FILE, 0,
					   ELT_SUB_MULTIM_OBJE, obj->file);
      if (obj->note)
	result |= write_note_subs(hndl, ELT_SUB_MULTIM_OBJE, obj->note);
      if (obj->extra)
	result |= write_user_data(hndl, obj->extra);
    }
  }

  return result;
}
