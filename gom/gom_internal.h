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
#include "gom.h"
#include "gedcom.h"
#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif
      
#define _(string) gettext(string)
#define N_(string) (string)

typedef enum {
  T_NULL,
  
  T_header, T_submission, T_submitter, T_family, T_individual,
  T_multimedia, T_note, T_repository, T_source, T_user_rec,
  
  T_address, T_event, T_place, T_source_citation, T_text,
  T_note_sub, T_multimedia_link, T_lds_event, T_user_ref_number,
  T_change_date, T_personal_name, T_family_link, T_pedigree,
  T_association, T_source_event, T_source_description
} OBJ_TYPE;

struct Gom_ctxt_struct {
  int ctxt_type;
  OBJ_TYPE obj_type;
  void* ctxt_ptr;
};

typedef struct Gom_ctxt_struct *Gom_ctxt;

Gom_ctxt make_gom_ctxt(int ctxt_type, OBJ_TYPE obj_type, void *ctxt_ptr);
void destroy_gom_ctxt(Gom_ctxt ctxt);
void gom_cast_error(char* file, int line, OBJ_TYPE expected, OBJ_TYPE found);
void gom_unexpected_context(char* file, int line, OBJ_TYPE found);

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

void gom_mem_error(char *filename, int line);

#define MEMORY_ERROR gom_mem_error(__FILE__, __LINE__)

void def_rec_end(Gedcom_rec rec, Gedcom_ctxt self);
void def_elt_end(Gedcom_elt elt, Gedcom_ctxt parent, Gedcom_ctxt self,
		 Gedcom_val parsed_value);
void set_xref_type(struct xref_value *xr, char* str);

typedef enum {
  WITHOUT_NL,
  WITH_NL
} NL_TYPE;

char* concat_strings(NL_TYPE type, char *str1, const char *str2);
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
    memset (VAL, 0, sizeof(struct STRUCTTYPE));                               \
    LINK_CHAIN_ELT(STRUCTTYPE, FIRSTVAL, VAL)                                 \
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

#define _REC_PARAMS_ Gedcom_rec rec, int level, Gedcom_val xref, char *tag,   \
                     char *raw_value, int parsed_tag, Gedcom_val parsed_value

#define _ELT_PARAMS_ Gedcom_elt elt, Gedcom_ctxt parent, int level, char *tag,\
                     char *raw_value, int parsed_tag, Gedcom_val parsed_value

#define REC_CB(STRUCTTYPE,CB_NAME,FUNC)                                       \
  Gedcom_ctxt CB_NAME(_REC_PARAMS_)                                           \
  {                                                                           \
    struct xref_value* xr = GEDCOM_XREF_PTR(xref);                            \
    if (! xr->object)                                                         \
      xr->object = (Gedcom_ctxt) FUNC(xr->string);                            \
    return (Gedcom_ctxt) MAKE_GOM_CTXT(rec, STRUCTTYPE, xr->object);          \
  }

#define GET_REC_BY_XREF(STRUCTTYPE,XREF_TYPE,FUNC_NAME)                       \
  struct STRUCTTYPE *FUNC_NAME(char *xrefstr)                                 \
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
    char *str = GEDCOM_STRING(parsed_value);                                  \
    struct STRUCTTYPE *obj                                                    \
      = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                         \
    if (obj) obj->FIELD = strdup(str);                                        \
    return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                 \
  }

#define DATE_CB(STRUCTTYPE,CB_NAME,FIELD)                                     \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    struct date_value dv = GEDCOM_DATE(parsed_value);                         \
    struct STRUCTTYPE *obj                                                    \
      = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                         \
    if (obj) obj->FIELD = dup_date(dv);                                       \
    return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                 \
  }

#define AGE_CB(STRUCTTYPE,CB_NAME,FIELD)                                      \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    struct age_value age = GEDCOM_AGE(parsed_value);                          \
    struct STRUCTTYPE *obj                                                    \
      = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                         \
    if (obj) obj->FIELD = dup_age(age);                                       \
    return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                 \
  }

#define XREF_CB(STRUCTTYPE,CB_NAME,FIELD,FUNC)                                \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    struct xref_value *xr = GEDCOM_XREF_PTR(parsed_value);                    \
    struct STRUCTTYPE *obj                                                    \
      = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                         \
    if (! xr->object)                                                         \
      xr->object = (Gedcom_ctxt) FUNC(xr->string);                            \
    if (obj) obj->FIELD = xr;                                                 \
    return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                 \
  }

#define XREF_LIST_CB(STRUCTTYPE,CB_NAME,FIELD,FUNC)                           \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    struct xref_value *xr = GEDCOM_XREF_PTR(parsed_value);                    \
    struct STRUCTTYPE *obj                                                    \
      = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                         \
    struct xref_list *xrl;                                                    \
    if (! xr->object)                                                         \
      xr->object = (Gedcom_ctxt) FUNC(xr->string);                            \
    MAKE_CHAIN_ELT(xref_list, obj->FIELD, xrl);                               \
    xrl->xref = xr;                                                           \
    return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                 \
  }

#define NULL_CB(STRUCTTYPE,CB_NAME)                                           \
  Gedcom_ctxt CB_NAME(_ELT_PARAMS_)                                           \
  {                                                                           \
    struct STRUCTTYPE *obj                                                    \
      = SAFE_CTXT_CAST(STRUCTTYPE, (Gom_ctxt)parent);                         \
    return (Gedcom_ctxt) MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                 \
  }

#endif /* __GOM_INTERNAL_H */
