/* Implementation of the interface to applications.
   Copyright (C) 2001, 2002 The Genes Development Team
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
#include "interface.h"

static Gedcom_rec_start_cb record_start_callback [NR_OF_RECS] = { NULL };
static Gedcom_rec_end_cb   record_end_callback   [NR_OF_RECS] = { NULL };
static Gedcom_elt_start_cb element_start_callback[NR_OF_ELTS] = { NULL };
static Gedcom_elt_end_cb   element_end_callback  [NR_OF_ELTS] = { NULL };
static Gedcom_def_cb       default_cb                         = NULL;

/** This function allows to set the default callback.  You can only register
    one default callback.
    \param func The default callback.
 */
void gedcom_set_default_callback(Gedcom_def_cb func)
{
  if (default_cb) {
    gedcom_error(_("Internal error: Duplicate registration for default callback"));
  }
  default_cb = func;
}

/** This function allows to subscribe to a record of a certain type, with a
    start and an end
    callback.  The end callback is optional: you can pass \c NULL if you are
    not interested in the end callback.  You can only register once for a
    given record type.
    \param rec The record to subscribe to (see the
    <a href="interface.html#Record_identifiers">interface details</a>)
    \param cb_start The start callback
    \param cb_end The end callback
 */
void gedcom_subscribe_to_record(Gedcom_rec rec,
				Gedcom_rec_start_cb cb_start,
				Gedcom_rec_end_cb cb_end)
{
  if (record_start_callback[rec] || record_end_callback[rec])
    gedcom_error(_("Internal error: Duplicate registration for record type %d"), rec);
  if (cb_start) {
    record_start_callback[rec] = cb_start;
    record_end_callback[rec]   = cb_end;
  }
}

/** This function allows to subscribe to an element of a certain type, with a
    start and an end
    callback.  The end callback is optional: you can pass \c NULL if you are
    not interested in the end callback.  You can only register once for a given
    element type.
    \param elt The element to subscribe to (see the
    <a href="interface.html#Element_identifiers">interface details</a>)
    \param cb_start The start callback
    \param cb_end The end callback
 */
void gedcom_subscribe_to_element(Gedcom_elt elt,
				 Gedcom_elt_start_cb cb_start,
				 Gedcom_elt_end_cb cb_end)
{
  if (element_start_callback[elt] || element_end_callback[elt])
    gedcom_error(_("Internal error: Duplicate registration for element type %d"), elt);
  if (cb_start) {
    element_start_callback[elt] = cb_start;
    element_end_callback[elt]   = cb_end;
  }
}

Gedcom_ctxt start_record(Gedcom_rec rec,
			 int level, Gedcom_val xref, struct tag_struct tag,
			 char *raw_value, Gedcom_val parsed_value)
{
  Gedcom_rec_start_cb cb = record_start_callback[rec];
  if (cb != NULL)
    return (*cb)(rec, level, xref, tag.string, raw_value, tag.value,
		 parsed_value);
  else
    return NULL;
}

void end_record(Gedcom_rec rec, Gedcom_ctxt self, Gedcom_val parsed_value)
{
  Gedcom_rec_end_cb cb = record_end_callback[rec];
  if (cb != NULL)
    (*cb)(rec, self, parsed_value);
}

Gedcom_ctxt start_element(Gedcom_elt elt, Gedcom_ctxt parent, 
			  int level, struct tag_struct tag, char *raw_value,
			  Gedcom_val parsed_value)
{
  Gedcom_elt_start_cb cb = element_start_callback[elt];
  Gedcom_ctxt ctxt = parent;
  if (cb != NULL)
    ctxt = (*cb)(elt, parent, level, tag.string, raw_value,
		 tag.value, parsed_value);
  else if (default_cb != NULL && parent != NULL)
    (*default_cb)(elt, parent, level, tag.string, raw_value, tag.value);
  return ctxt;
}

void end_element(Gedcom_elt elt, Gedcom_ctxt parent, Gedcom_ctxt self,
		 Gedcom_val parsed_value)
{
  Gedcom_elt_end_cb cb = element_end_callback[elt];
  if (cb != NULL)
    (*cb)(elt, parent, self, parsed_value);
}

const char* val_type_str[] = { N_("null value"),
			       N_("character string"),
			       N_("date"),
			       N_("age"),
			       N_("cross-reference") };

void gedcom_cast_error(const char* file, int line,
		       Gedcom_val_type tried_type,
		       Gedcom_val_type real_type)
{
  int tried_bit=0, real_bit=0;
  while (tried_type && tried_type % 2 == 0) {
    tried_bit++;
    tried_type >>= 1;
  }
  while (real_type && real_type % 2 == 0) {
    real_bit++;
    real_type >>= 1;
  }
  gedcom_warning
    (_("Wrong cast of value in file %s, at line %d: %s instead of %s"),
     file, line, _(val_type_str[tried_bit]), _(val_type_str[real_bit]));
}

/** This function allows to customize what happens on an error.  It doesn't
    influence the generation of error or warning messages, only the behaviour
    of the parser and its return code.
    \param mechanism The mechanism to be used; see \ref Gedcom_err_mech
    for the possible mechanisms.
 */
void gedcom_set_error_handling(Gedcom_err_mech mechanism)
{
  error_mechanism = mechanism;
}

/** This function allows to change the debug level.

    \param level  The debug level, one of the following values:
      - 0: no debugging information (this is the default)
      - 1: only debugging information from libgedcom itself
      - 2: debugging information from libgedcom and yacc
      
    \param f A file handle (which must be open) to write debugging information
    to; if \c NULL is passed, \c stderr will be used.
*/
void gedcom_set_debug_level(int level, FILE* f)
{
  if (f != NULL)
    trace_output = f;
  else
    trace_output = stderr;
  if (level > 0) {
    gedcom_high_level_debug = 1;
  }
  if (level > 1) {
    gedcom_enable_internal_debug();
  }
}

