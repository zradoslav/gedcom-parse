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
#include <libintl.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "gom.h"
#include "gedcom.h"
      
#define _(string) dgettext(PACKAGE, string)
#define N_(string) (string)

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
  T_association, T_source_event, T_source_description
} OBJ_TYPE;

/* Assumptions for context:
    - In case of error, NULL is passed as context
    - If not NULL, the ctxt_ptr of the context is not NULL also
    - UNEXPECTED_CONTEXT is not treated as an error, but as a warning
*/

struct Gom_ctxt_struct {
  int ctxt_type;
  OBJ_TYPE obj_type;
  void* ctxt_ptr;
};

typedef struct Gom_ctxt_struct *Gom_ctxt;

Gom_ctxt make_gom_ctxt(int ctxt_type, OBJ_TYPE obj_type, void *ctxt_ptr);
void destroy_gom_ctxt(Gom_ctxt ctxt);
void gom_cast_error(const char* file, int line,
		    OBJ_TYPE expected, OBJ_TYPE found);
void gom_no_context(const char* file, int line);
void gom_unexpected_context(const char* file, int line, OBJ_TYPE found);

int gom_write_xref_list(Gedcom_write_hndl hndl,
			Gedcom_elt elt, int tag, int parent_rec_or_elt,
			struct xref_list* val);

