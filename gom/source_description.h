/* Include file for the source description substructure in the gedcom object
   model.
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

#ifndef __SOURCE_DESCRIPTION_H
#define __SOURCE_DESCRIPTION_H

#include "gom.h"
#include "gom_internal.h"

void source_description_subscribe();
int write_source_descriptions(Gedcom_write_hndl hndl, int parent,
			      struct source_description *desc);

DECLARE_SUB_MAKEFUNC(source_description);

DECLARE_UNREFALLFUNC(source_description);
DECLARE_CLEANFUNC(source_description);
DECLARE_ADDFUNC2(source_description, user_data);

#endif /* __SOURCE_DESCRIPTION_H */
