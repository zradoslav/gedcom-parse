/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

/* Basic file encoding */
#ifndef __ENCODING_H
#define __ENCODING_H

typedef enum _ENC {
  ONE_BYTE = 0,
  TWO_BYTE_HILO = 1,
  TWO_BYTE_LOHI = 2
} ENCODING;

/* All Unicode characters between U+0000 and U+FFFF can be encoded in
   UTF-8 with 3 or less bytes */
#define UTF_FACTOR 3

int open_conv_to_internal(char* fromcode);
void close_conv_to_internal();
char* to_internal(char* str, size_t len,
		  char* output_buffer, size_t out_len);
void init_encodings();
void set_encoding_width(ENCODING enc);

#endif /* __ENCODING_H */
