/* Compatibility handling for the GEDCOM parser.
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

#include "compat.h"
#include "interface.h"
#include "encoding.h"
#include "xref.h"
#include "gedcom_internal.h"
#include "gedcom.h"

int compat_enabled = 1;
int compatibility  = 0; 
int compat_at = 0;
const char* default_charset = "";

#define SUBMITTER_LINK         "@__COMPAT__SUBM__@"
#define DEFAULT_SUBMITTER_NAME "Submitter"
#define DEFAULT_GEDCOM_VERS    "5.5"
#define DEFAULT_GEDCOM_FORM    "LINEAGE-LINKED"

/* Incompatibility list (with GEDCOM 5.5):

    - ftree:
        - no submitter record, no submitter link in the header
	- INDI.ADDR instead of INDI.RESI.ADDR
	- NOTE doesn't have a value

    - Lifelines (3.0.2):
        - no submitter record, no submitter link in the header
	- no GEDC field in the header
	- no CHAR field in the header
	- HEAD.TIME instead of HEAD.DATE.TIME (will be ignored here)
	- '@' not written as '@@' in values
	- lots of missing required values
 */

/* Compatibility handling */

void gedcom_set_compat_handling(int enable_compat)
{
  compat_enabled = enable_compat;
}

void set_compatibility(const char* program)
{
  /* Reinitialize compatibility */
  compat_at = 0;
  default_charset = "";
  compatibility = 0;
  
  if (compat_enabled) {
    if (! strncmp(program, "ftree", 6)) {
      gedcom_warning(_("Enabling compatibility with 'ftree'"));
      compatibility = C_FTREE;
    }
    else if (! strncmp(program, "LIFELINES", 9)) {
      /* Matches "LIFELINES 3.0.2" */
      gedcom_warning(_("Enabling compatibility with 'Lifelines'"));
      compatibility = C_LIFELINES;
      default_charset = "ANSI";
      compat_at = 1;
    }
  }
}

int compat_mode(int compat_flags)
{
  return (compat_flags & compatibility);
}

void compat_generate_submitter_link(Gedcom_ctxt parent)
{
  struct xref_value *xr = gedcom_parse_xref(SUBMITTER_LINK, XREF_USED,
					    XREF_SUBM);
  struct tag_struct ts;
  Gedcom_ctxt self;
  
  ts.string = "SUBM";
  ts.value  = TAG_SUBM;
  self = start_element(ELT_HEAD_SUBM,
		       parent, 1, ts, SUBMITTER_LINK,
		       GEDCOM_MAKE_XREF_PTR(val1, xr));
  end_element(ELT_HEAD_SUBM, parent, self, NULL);
}

void compat_generate_submitter()
{
  struct xref_value *xr = gedcom_parse_xref(SUBMITTER_LINK, XREF_DEFINED,
					    XREF_SUBM);
  struct tag_struct ts;
  Gedcom_ctxt self1, self2;

  /* first generate "0 SUBM" */
  ts.string = "SUBM";
  ts.value  = TAG_SUBM;
  self1 = start_record(REC_SUBM, 0, GEDCOM_MAKE_XREF_PTR(val1, xr), ts,
		       NULL, GEDCOM_MAKE_NULL(val2));

  /* then generate "1 NAME ..." */
  ts.string = "NAME";
  ts.value  = TAG_NAME;
  self2 = start_element(ELT_SUBM_NAME, self1, 1, ts, DEFAULT_SUBMITTER_NAME,
			GEDCOM_MAKE_STRING(val1, DEFAULT_SUBMITTER_NAME));

  /* close "1 NAME ..." */
  end_element(ELT_SUBM_NAME, self1, self2, NULL);

  /* close "0 SUBM" */
  end_record(REC_SUBM, self1, NULL);
}

void compat_generate_gedcom(Gedcom_ctxt parent)
{
  struct tag_struct ts;
  Gedcom_ctxt self1, self2;
  
  /* first generate "1 GEDC" */
  ts.string = "GEDC";
  ts.value  = TAG_GEDC;
  self1 = start_element(ELT_HEAD_GEDC, parent, 1, ts, NULL,
			GEDCOM_MAKE_NULL(val1));
  
  /* then generate "2 VERS <DEFAULT_GEDC_VERS>" */
  ts.string = "VERS";
  ts.value  = TAG_VERS;
  self2 = start_element(ELT_HEAD_GEDC_VERS, self1, 2, ts,
			DEFAULT_GEDCOM_VERS,
			GEDCOM_MAKE_STRING(val1, DEFAULT_GEDCOM_VERS));
  
  /* close "2 VERS" */
  end_element(ELT_HEAD_GEDC_VERS, self1, self2, NULL);
  
  /* then generate "2 FORM <DEFAULT_GEDCOM_FORM> */
  ts.string = "FORM";
  ts.value  = TAG_FORM;
  self2 = start_element(ELT_HEAD_GEDC_FORM, self1, 2, ts,
			DEFAULT_GEDCOM_FORM,
			GEDCOM_MAKE_STRING(val1, DEFAULT_GEDCOM_FORM));
  
  /* close "2 FORM" */
  end_element(ELT_HEAD_GEDC_FORM, self1, self2, NULL);
  
  /* close "1 GEDC" */
  end_element(ELT_HEAD_GEDC, parent, self1, NULL);
}

int compat_generate_char(Gedcom_ctxt parent)
{
  struct tag_struct ts;
  Gedcom_ctxt self1;
  char* charset;
  
  /* first generate "1 CHAR <DEFAULT_CHAR>" */
  ts.string = "CHAR";
  ts.value  = TAG_CHAR;

  /* Must strdup, because default_charset is const char */
  charset   = strdup(default_charset);
  if (! charset)
    MEMORY_ERROR;
  else {
    self1 = start_element(ELT_HEAD_CHAR, parent, 1, ts, charset,
			  GEDCOM_MAKE_STRING(val1, charset));
    free(charset);
    
    /* close "1 CHAR" */
    end_element(ELT_HEAD_CHAR, parent, self1, NULL);
  }
  if (open_conv_to_internal(default_charset) == 0)
    return 1;
  else
    return 0;
}

Gedcom_ctxt compat_generate_resi_start(Gedcom_ctxt parent)
{
  Gedcom_ctxt self;
  struct tag_struct ts;

  ts.string = "RESI";
  ts.value  = TAG_RESI;
  self = start_element(ELT_SUB_INDIV_RESI, parent, 1, ts, NULL,
		       GEDCOM_MAKE_NULL(val1));
  return self;
}

void compat_generate_resi_end(Gedcom_ctxt parent, Gedcom_ctxt self)
{
  end_element(ELT_SUB_INDIV_RESI, parent, self, NULL);
}
