/* General header for the Gedcom object model, defining function macros
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

#ifndef __FUNC_TEMPLATE_H
#define __FUNC_TEMPLATE_H

#define MAKEFUNC(STRUCTTYPE)     make_ ## STRUCTTYPE ## _record
#define SUB_MAKEFUNC(STRUCTTYPE) make_ ## STRUCTTYPE
#define DESTROYFUNC(STRUCTTYPE)  destroy_ ## STRUCTTYPE ## _record
#define GETXREFFUNC(STRUCTTYPE)  gom_get_ ## STRUCTTYPE ## _by_xref
#define CLEANFUNC(STRUCTTYPE)    STRUCTTYPE ## _cleanup
#define ADDFUNC(STRUCTTYPE)      gom_new_ ## STRUCTTYPE
#define SUB_SETFUNC(STRUCTTYPE)  gom_set_new_ ## STRUCTTYPE
#define UNREFALLFUNC(STRUCTTYPE) STRUCTTYPE ## _unref_all
#define DELETEFUNC(STRUCTTYPE)   gom_delete_ ## STRUCTTYPE
#define SUB_DELETEFUNC(STRUCTTYPE) gom_delete_ ## STRUCTTYPE
#define ADDFUNC2(T1,T2)          T1 ## _add_ ## T2
#define ADDFUNC2_TOVAR(T1,T2,F)  T1 ## _add_ ## T2 ## _to_ ## F
#define ADDFUNC2_NOLIST(T1,T2)   ADDFUNC2(T1,T2)
#define ADDFUNC2_STR(T1,F)       ADDFUNC2(T1,F)
#define ADDFUNC2_STRN(T1,F)      ADDFUNC2(T1,F)

#define DECLARE_MAKEFUNC(STRUCTTYPE)                                          \
  struct STRUCTTYPE* MAKEFUNC(STRUCTTYPE)(const char* xref)

#define DECLARE_SUB_MAKEFUNC(STRUCTTYPE)                                      \
  struct STRUCTTYPE* SUB_MAKEFUNC(STRUCTTYPE)()

#define DECLARE_CLEANFUNC(STRUCTTYPE)                                         \
  void CLEANFUNC(STRUCTTYPE)(struct STRUCTTYPE* obj)

#define DECLARE_UNREFALLFUNC(STRUCTTYPE)                                      \
  void UNREFALLFUNC(STRUCTTYPE)(struct STRUCTTYPE* obj)

#define DECLARE_ADDFUNC2(STRUCTTYPE,T2)                                       \
  void ADDFUNC2(STRUCTTYPE,T2)(Gom_ctxt ctxt, struct T2* obj)

#define DECLARE_ADDFUNC2_TOVAR(STRUCTTYPE,T2,F)                               \
  void ADDFUNC2_TOVAR(STRUCTTYPE,T2,F)(Gom_ctxt ctxt, struct T2* obj)

#define DECLARE_ADDFUNC2_NOLIST(STRUCTTYPE,T2)                                \
  void ADDFUNC2_NOLIST(STRUCTTYPE,T2)(Gom_ctxt ctxt, struct T2* obj)

#define DECLARE_ADDFUNC2_STR(STRUCTTYPE,F)                                    \
  void ADDFUNC2_STR(STRUCTTYPE,F)(Gom_ctxt ctxt, const char* str)

#define DECLARE_ADDFUNC2_STRN(STRUCTTYPE,F)                                   \
  void ADDFUNC2_STRN(STRUCTTYPE,F)(Gom_ctxt ctxt, const char* str)

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
      (FIRSTVAL)->previous->next = VAL;                                       \
      VAL->previous = (FIRSTVAL)->previous;                                   \
      (FIRSTVAL)->previous = VAL;                                             \
    }                                                                         \
  }

#define UNLINK_CHAIN_ELT(STRUCTTYPE, FIRSTVAL, VAL)                           \
  {                                                                           \
    struct STRUCTTYPE *_local_obj = VAL;                                      \
    if (VAL == FIRSTVAL)                                                      \
      FIRSTVAL = _local_obj->next;                                            \
    else                                                                      \
      VAL->previous->next = VAL->next;                                        \
    if (VAL->next)                                                            \
      VAL->next->previous = VAL->previous;                                    \
    else if (FIRSTVAL)                                                        \
      (FIRSTVAL)->previous = VAL->previous;                                   \
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

#define DESTROY_CHAIN_ELTS(STRUCTTYPE, FIRSTVAL)                              \
  {                                                                           \
    if (FIRSTVAL) {                                                           \
      struct STRUCTTYPE *runner, *next;                                       \
      runner = FIRSTVAL;                                                      \
      FIRSTVAL = NULL;                                                        \
      while (runner) {                                                        \
	next = runner->next;                                                  \
        CLEANFUNC(STRUCTTYPE)(runner);                                        \
        SAFE_FREE(runner);                                                    \
	runner = next;                                                        \
      }                                                                       \
    }                                                                         \
  }

/* General functions */
#define DEFINE_MAKEFUNC(STRUCTTYPE,FIRSTVAL)                                  \
  struct STRUCTTYPE* MAKEFUNC(STRUCTTYPE)(const char* xrefstr) {              \
    struct STRUCTTYPE* obj = NULL;                                            \
    if (xrefstr) {                                                            \
      MAKE_CHAIN_ELT(STRUCTTYPE, FIRSTVAL, obj);                              \
      if (obj) {                                                              \
	obj->xrefstr = strdup(xrefstr);                                       \
	if (!obj->xrefstr) MEMORY_ERROR;                                      \
      }                                                                       \
    }                                                                         \
    return obj;                                                               \
  }

