/* Header for compatibility handling for the GEDCOM parser.
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

#ifndef __COMPAT_H
#define __COMPAT_H

#include "buffer.h"
#include "gedcom.h"

typedef enum _COMPAT_RULES {
  C_NO_SUBMITTER,
  C_INDI_ADDR,
  C_NOTE_NO_VALUE,
  C_NO_GEDC,
  C_NO_CHAR,
  C_HEAD_TIME,
  C_NO_DOUBLE_AT,
  C_NO_REQUIRED_VALUES,
  C_551_TAGS,
  C_NO_SLGC_FAMC,
  C_SUBM_COMM,
  C_DOUBLE_DATES_4,
  C_NR_OF_RULES
} Compat_rule;

void set_compatibility_program(const char* program);
void set_compatibility_version(const char* version);
void compute_compatibility();
int  compat_mode(Compat_rule rule);

void compat_generate_submitter_link(Gedcom_ctxt parent);
void compat_generate_submitter();

void compat_generate_gedcom(Gedcom_ctxt parent);

int  compat_generate_char(Gedcom_ctxt parent);

Gedcom_ctxt compat_generate_resi_start(Gedcom_ctxt parent);
void compat_generate_resi_end(Gedcom_ctxt parent, Gedcom_ctxt self);

int  compat_check_551_tag(const char* tag, struct safe_buffer* b);

void compat_generate_slgc_famc_link(Gedcom_ctxt parent);
void compat_generate_slgc_famc_fam();

int  compat_check_subm_comm(const char* tag, const char* parent_tag,
			    struct safe_buffer* b);
void compat_close_subm_comm();
int  compat_check_subm_comm_cont(const char* tag);
Gedcom_ctxt compat_subm_comm_cont_start(Gedcom_ctxt parent, const char* str);
void compat_subm_comm_cont_end(Gedcom_ctxt parent, Gedcom_ctxt self);

#endif /* __COMPAT_H */
