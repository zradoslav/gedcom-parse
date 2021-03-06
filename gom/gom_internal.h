/* General header for the Gedcom object model.
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

#ifndef __GOM_INTERNAL_H
#define __GOM_INTERNAL_H

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "gom.h"
#include "gedcom.h"
      
#ifdef ENABLE_NLS
#include <libintl.h>

#define _(string) dgettext(PACKAGE, string)
#define N_(string) (string)
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

typedef enum {
  T_NULL,
  
  T_header, T_submission, T_submitter, T_family, T_individual,
  T_multimedia, T_note, T_repository, T_source, T_user_rec,
  
  T_address, T_event, T_place, T_source_citation, T_text,
  T_note_sub, T_multimedia_link, T_lds_event, T_user_ref_number,
  T_change_date, T_personal_name, T_family_link, T_pedigree,
  T_association, T_source_event, T_source_description,

  T_LAST
} OBJ_TYPE;

struct Gom_ctxt_struct;
typedef struct Gom_ctxt_struct *Gom_ctxt;

Gom_ctxt make_gom_ctxt(int ctxt_type, OBJ_TYPE obj_type, void *ctxt_ptr);
Gom_ctxt dup_gom_ctxt(Gom_ctxt ctxt, int ctxt_type);
void* safe_ctxt_cast(Gom_ctxt ctxt, OBJ_TYPE type, const char* file, int line);
int ctxt_type(Gom_ctxt ctxt);
OBJ_TYPE ctxt_obj_type(Gom_ctxt ctxt);

void gom_cast_error(const char* file, int line,
		    OBJ_TYPE expected, OBJ_TYPE found);
void gom_no_context(const char* file, int line);
void gom_unexpected_context(const char* file, int line, OBJ_TYPE found);
void gom_xref_already_in_use(const char *xrefstr);
void gom_move_error(const char* type);
void gom_find_error(const char* type);
void unref_xref_value(struct xref_value *xref);

int gom_write_xref_list(Gedcom_write_hndl hndl,
			Gedcom_elt elt, int tag, int parent_rec_or_elt,
			struct xref_list* val);

#define MAKE_GOM_CTXT(CTXT_TYPE, STRUCTTYPE, CTXT_PTR)                        \
  make_gom_ctxt(CTXT_TYPE, T_ ## STRUCTTYPE, CTXT_PTR)

#define SAFE_CTXT_CAST(STRUCTTYPE, VAL)                                       \
  safe_ctxt_cast(VAL, T_ ## STRUCTTYPE, __FILE__, __LINE__)

#define SAFE_FREE(PTR)                                                        \
  if (PTR) {                                                                  \
    free(PTR);                                                                \
    PTR = NULL;                                                               \
  }

#define UNEXPECTED_CONTEXT(CTXT_TYPE)                                         \
  gom_unexpected_context(__FILE__, __LINE__, CTXT_TYPE)

#define NO_CONTEXT                                                            \
  gom_no_context(__FILE__, __LINE__)

void gom_mem_error(const char *filename, int line);

#define MEMORY_ERROR gom_mem_error(__FILE__, __LINE__)

void def_rec_end(Gedcom_rec rec, Gedcom_ctxt self, Gedcom_val parsed_value);
void def_elt_end(Gedcom_elt elt, Gedcom_ctxt parent,
		 Gedcom_ctxt self, Gedcom_val parsed_value);
void set_xref_type(struct xref_value *xr, const char* str);

int  update_date(struct date_value** dv, struct tm* tm_ptr);
int  update_time(char** tv, struct tm* tm_ptr);

void NULL_DESTROY(void* anything);

#include "func_template.h"

DECLARE_UNREFALLFUNC(xref_list);
DECLARE_CLEANFUNC(xref_list);
  
#endif /* __GOM_INTERNAL_H */
