/* Header for age manipulation routines.
   Copyright (C) 2001 The Genes Development Team
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

#ifndef __AGE_H
#define __AGE_H

#include <stdlib.h>
#include "gedcom_internal.h"
#include "gedcom.h"

extern struct age_value age_s;

void copy_age(struct age_value *to, struct age_value from);

#define GEDCOM_MAKE_AGE(VAR, AGE) \
   GEDCOM_MAKE(VAR, AGE, GV_AGE_VALUE, age_val)

#endif /* __AGE_H */