#define DEFINE_SUB_MAKEFUNC(STRUCTTYPE)                                       \
  struct STRUCTTYPE* SUB_MAKEFUNC(STRUCTTYPE)() {                             \
    struct STRUCTTYPE* obj = NULL;                                            \
    obj = (struct STRUCTTYPE*) malloc(sizeof(struct STRUCTTYPE));             \
    if (!obj)                                                                 \
      MEMORY_ERROR;                                                           \
    else                                                                      \
      memset(obj, 0, sizeof(struct STRUCTTYPE));                              \
    return obj;                                                               \
  }

#define DEFINE_DESTROYFUNC(STRUCTTYPE,FIRSTVAL)                               \
  DECLARE_CLEANFUNC(STRUCTTYPE);                                              \
  void DESTROYFUNC(STRUCTTYPE)(struct STRUCTTYPE* obj) {                      \
    if (obj) {                                                                \
      CLEANFUNC(STRUCTTYPE)(obj);                                             \
      UNLINK_CHAIN_ELT(STRUCTTYPE, FIRSTVAL, obj);                            \
      SAFE_FREE(obj);                                                         \
    }                                                                         \
  }

#define DEFINE_GETXREFFUNC(STRUCTTYPE,XREF_TYPE)                              \
  struct STRUCTTYPE *GETXREFFUNC(STRUCTTYPE)(const char *xrefstr)             \
  {                                                                           \
    struct xref_value* xr = gedcom_get_by_xref(xrefstr);                      \
    if (xr && (xr->type == XREF_TYPE) && xr->object)                          \
      return (struct STRUCTTYPE*)(xr->object);                                \
    else                                                                      \
      return NULL;                                                            \
  }

#define DEFINE_ADDFUNC(STRUCTTYPE,XREF_TYPE)                                  \
  struct STRUCTTYPE *ADDFUNC(STRUCTTYPE)(const char* xrefstr)                 \
  {                                                                           \
    struct STRUCTTYPE *obj = NULL;                                            \
    struct xref_value* xrv = gedcom_get_by_xref(xrefstr);                     \
    if (xrv)                                                                  \
      gom_xref_already_in_use(xrefstr);                                       \
    else {                                                                    \
      obj = MAKEFUNC(STRUCTTYPE)(xrefstr);                                    \
      if (obj) {                                                              \
	xrv = gedcom_add_xref(XREF_TYPE, xrefstr, (Gedcom_ctxt)obj);          \
	if (!xrv) {                                                           \
	  DESTROYFUNC(STRUCTTYPE)(obj);                                       \
	  obj = NULL;                                                         \
	}                                                                     \
      }                                                                       \
    }                                                                         \
    return obj;                                                               \
  }

