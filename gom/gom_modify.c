/* Source code for modifying the gedcom object model.
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
#include "utf8tools.h"
#include "user_rec.h"
#include "gom.h"
#include "gom_internal.h"

void gom_set_unknown(const char* unknown)
{
  convert_set_unknown(unknown);
}

char* gom_get_string(char* data)
{
  return data;
}

char* gom_get_string_for_locale(char* data, int* conversion_failures)
{
  return convert_utf8_to_locale(gom_get_string(data), conversion_failures);
}

char* gom_set_string(char** data, const char* utf8_str)
{
  char* result = NULL;
  char* newptr;

  if (!is_utf8_string(utf8_str)) {
    gedcom_error(_("The input '%s' is not a valid UTF-8 string"), utf8_str);
  }
  else {
    newptr = strdup(utf8_str);
    if (!newptr)
      MEMORY_ERROR;
    else {
      if (*data) free(*data);
      *data = newptr;
      result = *data;
    }
  }
  
  return result;
}

char* gom_set_string_for_locale(char** data, const char* locale_str)
{
  char* result = NULL;
  char* utf8_str = convert_locale_to_utf8(locale_str);
  
  if (!utf8_str)
    gedcom_error(_("The input '%s' is not a valid string for the locale"),
		 locale_str);
  else
    result = gom_set_string(data, utf8_str);

  return result;
}

void CLEANFUNC(xref_list)(struct xref_list *obj)
{
  if (obj) {
    DESTROY_CHAIN_ELTS(user_data, obj->extra);
  }
}

struct xref_value* gom_set_xref(struct xref_value** data, const char* xref)
{
  struct xref_value* result = NULL;
  struct xref_value* newval = NULL;
  
  if (data) {
    if (xref) {
      newval = gedcom_get_by_xref(xref);
      if (!newval)
	gedcom_error(_("No record found for xref '%s'"), xref);
    }
    
    /* Unreference the old value if not NULL */
    if (*data)
      result = gedcom_unlink_xref((*data)->type, (*data)->string);
    else
      result = newval;
    
    /* Reference the new value if not NULL */
    if (result != NULL && newval) {
      result = gedcom_link_xref(newval->type, newval->string);
      /* On error, perform rollback to old value (guaranteed to work) */
      if (result == NULL)
	gedcom_link_xref((*data)->type, (*data)->string);
    }
    
    if (result != NULL) {
      *data = newval;
      result = newval;
    }
  }
  return result;
}

struct xref_list* gom_add_xref(struct xref_list** data, const char* xref)
{
  struct xref_value* result = NULL;
  struct xref_value* newval = NULL;
  struct xref_list* xrl = NULL;

  if (data && xref) {
    newval = gedcom_get_by_xref(xref);
    if (!newval)
      gedcom_error(_("No record found for xref '%s'"), xref);
    else {
      result = gedcom_link_xref(newval->type, newval->string);
      if (result != NULL) {
	MAKE_CHAIN_ELT(xref_list, *data, xrl);
	if (xrl) xrl->xref = newval;
      }
    }
  }

  return xrl;
}

int gom_remove_xref(struct xref_list** data, const char* xref)
{
  struct xref_value* xr = NULL;
  int result = 1;
  
  if (data && xref) {
    xr = gedcom_get_by_xref(xref);
    if (!xr)
      gedcom_error(_("No record found for xref '%s'"), xref);
    else {
      struct xref_list* xrl = NULL;
      for (xrl = *data ; xrl ; xrl = xrl->next) {
	if (xrl->xref == xr) {
	  UNLINK_CHAIN_ELT(xref_list, *data, xrl);
	  gedcom_unlink_xref(xr->type, xr->string);
	  CLEANFUNC(xref_list)(xrl);
	  SAFE_FREE(xrl);
	  result = 0;
	  break;
	}
      }
      if (result == 1)
	gedcom_error(_("Xref '%s' to remove not part of chain"), xref);
    }
  }

  return result;
}
