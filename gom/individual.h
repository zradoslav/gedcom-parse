/* Include file for the individual object in the gedcom object model.
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

#ifndef __INDIVIDUAL_H
#define __INDIVIDUAL_H

#include "gom.h"
#include "gom_internal.h"

void individual_subscribe();
void individuals_cleanup();
int write_individuals(Gedcom_write_hndl hndl);

DECLARE_MAKEFUNC(individual);
DECLARE_ADDFUNC2(individual, event);
DECLARE_ADDFUNC2(individual, personal_name);
DECLARE_ADDFUNC2(individual, lds_event);
DECLARE_ADDFUNC2(individual, association);
DECLARE_ADDFUNC2(individual, source_citation);
DECLARE_ADDFUNC2(individual, multimedia_link);
DECLARE_ADDFUNC2(individual, note_sub);
DECLARE_ADDFUNC2(individual, user_ref_number);
DECLARE_ADDFUNC2(individual, user_data);
DECLARE_ADDFUNC2_TOVAR(individual, event, attribute);
DECLARE_ADDFUNC2_NOLIST(individual, change_date);
DECLARE_ADDFUNC2_STR(individual, record_id);

void individual_add_family_link(Gom_ctxt ctxt, int ctxt_type,
				struct family_link* link);

#endif /* __INDIVIDUAL_H */
