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

/* Basic file encoding */
#ifndef __ENCODING_H
#define __ENCODING_H

#include "gedcom.h"
#include "utf8.h"

int open_conv_to_internal(const char* fromcode);
void close_conv_to_internal();
char* to_internal(const char* str, size_t len, struct conv_buffer *output_buf);
void init_encodings();
char* get_encoding(const char* gedcom_n, Encoding enc);
void set_encoding_width(Encoding enc);
void update_gconv_search_path();

#endif /* __ENCODING_H */
