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

#include "gedcom.h"

extern int compat_at;

enum _COMPAT {
  C_FTREE = 0x01,
  C_LIFELINES = 0x02
};

void set_compatibility(char* program);
int  compat_mode(int flags); 
void compat_generate_submitter_link(Gedcom_ctxt parent);
void compat_generate_submitter();
void compat_generate_gedcom(Gedcom_ctxt parent);
int  compat_generate_char(Gedcom_ctxt parent);
Gedcom_ctxt compat_generate_resi_start(Gedcom_ctxt parent);
void compat_generate_resi_end(Gedcom_ctxt parent, Gedcom_ctxt self);

#endif /* __COMPAT_H */
