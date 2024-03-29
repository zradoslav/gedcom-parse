/* Cross-reference manipulation routines.
   Copyright (C) 2001,2002 The Genes Development Team
   This file is part of the Gedcom parser library.
   Contributed by Peter Verthez <Peter.Verthez@advalvas.be>, 2001.

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

#include "gedcom_internal.h"
#include "gedcom.h"
#include "gedcom.tabgen.h"
#include "xref.h"
#include "hash.h"

struct xref_value def_xref_val = { XREF_NONE, "<error>", NULL };
static hash_t *xrefs = NULL;

const char* xref_type_str[] = { N_("nothing"),
				N_("a family"),
				N_("an individual"),
				N_("a note"),
				N_("a multimedia object"),
				N_("a source repository"),
				N_("a source"),
				N_("a submitter"),
				N_("a submission record"),
				N_("an application-specific record"),
                              };

struct xref_node {
  struct xref_value xref;
  Xref_type defined_type;
  Xref_type used_type;
  int defined_line;
  int used_line;
  int use_count;
};

hnode_t *xref_alloc(void *c UNUSED)
{
  return malloc(sizeof *xref_alloc(NULL));
}

void xref_free(hnode_t *n, void *c UNUSED)
{
  struct xref_node *xr = (struct xref_node *)hnode_get(n);
  free((void*)hnode_getkey(n));
  free(xr->xref.string);
  free(xr);
  free(n);
}

void clear_xref_node(struct xref_node *xr)
{
  xr->xref.type    = XREF_NONE;
  /* Make sure that the 'string' member always contains a valid string */
  if (!xr->xref.string)
    xr->xref.string  = strdup("");
  if (!xr->xref.string) MEMORY_ERROR;
  xr->xref.object  = NULL;
  xr->defined_type = XREF_NONE;
  xr->used_type    = XREF_NONE;
  xr->defined_line = -1;
  xr->used_line    = -1;
  xr->use_count    = 0;
}

struct xref_node *make_xref_node()
{
  struct xref_node *xr = (struct xref_node *)malloc(sizeof(struct xref_node));
  if (xr) {
    xr->xref.string = NULL;
    clear_xref_node(xr);
  }
  else
    MEMORY_ERROR;
  return xr;
}

void delete_xref_node(struct xref_node* xr)
{
  if (!xr->xref.string)
    free(xr->xref.string);
  free(xr);
}

void cleanup_xrefs()
{
  hash_free(xrefs);
  xrefs = NULL;
}

void make_xref_table()
{
  if (xrefs)
    cleanup_xrefs();
  else
    /* Only register initially (if xrefs is still NULL) */
    /* So that it is only registered once */
    if (atexit(cleanup_xrefs) != 0) {
      gedcom_warning(_("Could not register xref cleanup function"));
    }
  xrefs = hash_create(HASHCOUNT_T_MAX, NULL, NULL);
  hash_set_allocator(xrefs, xref_alloc, xref_free, NULL);
}

int check_xref_table()
{
  int result = 0;
  hscan_t hs;
  hnode_t *node;
  struct xref_node *xr;

  /* Check for undefined and unused xrefs */
  hash_scan_begin(&hs, xrefs);
  while ((node = hash_scan_next(&hs))) {
    xr = (struct xref_node *)hnode_get(node);
    if (xr->defined_type == XREF_NONE && xr->used_type != XREF_NONE) {
      gedcom_error(_("Cross-reference %s used on line %d is not defined"),
		   xr->xref.string, xr->used_line);
      result |= 1;
    }
    if (xr->used_type == XREF_NONE && xr->defined_type != XREF_NONE) {
      gedcom_warning(_("Cross-reference %s defined on line %d is never used"),
		     xr->xref.string, xr->defined_line);
    }
  }
  
  return result;
}

struct xref_node* add_xref(Xref_type xref_type, const char* xrefstr,
			   Gedcom_ctxt object)
{
  struct xref_node *xr = NULL;
  char *key = strdup(xrefstr);
  if (key) {
    xr = make_xref_node();
    xr->xref.type = xref_type;
    xr->xref.object = object;
    if (xr->xref.string)
      free(xr->xref.string);
    xr->xref.string = strdup(xrefstr);
    if (! xr->xref.string) {
      MEMORY_ERROR;
      free(key);
      delete_xref_node(xr);
      xr = NULL;
    }
    else {
      hash_alloc_insert(xrefs, key, xr);
    }
  }
  else
    MEMORY_ERROR;
  return xr;
}

