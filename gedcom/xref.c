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

hnode_t *xref_alloc(void *c __attribute__((unused)))
{
  return malloc(sizeof *xref_alloc(NULL));
}

void xref_free(hnode_t *n, void *c __attribute__((unused)))
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
  const char *key = strdup(xrefstr);
  if (key) {
    xr = make_xref_node();
    xr->xref.type = xref_type;
    xr->xref.object = object;
    if (xr->xref.string)
      free(xr->xref.string);
    xr->xref.string = strdup(xrefstr);
    if (! xr->xref.string) MEMORY_ERROR;
    hash_alloc_insert(xrefs, key, xr);
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

  set_xref_fields(xr, ctxt, xref_type);
  return &(xr->xref);
}

/* Functions for retrieving, modifying and deleting cross-references */

struct xref_value* gedcom_get_by_xref(const char *key)
{
  if (gedcom_check_token(key, STATE_NORMAL, POINTER) != 0) {
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

struct xref_value* gedcom_add_xref(Xref_type type, const char* xrefstr,
				   Gedcom_ctxt object)
{
  struct xref_node *xr = NULL;

  if (gedcom_check_token(xrefstr, STATE_NORMAL, POINTER) != 0) {
    gedcom_error(_("String '%s' is not a valid cross-reference key"), xrefstr);
  }
  else {
    hnode_t *node = hash_lookup(xrefs, xrefstr);
    if (node) {
      gedcom_error(_("Cross-reference %s already exists"), xrefstr);
    }
    else {
      xr = add_xref(type, xrefstr, object);    
      set_xref_fields(xr, XREF_DEFINED, type);
    }
  }
  if (xr)
    return &(xr->xref);
  else
    return NULL;
}

struct xref_value* gedcom_link_xref(Xref_type type, const char* xrefstr)
{
  struct xref_node *xr = NULL;

  if (gedcom_check_token(xrefstr, STATE_NORMAL, POINTER) != 0) {
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

struct xref_value* gedcom_unlink_xref(Xref_type type, const char* xrefstr)
{
  struct xref_node *xr = NULL;
  if (gedcom_check_token(xrefstr, STATE_NORMAL, POINTER) != 0) {
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

int gedcom_delete_xref(const char* xrefstr)
{
  struct xref_node *xr = NULL;
  int result = 1;

  if (gedcom_check_token(xrefstr, STATE_NORMAL, POINTER) != 0) {
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