#define DEFINE_SUB_SETFUNC(STRUCTTYPE)                                        \
  struct STRUCTTYPE *SUB_SETFUNC(STRUCTTYPE)(struct STRUCTTYPE** addto)       \
  {                                                                           \
    struct STRUCTTYPE *obj = NULL;                                            \
    if (addto && ! *addto) {                                                  \
      obj = SUB_MAKEFUNC(STRUCTTYPE)();                                       \
      if (obj) *addto = obj;                                                  \
    }                                                                         \
    return obj;                                                               \
  }

#define DEFINE_DELETEFUNC(STRUCTTYPE)                                         \
  DECLARE_UNREFALLFUNC(STRUCTTYPE);                                           \
  int DELETEFUNC(STRUCTTYPE)(struct STRUCTTYPE* obj)                          \
  {                                                                           \
    int result = 1;                                                           \
    if (obj) {                                                                \
      result = gedcom_delete_xref(obj->xrefstr);                              \
      if (result == 0) {                                                      \
        UNREFALLFUNC(STRUCTTYPE)(obj);                                        \
	DESTROYFUNC(STRUCTTYPE)(obj);                                         \
      }                                                                       \
    }                                                                         \
    return result;                                                            \
  }

#define DEFINE_SUB_DELETEFUNC(STRUCTTYPE)                                     \
  int SUB_DELETEFUNC(STRUCTTYPE)(struct STRUCTTYPE** obj)                     \
  {                                                                           \
    int result = 1;                                                           \
    if (obj && *obj) {                                                        \
      UNREFALLFUNC(STRUCTTYPE)(*obj);                                         \
      CLEANFUNC(STRUCTTYPE)(*obj);                                            \
      SAFE_FREE(*obj);                                                        \
      result = 0;                                                             \
    }                                                                         \
    return result;                                                            \
  }

#define DEFINE_ADDFUNC2(STRUCTTYPE,T2,FIELD)                                  \
  void ADDFUNC2(STRUCTTYPE,T2)(Gom_ctxt ctxt, struct T2* addobj)              \
  {                                                                           \
    struct STRUCTTYPE *obj = SAFE_CTXT_CAST(STRUCTTYPE, ctxt);                \
    if (obj)                                                                  \
      LINK_CHAIN_ELT(T2, obj->FIELD, addobj);                                 \
  }

#define DEFINE_ADDFUNC2_TOVAR(STRUCTTYPE,T2,FIELD)                            \
  void ADDFUNC2_TOVAR(STRUCTTYPE,T2,FIELD)(Gom_ctxt ctxt, struct T2* addobj)  \
  {                                                                           \
    struct STRUCTTYPE *obj = SAFE_CTXT_CAST(STRUCTTYPE, ctxt);                \
    if (obj)                                                                  \
      LINK_CHAIN_ELT(T2, obj->FIELD, addobj);                                 \
  }

#define DEFINE_ADDFUNC2_NOLIST(STRUCTTYPE,T2, FIELD)                          \
  void ADDFUNC2_NOLIST(STRUCTTYPE,T2)(Gom_ctxt ctxt, struct T2* addobj)       \
  {                                                                           \
    struct STRUCTTYPE *obj = SAFE_CTXT_CAST(STRUCTTYPE, ctxt);                \
    if (obj)                                                                  \
      obj->FIELD = addobj;                                                    \
  }

