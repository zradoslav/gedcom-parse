/* Header file for UTF-8 functions
   Copyright (C) 2001, 2002 Peter Verthez

   The UTF8 tools library is free software; you can redistribute it
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
   02111-1307 USA.
*/

/* $Id$ */
/* $Name$ */

#ifndef __UTF8_H
#define __UTF8_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

struct conv_buffer;
struct convert;
typedef struct conv_buffer *conv_buffer_t;
typedef struct convert *convert_t;

  /* Returns -1 if the string is not a valid UTF-8 string, returns its
     string length otherwise */
int   utf8_strlen(const char* input);

  /* Returns 1 if string is valid UTF-8 string, 0 otherwise */
int   is_utf8_string(const char* input);

  /* Functions for creating and freeing conversion buffers yourself */
conv_buffer_t create_conv_buffer(int size);
void free_conv_buffer(conv_buffer_t buf);
  
  /* General conversion interface (is bidirectional) */
  /* Pass 0 for external_outbuf unless you want to control the
     output buffer yourself */
convert_t initialize_utf8_conversion(const char* charset, int external_outbuf);
int   conversion_set_unknown(convert_t conv, const char* unknown);
int   conversion_set_output_buffer(convert_t conv, conv_buffer_t buf);
void  cleanup_utf8_conversion(convert_t conv);
char* convert_from_utf8(convert_t conv, const char* input, int* conv_fails,
			size_t* output_len);
char* convert_to_utf8(convert_t conv, const char* input, size_t input_len);
char* convert_to_utf8_incremental(convert_t conv,
				  const char* input, size_t input_len);

  /* Specific locale conversion interface (initializes a convert_t structure
     implicitly */
void  convert_set_unknown(const char* unknown);
char* convert_utf8_to_locale(const char* input, int *conv_fails);
char* convert_locale_to_utf8(const char* input);

#ifdef __cplusplus
}
#endif

#endif /* __UTF8_H */
