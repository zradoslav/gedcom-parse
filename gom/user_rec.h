/* Include file for the user record object in the gedcom object model.
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

#ifndef __USER_REC_H
#define __USER_REC_H

#include "gom.h"
#include "gom_internal.h"

void user_rec_subscribe();
void user_recs_cleanup();
int write_user_recs(Gedcom_write_hndl hndl);
int write_user_data(Gedcom_write_hndl hndl, struct user_data* data);

DECLARE_MAKEFUNC(user_rec);
DECLARE_ADDFUNC2(user_rec, user_data);

DECLARE_UNREFALLFUNC(user_data);
DECLARE_CLEANFUNC(user_data);

#endif /* __USER_REC_H */