void remove_xref(struct xref_node* xr)
{
  hnode_t *node = hash_lookup(xrefs, xr->xref.string);
  hash_delete_free(xrefs, node);
}

int set_xref_fields(struct xref_node* xr, Xref_ctxt ctxt, Xref_type xref_type)
{
  int result = 0;

  if (xr->defined_type != XREF_NONE && xr->defined_type != xref_type &&
      xr->defined_type != XREF_ANY) {
    if (xr->defined_line != 0)
      gedcom_error(_("Cross-reference %s previously defined as pointer to %s, "
		     "on line %d"),
		   xr->xref.string, xref_type_str[xr->defined_type],
		   xr->defined_line);
    else
      gedcom_error(_("Cross-reference %s previously defined as pointer to %s"),
		   xr->xref.string, xref_type_str[xr->defined_type]);

    result = 1;
  }  
  else if (xr->used_type != XREF_NONE && xr->used_type != xref_type) {
    if (xr->used_line != 0)
      gedcom_error(_("Cross-reference %s previously used as pointer to %s, "
		     "on line %d"),
		   xr->xref.string, xref_type_str[xr->used_type],
		   xr->used_line);
    else
      gedcom_error(_("Cross-reference %s previously used as pointer to %s"),
		   xr->xref.string, xref_type_str[xr->used_type]);

    result = 1;
  }

  if (result == 0) {
    if (ctxt == XREF_USED)
      xr->use_count++;
    if (ctxt == XREF_DEFINED && xr->defined_type == XREF_NONE) {
      xr->defined_type = xref_type;
      xr->defined_line = line_no;
    }
    else if (ctxt == XREF_USED && xr->used_type == XREF_NONE) {
      xr->used_type = xref_type;
      xr->used_line = line_no;
    }
  }
  
  return result;
}

struct xref_value *gedcom_parse_xref(const char *raw_value,
				     Xref_ctxt ctxt, Xref_type xref_type)
{
  struct xref_node *xr = NULL;
  
  hnode_t *node = hash_lookup(xrefs, raw_value);
  if (node) {
    xr = (struct xref_node *)hnode_get(node);
  }
  else {
    xr = add_xref(xref_type, raw_value, NULL);
  }

  if (xr) {
    set_xref_fields(xr, ctxt, xref_type);
    return &(xr->xref);
  }
  else
    return NULL;
}

/* Functions for retrieving, modifying and deleting cross-references */

int is_valid_pointer(const char *key)
{
  return (strlen(key) <= 22 &&
	  gedcom_check_token(key, STATE_NORMAL, POINTER) == 0);
}

/** Retrieve an xref_value by its key.

    \param key  The given cross-reference key

    \return The object referenced by the key, or \c NULL if the given key
    isn't a valid cross-reference key (see detailed description of
    \ref parsed_xref) or isn't used.
*/ 
struct xref_value* gedcom_get_by_xref(const char *key)
{
  if (!is_valid_pointer(key)) {
    gedcom_error(_("String '%s' is not a valid cross-reference key"), key);
    return NULL;
  }
  else {
    hnode_t *node = hash_lookup(xrefs, key);
    if (node) {
      struct xref_node *xr = (struct xref_node *)hnode_get(node);
      return &(xr->xref);
    }
    else
      return NULL;
  }
}

/** Add an xref_value of the given type, with the given key, to the given
    object, with a use count equal to 0.

    \param type  The type of the referenced object
    \param xrefstr  The key for the object
    \param object   The object to be referenced

    \return The new xref_value if success, or \c NULL in one of the following
    cases:
     - the key isn't a valid cross-reference key (see detailed description of
       \ref parsed_xref)
     - there is already an xref_value with the same key
     - there was a memory allocation error
*/
struct xref_value* gedcom_add_xref(Xref_type type, const char* xrefstr,
				   Gedcom_ctxt object)
{
  struct xref_node *xr = NULL;

  if (!is_valid_pointer(xrefstr)) {
    gedcom_error(_("String '%s' is not a valid cross-reference key"), xrefstr);
  }
  else {
    hnode_t *node = hash_lookup(xrefs, xrefstr);
    if (node) {
      gedcom_error(_("Cross-reference %s already exists"), xrefstr);
    }
    else {
      xr = add_xref(type, xrefstr, object);
      if (xr)
	set_xref_fields(xr, XREF_DEFINED, type);
    }
  }
  if (xr)
    return &(xr->xref);
  else
    return NULL;
}

