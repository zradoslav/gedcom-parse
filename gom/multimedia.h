/* Include file for the multimedia object in the gedcom object model.
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

#ifndef __MULTIMEDIA_H
#define __MULTIMEDIA_H

#include "gom.h"
#include "gom_internal.h"

void multimedia_subscribe();
void multimedias_cleanup();
int write_multimedia_recs(Gedcom_write_hndl hndl);

DECLARE_MAKEFUNC(multimedia);
DECLARE_ADDFUNC2(multimedia, note_sub);
DECLARE_ADDFUNC2(multimedia, user_ref_number);
DECLARE_ADDFUNC2(multimedia, user_data);
DECLARE_ADDFUNC2_NOLIST(multimedia, change_date);
DECLARE_ADDFUNC2_STR(multimedia, record_id);

#endif /* __MULTIMEDIA_H */
