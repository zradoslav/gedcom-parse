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

int gom_parse_file(const char* file_name)
{
  subscribe_all();
  return gedcom_parse_file(file_name);
}

int gom_new_model()
{
  subscribe_all();
  return gedcom_new_model();
}

int gom_write_file(const char* file_name, int *total_conv_fails)
{
  Gedcom_write_hndl hndl;
  int result = 1;

  hndl = gedcom_write_open(file_name);
  if (hndl) {
    result = write_header(hndl);
    result |= gedcom_write_close(hndl, total_conv_fails);
  }

  return result;
}

Gom_ctxt make_gom_ctxt(int ctxt_type, OBJ_TYPE obj_type, void *ctxt_ptr)
{
  Gom_ctxt ctxt   = (Gom_ctxt)malloc(sizeof(struct Gom_ctxt_struct));
  if (! ctxt)
    MEMORY_ERROR;
  else {
    ctxt->ctxt_type = ctxt_type;
    ctxt->obj_type  = obj_type;
    ctxt->ctxt_ptr  = ctxt_ptr;
  }
  return ctxt;
}

void NULL_DESTROY(void* anything UNUSED)
{
}

void destroy_gom_ctxt(Gom_ctxt ctxt)
{
  SAFE_FREE(ctxt);
}

void gom_cast_error(const char* file, int line,
		    OBJ_TYPE expected, OBJ_TYPE found)
{
  fprintf(stderr,
	  "Wrong gom ctxt cast at %s, line %d: expected %d, found %d\n",
	  file, line, expected, found);
  abort();
}

void gom_mem_error(const char *filename, int line)
{
  gedcom_error(_("Could not allocate memory at %s, %d"), filename, line);
}

void gom_unexpected_context(const char* file, int line, OBJ_TYPE found)
{
  gedcom_warning(_("Internal error: Unexpected context at %s, line %d: %d"),
		 file, line, found);
}

void gom_no_context(const char* file, int line)
{
  gedcom_warning(_("Internal error: No context at %s, line %d"),
		 file, line);
}

void gom_default_callback (Gedcom_elt elt UNUSED, Gedcom_ctxt parent UNUSED,
			   int level, char* tag, char* raw_value,
			   int parsed_tag UNUSED)
{
  gedcom_warning(_("Data loss in import: \"%d %s %s\""),
                 level, tag, raw_value);
}

void def_rec_end(Gedcom_rec rec UNUSED, Gedcom_ctxt self,
		 Gedcom_val parsed_value UNUSED)
{
  Gom_ctxt ctxt = (Gom_ctxt)self;
  destroy_gom_ctxt(ctxt);
}

void def_elt_end(Gedcom_elt elt UNUSED, Gedcom_ctxt parent UNUSED,
		 Gedcom_ctxt self, Gedcom_val parsed_value UNUSED)
{
  Gom_ctxt ctxt = (Gom_ctxt)self;
  destroy_gom_ctxt(ctxt);
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

char* concat_strings(NL_TYPE type, char *str1, const char *str2)
{
  if (str1 != NULL && str2 != NULL) {
    char *newp;
    char *wp;
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    size_t len  = len1 + len2 + 1;
    if (type == WITH_NL)
      len++;
    newp = (char*) realloc(str1, len);
    if (newp == NULL)
      return NULL;
    wp   = newp + len1;
    str1 = newp;
    if (type == WITH_NL)
      *wp++ = '\n';
    wp = memcpy (wp, str2, len2);
    wp += len2;
    *wp++ = '\0';
  }

  return str1;
}

struct date_value* dup_date(struct date_value dv)
{
  struct date_value* dv_ptr;
  dv_ptr = (struct date_value*) malloc(sizeof(struct date_value));
  if (! dv_ptr)
    MEMORY_ERROR;
  else {
    memcpy(dv_ptr, &dv, sizeof(struct date_value));
  }
  return dv_ptr;
}

struct age_value* dup_age(struct age_value age)
{
  struct age_value* age_ptr;
  age_ptr = (struct age_value*) malloc(sizeof(struct age_value));
  if (! age_ptr)
    MEMORY_ERROR;
  else {
    memcpy(age_ptr, &age, sizeof(struct age_value));
  }
  return age_ptr;
}