#define DEFINE_ADDFUNC2_STR(STRUCTTYPE,FIELD)                                 \
void ADDFUNC2_STR(STRUCTTYPE,FIELD)(Gom_ctxt ctxt, const char *str)           \
{                                                                             \
  struct STRUCTTYPE *obj = SAFE_CTXT_CAST(STRUCTTYPE, ctxt);                  \
  if (obj) {                                                                  \
    obj->FIELD = strdup(str);                                                 \
    if (! obj->FIELD) MEMORY_ERROR;                                           \
  }                                                                           \
}

#define DEFINE_ADDFUNC2_STRN(STRUCTTYPE,FIELD,N)                              \
void ADDFUNC2_STRN(STRUCTTYPE,FIELD)(Gom_ctxt ctxt, const char *str)          \
{                                                                             \
  struct STRUCTTYPE *obj = SAFE_CTXT_CAST(STRUCTTYPE, ctxt);                  \
  if (obj) {                                                                  \
    int i = 0;                                                                \
    while (i < N-1 && obj->FIELD[i]) i++;                                     \
    if (! obj->FIELD[i]) {                                                    \
      obj->FIELD[i] = strdup(str);                                            \
      if (! obj->FIELD[i]) MEMORY_ERROR;                                      \
    }                                                                         \
  }                                                                           \
}

/* Definition of callbacks */
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

#define DEFINE_REC_CB(STRUCTTYPE,CB_NAME)                                     \
  Gedcom_ctxt CB_NAME(_REC_PARAMS_)                                           \
  {                                                                           \
    struct xref_value* xr = GEDCOM_XREF_PTR(xref);                            \
    if (! xr->object)                                                         \
      xr->object = (Gedcom_ctxt) MAKEFUNC(STRUCTTYPE)(xr->string);            \
    if (xr->object)                                                           \
      return (Gedcom_ctxt) MAKE_GOM_CTXT(rec, STRUCTTYPE, xr->object);        \
    else                                                                      \
      return NULL;                                                            \
  }

#define DEFINE_STRING_CB(STRUCTTYPE,CB_NAME,FIELD)                            \
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

#define DEFINE_STRING_END_CB(STRUCTTYPE,CB_NAME,FIELD)                        \
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

#define DEFINE_STRING_END_REC_CB(STRUCTTYPE,CB_NAME,FIELD)                    \
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

#define DEFINE_DATE_CB(STRUCTTYPE,CB_NAME,FIELD)                              \
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
        obj->FIELD = gedcom_new_date_value(&dv);                              \
        if (! obj->FIELD)                                                     \
	  MEMORY_ERROR;                                                       \
        else                                                                  \
          result = MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                       \
      }                                                                       \
    }                                                                         \
    return (Gedcom_ctxt)result;                                               \
  }

#define DEFINE_AGE_CB(STRUCTTYPE,CB_NAME,FIELD)                               \
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
        obj->FIELD = gedcom_new_age_value(&age);                              \
        if (! obj->FIELD)                                                     \
	  MEMORY_ERROR;                                                       \
        else                                                                  \
          result = MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                       \
      }                                                                       \
    }                                                                         \
    return (Gedcom_ctxt)result;                                               \
  }

#define DEFINE_XREF_CB(STRUCTTYPE,CB_NAME,FIELD,LINKSTRTYPE)                  \
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
        xr->object = (Gedcom_ctxt) MAKEFUNC(LINKSTRTYPE)(xr->string);         \
      if (obj) {                                                              \
	obj->FIELD = xr;                                                      \
        result = MAKE_GOM_CTXT(elt, STRUCTTYPE, obj);                         \
      }                                                                       \
    }                                                                         \
    return (Gedcom_ctxt)result;                                               \
  }

#define DEFINE_XREF_LIST_CB(STRUCTTYPE,CB_NAME,FIELD,LINKSTRTYPE)             \
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
        xr->object = (Gedcom_ctxt) MAKEFUNC(LINKSTRTYPE)(xr->string);         \
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

#define DEFINE_NULL_CB(STRUCTTYPE,CB_NAME)                                    \
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



#endif /* __FUNC_TEMPLATE_H */ 
