/* Internals for building the gedcom object model.
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

#include "gom.h"
#include "gom_internal.h"

const char* ctxt_names[] =
{
  "NULL",
  
  "header", "submission", "submitter", "family", "individual",
  "multimedia", "note", "repository", "source", "user_rec",
  
  "address", "event", "place", "source_citation", "text",
  "note_sub", "multimedia_link", "lds_event", "user_ref_number",
  "change_date", "personal_name", "family_link", "pedigree",
  "association", "source_event", "source_description"
};

/* Assumptions for context:
    - In case of error, NULL is passed as context
    - If not NULL, the ctxt_ptr of the context is not NULL also
    - UNEXPECTED_CONTEXT is not treated as an error, but as a warning

   The context chain keeps contexts until the end of the record, so that
   elements out of context can be handled.
*/

struct Gom_ctxt_struct {
  int ctxt_type;
  OBJ_TYPE obj_type;
  void* ctxt_ptr;
  struct Gom_ctxt_struct* next;
};

struct Gom_ctxt_struct* ctxt_chain = NULL;

Gom_ctxt make_gom_ctxt(int ctxt_type, OBJ_TYPE obj_type, void *ctxt_ptr)
{
  Gom_ctxt ctxt   = (Gom_ctxt)malloc(sizeof(struct Gom_ctxt_struct));
  if (! ctxt)
    MEMORY_ERROR;
  else {
    ctxt->ctxt_type = ctxt_type;
    ctxt->obj_type  = obj_type;
    ctxt->ctxt_ptr  = ctxt_ptr;
    ctxt->next      = ctxt_chain;
    ctxt_chain      = ctxt;
  }
  return ctxt;
}

Gom_ctxt dup_gom_ctxt(Gom_ctxt ctxt, int ctxt_type)
{
  return make_gom_ctxt(ctxt_type, ctxt->obj_type, ctxt->ctxt_ptr);
}

int ctxt_type(Gom_ctxt ctxt)
{
  return ctxt->ctxt_type;
}

OBJ_TYPE ctxt_obj_type(Gom_ctxt ctxt)
{
  return ctxt->obj_type;
}

void* safe_ctxt_cast(Gom_ctxt ctxt, OBJ_TYPE type, const char* file, int line)
{
  if (ctxt->obj_type != type) {
    gom_cast_error(file, line, type, ctxt->obj_type);
  }
  return ctxt->ctxt_ptr;
}

void NULL_DESTROY(void* anything UNUSED)
{
}

void destroy_gom_ctxt(Gom_ctxt ctxt)
{
  SAFE_FREE(ctxt);
}

void gom_cast_error(const char* file, int line,
		    OBJ_TYPE expected, OBJ_TYPE found)
{
  const char* expected_name = "<out-of-bounds>";
  const char* found_name    = "<out-of-bounds>";
  if (expected < T_LAST)
    expected_name = ctxt_names[expected];
  if (found < T_LAST)
    found_name = ctxt_names[found];
  fprintf(stderr,
	  "Wrong gom ctxt cast at %s, line %d: expected %s, found %s\n",
	  file, line, expected_name, found_name);
  abort();
}

void gom_unexpected_context(const char* file, int line, OBJ_TYPE found)
{
  const char* found_name    = "<out-of-bounds>";
  if (found < T_LAST)
    found_name = ctxt_names[found];
  gedcom_warning(_("Internal error: Unexpected context at %s, line %d: %s"),
		 file, line, found_name);
}

void gom_no_context(const char* file, int line)
{
  gedcom_warning(_("Internal error: No context at %s, line %d"),
		 file, line);
}

void def_rec_end(Gedcom_rec rec UNUSED, Gedcom_ctxt self UNUSED,
		 Gedcom_val parsed_value UNUSED)
{
  Gom_ctxt ctxt;
  while (ctxt_chain) {
    ctxt = ctxt_chain;
    ctxt_chain = ctxt->next;
    destroy_gom_ctxt(ctxt);
  }
}

void def_elt_end(Gedcom_elt elt UNUSED, Gedcom_ctxt parent UNUSED,
		 Gedcom_ctxt self UNUSED, Gedcom_val parsed_value UNUSED)
{
}