#define MAKE_GOM_CTXT(CTXT_TYPE, STRUCTTYPE, CTXT_PTR)                        \
  make_gom_ctxt(CTXT_TYPE, T_ ## STRUCTTYPE, CTXT_PTR)

#define SAFE_CTXT_CAST(STRUCTTYPE, VAL)                                       \
  (((VAL)->obj_type == T_ ## STRUCTTYPE) ?                                    \
   (VAL)->ctxt_ptr :                                                          \
   (gom_cast_error(__FILE__, __LINE__, T_ ## STRUCTTYPE, (VAL)->obj_type),    \
    (VAL)->ctxt_ptr))

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

struct date_value* dup_date(struct date_value dv);
struct age_value*  dup_age(struct age_value age);

/* Doubly-linked list, but last rec->next is NULL (doesn't go to first rec) */
#define LINK_CHAIN_ELT(STRUCTTYPE, FIRSTVAL, VAL)                             \
  {                                                                           \
    struct STRUCTTYPE *_local_obj = VAL;                                      \
    if (! FIRSTVAL) {                                                         \
      VAL->next = NULL;                                                       \
      VAL->previous = _local_obj;                                             \
      FIRSTVAL = VAL;                                                         \
    }                                                                         \
    else {                                                                    \
      VAL->next = NULL;                                                       \
      FIRSTVAL->previous->next = VAL;                                         \
      VAL->previous = FIRSTVAL->previous;                                     \
      FIRSTVAL->previous = VAL;                                               \
    }                                                                         \
  }

#define MAKE_CHAIN_ELT(STRUCTTYPE, FIRSTVAL, VAL)                             \
  {                                                                           \
    VAL = (struct STRUCTTYPE*) malloc(sizeof(struct STRUCTTYPE));             \
    if (! VAL)                                                                \
      MEMORY_ERROR;                                                           \
    else {                                                                    \
      memset (VAL, 0, sizeof(struct STRUCTTYPE));                             \
      LINK_CHAIN_ELT(STRUCTTYPE, FIRSTVAL, VAL)                               \
    }                                                                         \
  }

void NULL_DESTROY(void* anything);

#define DESTROY_CHAIN_ELTS(STRUCTTYPE, FIRSTVAL, DESTROYFUNC)                 \
  {                                                                           \
    if (FIRSTVAL) {                                                           \
      struct STRUCTTYPE *runner, *next;                                       \
      runner = FIRSTVAL;                                                      \
      while (runner) {                                                        \
	next = runner->next;                                                  \
        DESTROYFUNC(runner);                                                  \
        SAFE_FREE(runner);                                                    \
	runner = next;                                                        \
      }                                                                       \
    }                                                                         \
  }

#define _REC_PARAMS_ Gedcom_rec rec UNUSED, int level UNUSED,                 \
                     Gedcom_val xref UNUSED, char *tag UNUSED,                \
                     char *raw_value UNUSED, int parsed_tag UNUSED,           \
                     Gedcom_val parsed_value UNUSED

#define _REC_END_PARAMS_ Gedcom_rec rec UNUSED, Gedcom_ctxt self UNUSED,      \
                         Gedcom_val parsed_value UNUSED

#define _ELT_PARAMS_ Gedcom_elt elt UNUSED, Gedcom_ctxt parent UNUSED,        \
                     int level UNUSED, char *tag UNUSED,                      \
                     char *raw_value UNUSED, int parsed_tag UNUSED,           \
                     Gedcom_val parsed_value UNUSED

#define _ELT_END_PARAMS_ Gedcom_elt elt UNUSED, Gedcom_ctxt parent UNUSED,    \
                         Gedcom_ctxt self UNUSED,                             \
                         Gedcom_val parsed_value UNUSED

#define REC_CB(STRUCTTYPE,CB_NAME,FUNC)                                       \
  Gedcom_ctxt CB_NAME(_REC_PARAMS_)                                           \
  {                                                                           \
    struct xref_value* xr = GEDCOM_XREF_PTR(xref);                            \
    if (! xr->object)                                                         \
      xr->object = (Gedcom_ctxt) FUNC(xr->string);                            \
    if (xr->object)                                                           \
      return (Gedcom_ctxt) MAKE_GOM_CTXT(rec, STRUCTTYPE, xr->object);        \
    else                                                                      \
      return NULL;                                                            \
  }

#define GET_REC_BY_XREF(STRUCTTYPE,XREF_TYPE,FUNC_NAME)                       \
  struct STRUCTTYPE *FUNC_NAME(const char *xrefstr)                           \
  {                                                                           \
    struct xref_value* xr = gedcom_get_by_xref(xrefstr);                      \
    if (xr && (xr->type == XREF_TYPE) && xr->object)                          \
      return (struct STRUCTTYPE*)(xr->object);                                \
    else                                                                      \
      return NULL;                                                            \
  }

#define STRING_CB(STRUCTTYPE,CB_NAME,FIELD)                                   \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    Gom_ctxt result = NULL;                                                   \
    if (! parent)                                                             \
      NO_CONTEXT;                                                             \
    else {                                                                    \
      struct STRUCTTYPE *obj                                                  \
        = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                       \
      if (obj) {                                                              \
        char *str = GEDCOM_STRING(parsed_value);                              \
        obj->FIELD = strdup(str);                                             \
        if (! obj->FIELD)                                                     \
	  MEMORY_ERROR;                                                       \
        else                                                                  \
          result = MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                       \
      }                                                                       \
    }                                                                         \
    return (Gedcom_ctxt)result;                                               \
  }

#define STRING_END_CB(STRUCTTYPE,CB_NAME,FIELD)                               \
  void CB_NAME(_ELT_END_PARAMS_)                                              \
  {                                                                           \
    Gom_ctxt ctxt = (Gom_ctxt)self;                                           \
    if (! ctxt)                                                               \
      NO_CONTEXT;                                                             \
    else {                                                                    \
      struct STRUCTTYPE *obj = SAFE_CTXT_CAST(STRUCTTYPE, ctxt);              \
      if (obj) {                                                              \
	char *str = GEDCOM_STRING(parsed_value);                              \
	char *newvalue = strdup(str);                                         \
	if (! newvalue)                                                       \
	  MEMORY_ERROR;                                                       \
	else                                                                  \
	  obj->FIELD = newvalue;                                              \
      }                                                                       \
      destroy_gom_ctxt(ctxt);                                                 \
    }                                                                         \
  }

#define STRING_END_REC_CB(STRUCTTYPE,CB_NAME,FIELD)                           \
  void CB_NAME(_REC_END_PARAMS_)                                              \
  {                                                                           \
    Gom_ctxt ctxt = (Gom_ctxt)self;                                           \
    if (! ctxt)                                                               \
      NO_CONTEXT;                                                             \
    else {                                                                    \
      struct STRUCTTYPE *obj = SAFE_CTXT_CAST(STRUCTTYPE, ctxt);              \
      if (obj) {                                                              \
	char *str = GEDCOM_STRING(parsed_value);                              \
	char *newvalue = strdup(str);                                         \
	if (! newvalue)                                                       \
	  MEMORY_ERROR;                                                       \
	else                                                                  \
	  obj->FIELD = newvalue;                                              \
      }                                                                       \
      destroy_gom_ctxt(ctxt);                                                 \
    }                                                                         \
  }

#define DATE_CB(STRUCTTYPE,CB_NAME,FIELD)                                     \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    Gom_ctxt result = NULL;                                                   \
    if (! parent)                                                             \
      NO_CONTEXT;                                                             \
    else {                                                                    \
      struct STRUCTTYPE *obj                                                  \
        = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                       \
      if (obj) {                                                              \
        struct date_value dv = GEDCOM_DATE(parsed_value);                     \
        obj->FIELD = dup_date(dv);                                            \
        if (! obj->FIELD)                                                     \
	  MEMORY_ERROR;                                                       \
        else                                                                  \
          result = MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                       \
      }                                                                       \
    }                                                                         \
    return (Gedcom_ctxt)result;                                               \
  }

#define AGE_CB(STRUCTTYPE,CB_NAME,FIELD)                                      \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    Gom_ctxt result = NULL;                                                   \
    if (! parent)                                                             \
      NO_CONTEXT;                                                             \
    else {                                                                    \
      struct STRUCTTYPE *obj                                                  \
        = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                       \
      if (obj) {                                                              \
        struct age_value age = GEDCOM_AGE(parsed_value);                      \
        obj->FIELD = dup_age(age);                                            \
        if (! obj->FIELD)                                                     \
	  MEMORY_ERROR;                                                       \
        else                                                                  \
          result = MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                       \
      }                                                                       \
    }                                                                         \
    return (Gedcom_ctxt)result;                                               \
  }

