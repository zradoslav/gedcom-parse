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
#include "xref.h"
#include "gedcom_internal.h"
#include "gedcom.h"

#define SUBMITTER_LINK "@__COMPAT__SUBM__@"
#define DEFAULT_SUBMITTER_NAME "Submitter"

/* Incompatibily list (with GEDCOM 5.5):

    - ftree:
        - no submitter record, no submitter link in the header
	- INDI.ADDR instead of INDI.RESI.ADDR
	- NOTE doesn't have a value

 */

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
  end_record(REC_SUBM, self1);
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
