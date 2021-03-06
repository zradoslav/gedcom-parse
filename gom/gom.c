/* Main file for building the gedcom object model.
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
#include <stdio.h>
#include "gedcom.h"
#include "header.h"
#include "submitter.h"
#include "submission.h"
#include "family.h"
#include "individual.h"
#include "multimedia.h"
#include "note.h"
#include "repository.h"
#include "source.h"
#include "user_rec.h"
#include "address.h"
#include "event.h"
#include "place.h"
#include "source_citation.h"
#include "multimedia_link.h"
#include "note_sub.h"
#include "lds_event.h"
#include "user_ref.h"
#include "change_date.h"
#include "personal_name.h"
#include "family_link.h"
#include "association.h"
#include "source_event.h"
#include "source_description.h"
#include "gom.h"
#include "gom_internal.h"

void gom_default_callback (Gedcom_elt elt, Gedcom_ctxt parent, int level,
			   char* tag, char* raw_value, int parsed_tag);

void gom_cleanup()
{
  header_cleanup();
  submission_cleanup();
  families_cleanup();
  individuals_cleanup();
  multimedias_cleanup();
  notes_cleanup();
  repositories_cleanup();
  sources_cleanup();
  submitters_cleanup();
  user_recs_cleanup();
}

void subscribe_all()
{
  gedcom_set_default_callback(gom_default_callback);
  header_subscribe();
  submission_subscribe();
  family_subscribe();
  individual_subscribe();
  multimedia_subscribe();
  note_subscribe();
  repository_subscribe();
  source_subscribe();
  submitter_subscribe();
  user_rec_subscribe();
  
  address_subscribe();
  event_subscribe();
  place_subscribe();
  citation_subscribe();
  note_sub_subscribe();
  multimedia_link_subscribe();
  lds_event_subscribe();
  user_ref_subscribe();
  change_date_subscribe();
  name_subscribe();
  family_link_subscribe();
  association_subscribe();
  source_event_subscribe();
  source_description_subscribe();

  if (atexit(gom_cleanup) != 0) {
    gedcom_warning(_("Could not register gom cleanup function"));
  }
}

int gom_active = 0;

/** This function initializes the object model by parsing the given GEDCOM
    file.

    \param file_name  The input file

    \retval 0 on success
    \retval 1 on failure
*/
int gom_parse_file(const char* file_name)
{
  if (gom_active) {
    gom_cleanup();
  }
  else {
    gedcom_set_compat_options(COMPAT_ALLOW_OUT_OF_CONTEXT);
    subscribe_all();
  }
  gom_active = 1;
  return gedcom_parse_file(file_name);
}

/** This function starts an empty model.  It does this by parsing the
    \c new.ged
    file in the data directory of the library (\c $PREFIX/share/gedcom-parse).
    This can be used to start from an empty model, and to build up the model
    by adding new records yourself.

    \retval 0 on success
    \retval nonzero on errors (mainly the errors from
            \ref gedcom_parse_file()).
*/
int gom_new_model()
{
  if (gom_active) {
    gom_cleanup();
  }
  else {
    subscribe_all();
  }
  gom_active = 1;
  return gedcom_new_model();
}

/** This function writes the current Gedcom model to a file.

    \param file_name  The name of the file to write to
    \param total_conv_fails Pass a pointer to an integer if you want to know
    the number of conversion failures (filled in on return).  You can pass
    \c NULL if you're not interested.

    \retval 0 on success
    \retval nonzero on errors
*/
int gom_write_file(const char* file_name, int *total_conv_fails)
{
  Gedcom_write_hndl hndl;
  int result = 1;

  hndl = gedcom_write_open(file_name);
  if (hndl) {
    result = write_header(hndl);
    result |= write_submission(hndl);
    result |= write_submitters(hndl);
    result |= write_individuals(hndl);
    result |= write_families(hndl);
    result |= write_multimedia_recs(hndl);
    result |= write_notes(hndl);
    result |= write_repositories(hndl);
    result |= write_sources(hndl);
    result |= write_user_recs(hndl);
    result |= gedcom_write_close(hndl, total_conv_fails);
  }

  return result;
}

int gom_write_xref_list(Gedcom_write_hndl hndl,
			Gedcom_elt elt, int tag, int parent_rec_or_elt,
			struct xref_list* val)
{
  int result = 0;
  struct xref_list* xrl;
  for (xrl = val; xrl; xrl = xrl->next) {
    result |= gedcom_write_element_xref(hndl, elt, tag, parent_rec_or_elt,
					xrl->xref);
  }
  return result;
}

void gom_default_callback (Gedcom_elt elt UNUSED, Gedcom_ctxt parent UNUSED,
			   int level, char* tag, char* raw_value,
			   int parsed_tag UNUSED)
{
  gedcom_warning(_("Data loss in import: \"%d %s %s\""),
                 level, tag, raw_value);
}

void gom_mem_error(const char *filename, int line)
{
  gedcom_error(_("Could not allocate memory at %s, %d"), filename, line);
}

void gom_xref_already_in_use(const char *xrefstr)
{
  gedcom_error(_("Cross-reference key '%s' is already in use"), xrefstr);
}

void gom_move_error(const char* type)
{
  gedcom_warning(_("Could not move struct of type %s"), type);
}

void gom_find_error(const char* type)
{
  gedcom_warning(_("Could not find struct of type %s in chain"), type);
}

void set_xref_type(struct xref_value* xr, const char *str)
{
  if (!strcasecmp(str, "FAM"))
    xr->type = XREF_FAM;
  else if (!strcasecmp(str, "INDI"))
    xr->type = XREF_INDI;
  else if (!strcasecmp(str, "NOTE"))
    xr->type = XREF_NOTE;
  else if (!strcasecmp(str, "OBJE"))
    xr->type = XREF_OBJE;
  else if (!strcasecmp(str, "REPO"))
    xr->type = XREF_REPO;
  else if (!strcasecmp(str, "SOUR"))
    xr->type = XREF_SOUR;
  else if (!strcasecmp(str, "SUBM"))
    xr->type = XREF_SUBM;
  else if (!strcasecmp(str, "SUBN"))
    xr->type = XREF_SUBN;
  else
    xr->type = XREF_ANY;
}