/** Declare the xref_value corresponding to the given key as being used as the
    given type.  The use of this function is not mandatory, but it can aid in
    spotting places in the code where xref_value objects are deleted while
    they are still referenced.

    \param type  The type of the referenced object
    \param xrefstr  The key for the object

    \return The xref_value object if success, and its use count is incremented.
    Returns NULL in one of the following cases:
     - the key isn't a valid cross-reference key (see detailed description of
       \ref parsed_xref)
     - there is no xref_value with the given key
     - the xref_value was previously added as another type than the type
       provided here
 */
struct xref_value* gedcom_link_xref(Xref_type type, const char* xrefstr)
{
  struct xref_node *xr = NULL;

  if (!is_valid_pointer(xrefstr)) {
    gedcom_error(_("String '%s' is not a valid cross-reference key"), xrefstr);
  }
  else {
    hnode_t *node = hash_lookup(xrefs, xrefstr);
    if (!node) {
      gedcom_error(_("Cross-reference %s not defined"), xrefstr);
    }
    else {
      xr = (struct xref_node *)hnode_get(node);
      if (set_xref_fields(xr, XREF_USED, type) != 0)
	xr = NULL;
    }
  }

  if (xr)
    return &(xr->xref);
  else
    return NULL;
}

/** Declare the xref_value corresponding to the given key no longer used.
    The use of this function is not mandatory, but it can aid in
    spotting places in the code where xref_value objects are deleted while
    they are still referenced.

    \param type  The type of the referenced object
    \param xrefstr  The key for the object

    \return The xref_value object if success, and its use count is decremented.
    Returns NULL in one of the following cases:
     - the key isn't a valid cross-reference key (see detailed description of
       \ref parsed_xref)
     - there is no xref_value with the given key
     - the xref_value was previously added as another type than the type
       provided here
 */
struct xref_value* gedcom_unlink_xref(Xref_type type, const char* xrefstr)
{
  struct xref_node *xr = NULL;
  if (!is_valid_pointer(xrefstr)) {
    gedcom_error(_("String '%s' is not a valid cross-reference key"), xrefstr);
  }
  else {
    hnode_t *node = hash_lookup(xrefs, xrefstr);
    if (! node) {
      gedcom_error(_("Cross-reference %s not defined"), xrefstr);
    }
    else {
      xr = (struct xref_node*) hnode_get(node);
      if (xr->defined_type != type && xr->defined_type != XREF_ANY) {
	gedcom_error
	  (_("Cross-reference %s previously defined as pointer to %s"),
	   xr->xref.string, xref_type_str[xr->defined_type]);
	xr = NULL;
      }
      else
	xr->use_count--;
    }
  }
  if (xr)
    return &(xr->xref);
  else
    return NULL;
}

/** Delete the xref_value corresponding to the given key.

    \param xrefstr  The key for the object

    \return 0 if success; 1 in one of the following cases:
      - the key isn't a valid cross-reference key (see detailed description of
       \ref parsed_xref)
      - there is no xref_value with the given key
      - the xref_value is still in use, i.e. its use count is not 0 (see
        gedcom_link_xref() and gedcom_unlink_xref())
 */
int gedcom_delete_xref(const char* xrefstr)
{
  struct xref_node *xr = NULL;
  int result = 1;

  if (!is_valid_pointer(xrefstr)) {
    gedcom_error(_("String '%s' is not a valid cross-reference key"), xrefstr);
  }
  else {
    hnode_t *node = hash_lookup(xrefs, xrefstr);
    if (! node) {
      gedcom_error(_("Cross-reference %s not defined"), xrefstr);
    }
    else {
      xr = (struct xref_node*) hnode_get(node);
      if (xr->use_count != 0)  {
	gedcom_error(_("Cross-reference %s still in use"), xrefstr);
      }
      else {
	remove_xref(xr);
	result = 0;
      }
    }
  }
  return result;
}