#define XREF_CB(STRUCTTYPE,CB_NAME,FIELD,FUNC)                                \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    Gom_ctxt result = NULL;                                                   \
    if (! parent)                                                             \
      NO_CONTEXT;                                                             \
    else {                                                                    \
      struct STRUCTTYPE *obj                                                  \
        = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                       \
      struct xref_value *xr = GEDCOM_XREF_PTR(parsed_value);                  \
      if (! xr->object)                                                       \
        xr->object = (Gedcom_ctxt) FUNC(xr->string);                          \
      if (obj) {                                                              \
	obj->FIELD = xr;                                                      \
        result = MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                         \
      }                                                                       \
    }                                                                         \
    return (Gedcom_ctxt)result;                                               \
  }

#define XREF_LIST_CB(STRUCTTYPE,CB_NAME,FIELD,FUNC)                           \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    Gom_ctxt result = NULL;                                                   \
    if (! parent)                                                             \
      NO_CONTEXT;                                                             \
    else {                                                                    \
      struct STRUCTTYPE *obj                                                  \
        = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                       \
      struct xref_value *xr = GEDCOM_XREF_PTR(parsed_value);                  \
      struct xref_list *xrl;                                                  \
      if (! xr->object)                                                       \
        xr->object = (Gedcom_ctxt) FUNC(xr->string);                          \
      if (obj) {                                                              \
        MAKE_CHAIN_ELT(xref_list, obj->FIELD, xrl);                           \
        if (xrl) {                                                            \
          xrl->xref = xr;                                                     \
          result = MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                       \
        }                                                                     \
      }                                                                       \
    }                                                                         \
    return (Gedcom_ctxt)result;                                               \
  }

#define NULL_CB(STRUCTTYPE,CB_NAME)                                           \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    Gom_ctxt result = NULL;                                                   \
    if (! parent)                                                             \
      NO_CONTEXT;                                                             \
    else {                                                                    \
      struct STRUCTTYPE *obj                                                  \
        = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                       \
      if (obj)                                                                \
	result = MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                         \
    }                                                                         \
    return (Gedcom_ctxt)result;                                               \
  }

#endif /* __GOM_INTERNAL_H */
