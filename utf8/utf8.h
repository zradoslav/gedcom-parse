/* Header file for UTF-8 functions
   Copyright (C) 2001, 2002 Peter Verthez

   Permission granted to do anything with this file that you want, as long
   as the above copyright is retained in all copies.
   THERE IS NO WARRANTY - USE AT YOUR OWN RISK
*/

/* $Id$ */
/* $Name$ */

#ifndef __UTF8_H
#define __UTF8_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iconv.h"

struct conv_buffer {
  char*   buffer;
  size_t  size;
  int     type;  /* For internal use */
};

struct convert {
  iconv_t from_utf8;
  iconv_t to_utf8;
  struct conv_buffer* inbuf;
  size_t  insize;
  struct conv_buffer* outbuf;
  char*   unknown;
};

typedef struct convert *convert_t;

  /* Returns -1 if the string is not a valid UTF-8 string, returns its
     string length otherwise */
int   utf8_strlen(const char* input);

  /* Returns 1 if string is valid UTF-8 string, 0 otherwise */
int   is_utf8_string(const char* input);

  /* Functions for creating and freeing conversion buffers yourself */
struct conv_buffer* create_conv_buffer(int size);
void free_conv_buffer(struct conv_buffer* buf);
  
  /* General conversion interface (is bidirectional) */
  /* Pass 0 for external_outbuf unless you want to control the
     output buffer yourself */
convert_t initialize_utf8_conversion(const char* charset, int external_outbuf);
int   conversion_set_unknown(convert_t conv, const char* unknown);
int   conversion_set_output_buffer(convert_t conv, struct conv_buffer* buf);
void  cleanup_utf8_conversion(convert_t conv);
char* convert_from_utf8(convert_t conv, const char* input, int* conv_fails);
char* convert_to_utf8(convert_t conv, const char* input);
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
