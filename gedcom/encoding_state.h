/* Header file for encoding.c.
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

#ifndef __ENCODING_STATE_H
#define __ENCODING_STATE_H

#include "gedcom.h"

#define MAX_CHARSET_LEN 32
#define MAX_TERMINATOR_LEN 2

struct encoding_state {
  char         charset[MAX_CHARSET_LEN + 1];
  const char*  encoding;
  Encoding     width;
  Enc_bom      bom;
  char         terminator[MAX_TERMINATOR_LEN + 1];
};

struct encoding_state read_encoding;
struct encoding_state write_encoding;

void set_read_encoding(const char* charset, const char* encoding);
void set_read_encoding_width(Encoding enc);
void set_read_encoding_bom(Enc_bom bom);
void set_read_encoding_terminator(char* term);

void init_write_encoding();
void init_write_terminator();

#endif /* __ENCODING_STATE_H */
