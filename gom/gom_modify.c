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

/** If some characters cannot be converted by gom_get_string_for_locale(),
    then these characters are by default replaced by the string "?".  The
    function gom_set_unknown() can configure the string to be used.

    \param unknown The new replacement string for conversion failures
*/
void gom_set_unknown(const char* unknown)
{
  convert_set_unknown(unknown);
}

/** Returns the string from the Gedcom object model referenced by \c data
    in UTF-8 format.

    \param data The string from the Gedcom object model
    \return The string in UTF-8 format (is in fact a pass-through operation)
*/
char* gom_get_string(char* data)
{
  return data;
}

/** Returns the string from the Gedcom object model referenced by \c data
    in the encoding defined by the current locale.

    Since the conversion from
    UTF-8 to the locale encoding is not always possible, the function has a
    second parameter that can return the number of conversion failures for the
    result string.

    \param data The string from the Gedcom object model
    \param conversion_failures Pass a pointer to an integer if you want to know
    the number of conversion failures (filled in on return).  You can pass
    \c NULL if you're not interested.

    \return The string in the encoding defined by the current locale, with
    unconvertible characters replaced by the "unknown string" (see
    gom_set_unknown())
*/
char* gom_get_string_for_locale(char* data, int* conversion_failures)
{
  return convert_utf8_to_locale(gom_get_string(data), conversion_failures);
}

/** Sets the string from the Gedcom object model referenced by \c data to the
    given input \c utf8_str, which must be a string in UTF-8 encoding.

    The function makes a copy of the input string
    to store it in the object model.  It also takes care of deallocating the
    old value of the data if needed.

    Note that this function needs the \em address of the data variable, to
    be able to modify it.

    \param data The string from the Gedcom object model
    \param utf8_str A new string, in UTF-8 encoding

    \return The new value if successful, or \c NULL if an error occurred, e.g.:
      - failure to allocate memory
      - the given string is not a valid UTF-8 string
      .
    In the case of an error, the target data variable is not modified.
*/
char* gom_set_string(char** data, const char* utf8_str)
{
  char* result = NULL;
  char* newptr;

  if (utf8_str == NULL) {
    SAFE_FREE(*data);
  }
  else {
    if (!is_utf8_string(utf8_str)) {
      gedcom_error(_("The input '%s' is not a valid UTF-8 string"), utf8_str);
    }
    else {
      newptr = strdup(utf8_str);
      if (!newptr)
	MEMORY_ERROR;
      else {
	SAFE_FREE(*data);
	*data = newptr;
	result = *data;
      }
    }
  }
  
  return result;
}

/** Sets the string from the Gedcom object model referenced by \c data to the
    given input \c locale_str, which must be a string in the encoding defined
    by the current locale.

    The function makes a copy of the input string
    to store it in the object model.  It also takes care of deallocating the
    old value of the data if needed.

    Note that this function needs the \em address of the data variable, to
    be able to modify it.

    \param data The string from the Gedcom object model
    \param locale_str A new string, in encoding defined by the current locale

    \return The new value if successful, or \c NULL if an error occurred, e.g.:
      - failure to allocate memory
      - the given string is not a valid string for the current locale
      .
    In the case of an error, the target data variable is not modified.
*/
char* gom_set_string_for_locale(char** data, const char* locale_str)
{
  char* result = NULL;

  if (locale_str == NULL) {
    result = gom_set_string(data, NULL);
  }
  else {
    char* utf8_str = convert_locale_to_utf8(locale_str);
    
    if (!utf8_str)
      gedcom_error(_("The input '%s' is not a valid string for the locale"),
		   locale_str);
    else
      result = gom_set_string(data, utf8_str);
  }

  return result;
}

void unref_xref_value(struct xref_value *xref)
{
  if (xref)
    gedcom_unlink_xref(xref->type, xref->string);
}

void UNREFALLFUNC(xref_list)(struct xref_list* obj)
{
  if (obj) {
    struct xref_list* runner;
    for (runner = obj; runner; runner = runner->next) {
      unref_xref_value(runner->xref);
      UNREFALLFUNC(user_data)(runner->extra);
    }
  }
}

void CLEANFUNC(xref_list)(struct xref_list *obj)
{
  if (obj) {
    DESTROY_CHAIN_ELTS(user_data, obj->extra);
  }
}

/** This function modifies a data variable in the Gedcom object model of
    type struct xref_value to point to the given \c xref, taking care of
    unreferencing the old value, and referencing the new value (resp.
    decrementing and incrementing the reference count).

    \param data The address of the xref_value member in the object model
    \param xref The cross-reference key of an existing object, or \c NULL
    \return The new value of the data variable, or \c NULL if an error
    occurred, e.g. there was no record found with the given key
    (in this case, the data variable is not changed).
*/
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

/** This function adds the given cross-reference to the end of the \c data
    list, taking care of incrementing the reference count of the
    cross-reference.

    \param data The address of the xref_list member in the object model
    \param xref The cross-reference key of an existing object

    \return The new entry in the \c data list, or \c NULL if an error occurred.
*/
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

struct xref_list* find_xref(struct xref_list** data, const char* xref)
{
  struct xref_list* result = NULL;
  struct xref_value* xr = gedcom_get_by_xref(xref);
  if (!xr)
    gedcom_error(_("No record found for xref '%s'"), xref);
  else {
    struct xref_list* xrl;
    for (xrl = *data ; xrl ; xrl = xrl->next) {
      if (xrl->xref == xr) {
	result = xrl;
	break;
      }
    }
    if (! result)
      gedcom_error(_("Xref '%s' not part of chain"), xref);
  }
  return result;
}

/** This function removes the given cross-reference from the \c data
    list, taking care of decrementing the reference count of the
    cross-reference.

    \param data The address of the xref_list member in the object model
    \param xref The cross-reference key of an existing object

    \retval 0 if successful
    \retval 1 if error (e.g. because not present in the list)
*/
int gom_remove_xref(struct xref_list** data, const char* xref)
{
  int result = 1;

  if (data && xref) {
    struct xref_list* xrl = find_xref(data, xref);
    if (xrl) {
      UNLINK_CHAIN_ELT(xref_list, *data, xrl);
      gedcom_unlink_xref(xrl->xref->type, xrl->xref->string);
      CLEANFUNC(xref_list)(xrl);
      SAFE_FREE(xrl);
      result = 0;
    }
  }

  return result;
}

/** This function moves the given cross-reference up or down the \c data
    list.

    If the cross-reference cannot be moved up (because the first in the list)
    or down (because the last in the list), a warning is generated, but the
    function still returns success.

    \param dir The direction to move into
    \param data The address of the xref_list member in the object model
    \param xref The cross-reference key of an existing object

    \retval 0 if successful
    \retval 1 if error (e.g. because not present in the list)
*/
int gom_move_xref(Gom_direction dir, struct xref_list** data, const char* xref)
{
  int result = 1;

  if (data && xref) {
    struct xref_list* xrl = find_xref(data, xref);
    if (xrl) {
      MOVE_CHAIN_ELT(xref_list, dir, *data, xrl);
      result = 0;
    }
  }

  return result;
}
