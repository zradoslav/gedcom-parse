/* Encoding utility from UTF-8 to another charset and vice versa
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

#include "utf8tools.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include "config.h"

#define INITIAL_BUFSIZE 256
#define DEFAULT_UNKNOWN "?"

#define INTERNAL_BUFFER 0
#define EXTERNAL_BUFFER 1

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

static void reset_conv_buffer(conv_buffer_t buf)
{
  memset(buf->buffer, 0, buf->size);
}

conv_buffer_t create_conv_buffer(int size)
{
  struct conv_buffer* buf = NULL;

  if (size == 0) size = INITIAL_BUFSIZE;
  
  buf = (struct conv_buffer*) malloc(sizeof(struct conv_buffer));
  if (buf) {
    buf->size   = size;
    buf->buffer = (char*)malloc(size);
    buf->type   = EXTERNAL_BUFFER;
    if (buf->buffer)
      reset_conv_buffer(buf);
    else
      buf->size = 0;
  }

  return buf;
}

void free_conv_buffer(conv_buffer_t buf)
{
  if (buf) {
    free(buf->buffer);
    free(buf);
  }
}

static char* grow_conv_buffer(conv_buffer_t buf, char* curr_pos)
{
  size_t outlen, new_size;
  char*  new_buffer;
  outlen     = curr_pos - buf->buffer;
  new_size   = buf->size * 2;
  new_buffer = realloc(buf->buffer, new_size);
  if (new_buffer) {
    buf->buffer = new_buffer;
    buf->size   = new_size;
    curr_pos    = buf->buffer + outlen;
    memset(curr_pos, 0, buf->size - (curr_pos - buf->buffer));
    return curr_pos;
  }
  else
    return NULL;
}

convert_t initialize_utf8_conversion(const char* charset, int external_outbuf)
{
  struct convert *conv = NULL;
  int save_errno = 0;
  int cleanup = 0;

  conv = (struct convert *)malloc(sizeof(struct convert));
  if (conv) {
    /* Unless reset to 0 at the end, this will force cleanup */
    cleanup = 1;
    /* First initialize to default values */
    conv->from_utf8  = (iconv_t)-1;
    conv->to_utf8    = (iconv_t)-1;
    conv->inbuf      = NULL;
    conv->insize     = 0;
    conv->outbuf     = NULL;
    conv->unknown    = NULL;

    /* Now initialize everything to what it should be */
    conv->from_utf8 = iconv_open(charset, "UTF-8");
    if (conv->from_utf8 != (iconv_t)-1) {
      conv->to_utf8 = iconv_open("UTF-8", charset);
      if (conv->to_utf8 != (iconv_t)-1) {
	conv->unknown = strdup(DEFAULT_UNKNOWN);
	if (conv->unknown) {
	  conv->inbuf = create_conv_buffer(INITIAL_BUFSIZE);
	  conv->inbuf->type = INTERNAL_BUFFER;
	  if (conv->inbuf) {
	    if (external_outbuf)
	      cleanup = 0;
	    else {
	      conv->outbuf = create_conv_buffer(INITIAL_BUFSIZE);
	      conv->outbuf->type = INTERNAL_BUFFER;
	      if (conv->outbuf)
		cleanup = 0;    /* All successful */
	    }
	  }
	}
      }
    }
  }

  if (cleanup) {
    save_errno = errno;
    cleanup_utf8_conversion(conv);
    errno = save_errno;
    conv = NULL;
  }
  
  return conv;
}

int conversion_set_unknown(convert_t conv, const char* unknown)
{
  int result = 1;
  
  if (conv && unknown) {
    char* unknown_copy = strdup(unknown);
    if (unknown_copy) {
      if (conv->unknown) free(conv->unknown);
      conv->unknown = unknown_copy;
    }
    else
      result = 0;
  }

  return result;
}

int conversion_set_output_buffer(convert_t conv, conv_buffer_t buf)
{
  if (!conv)
    return 0;
  else if ((!conv->outbuf || conv->outbuf->type == EXTERNAL_BUFFER)
	   && buf && buf->type == EXTERNAL_BUFFER) {
    conv->outbuf = buf;
    return 1;
  }
  else
    return 0;
}

void cleanup_utf8_conversion(convert_t conv)
{
  if (conv) {
    if (conv->from_utf8 != (iconv_t)-1)
      iconv_close(conv->from_utf8);
    if (conv->to_utf8 != (iconv_t)-1)
      iconv_close(conv->to_utf8);
    if (conv->inbuf && conv->inbuf->type == INTERNAL_BUFFER)
      free_conv_buffer(conv->inbuf);
    if (conv->outbuf && conv->outbuf->type == INTERNAL_BUFFER)
      free_conv_buffer(conv->outbuf);
    if (conv->unknown)
      free(conv->unknown);
    free(conv);
  }
}

char* convert_from_utf8(convert_t conv, const char* input, int* conv_fails,
			size_t* output_len)
{
  size_t insize;
  size_t outsize;
  ICONV_CONST char* inptr  = (ICONV_CONST char*) input;
  char   *outptr;
  size_t nconv;
  struct conv_buffer* outbuf;

  if (!conv || !conv->outbuf || !input) {
    if (conv_fails != NULL) *conv_fails = (input ? strlen(input) : 0);
    return NULL;
  }
  insize = strlen(input);
  /* make sure we start from an empty state */
  iconv(conv->from_utf8, NULL, NULL, NULL, NULL);
  if (conv_fails != NULL) *conv_fails = 0;
  /* set up output buffer (empty it) */
  outbuf  = conv->outbuf;
  outptr  = outbuf->buffer;
  outsize = outbuf->size;
  reset_conv_buffer(conv->outbuf);
  nconv = iconv(conv->from_utf8, &inptr, &insize, &outptr, &outsize);
  while (nconv == (size_t)-1) {
    if (errno == E2BIG) {
      /* grow the output buffer */
      outptr  = grow_conv_buffer(outbuf, outptr);
      if (outptr)
	outsize = outbuf->size - (outptr - outbuf->buffer);
      else {
	errno = ENOMEM;
	return NULL;
      }
    }
    else if (errno == EILSEQ) {
      /* skip over character */
      const char* unkn_ptr = conv->unknown;
      if (conv_fails != NULL) (*conv_fails)++;
      if ((*inptr & 0x80) == 0) {
	/* an ASCII character, just skip one (this case is very improbable) */
	inptr++; insize--;
      }
      else {
	/* a general UTF-8 character, skip all 0x10xxxxxx bytes */
	inptr++; insize--;
	while ((*inptr & 0xC0) == 0x80) {
	  inptr++; insize--;
	}
      }
      /* append the "unknown" string to the output */
      while (*unkn_ptr) { *outptr++ = *unkn_ptr++; outsize--; }
    }
    else {
      /* EINVAL should not happen, since we convert entire strings */
      /* EBADF is an error which should be captured by the first if above */
      if (conv_fails != NULL) *conv_fails += insize;
      return NULL;
    }
    nconv = iconv(conv->from_utf8, &inptr, &insize, &outptr, &outsize);
  }
  if (output_len) *output_len = outptr - outbuf->buffer;
  return outbuf->buffer;
}

char* convert_to_utf8(convert_t conv, const char* input, size_t input_len)
{
  size_t outsize;
  ICONV_CONST char *inptr  = (ICONV_CONST char*) input;
  char   *outptr;
  size_t nconv;
  struct conv_buffer* outbuf;

  if (!conv || !conv->outbuf || !input)
    return NULL;
  /* make sure we start from an empty state */
  iconv(conv->to_utf8, NULL, NULL, NULL, NULL);
  /* set up output buffer (empty it) */
  outbuf  = conv->outbuf;
  outptr  = outbuf->buffer;
  outsize = outbuf->size;
  reset_conv_buffer(conv->outbuf);
  nconv = iconv(conv->to_utf8, &inptr, &input_len, &outptr, &outsize);
  while (nconv == (size_t)-1) {
    if (errno == E2BIG) {
      /* grow the output buffer */
      outptr  = grow_conv_buffer(outbuf, outptr);
      if (outptr)
	outsize = outbuf->size - (outptr - outbuf->buffer);
      else {
	errno = ENOMEM;
	return NULL;
      }
    }
    else {
      /* EILSEQ happens when the input doesn't match the source encoding,
         return NULL in this case */
      /* EINVAL should not happen, since we convert entire strings */
      /* EBADF is an error which should be captured by the first if above */
      return NULL;
    }
    nconv = iconv(conv->to_utf8, &inptr, &input_len, &outptr, &outsize);
  }
  return outbuf->buffer;  
}

char* convert_to_utf8_incremental(convert_t conv,
				  const char* input, size_t input_len)
{
  size_t res;
  struct conv_buffer* outbuf = conv->outbuf;
  struct conv_buffer* inbuf  = conv->inbuf;
  size_t outsize = outbuf->size;
  char* wrptr = outbuf->buffer;
  ICONV_CONST char* rdptr = (ICONV_CONST char*) inbuf->buffer;
  char* retval = outbuf->buffer;

  if (!conv || !conv->outbuf)
    return NULL;
  
  if (!input) {
    iconv(conv->to_utf8, NULL, NULL, NULL, NULL);
    reset_conv_buffer(inbuf);
    conv->insize = 0;
    return NULL;
  }
  
  /* set up input buffer (concatenate to what was left previous time) */
  /* can't use strcpy, because possible null bytes from unicode */
  while (conv->insize + input_len > inbuf->size)
    grow_conv_buffer(inbuf, inbuf->buffer + conv->insize);
  memcpy(inbuf->buffer + conv->insize, input, input_len);
  conv->insize += input_len;

  /* set up output buffer (empty it) */
  reset_conv_buffer(outbuf);

  /* do the conversion */
  res = iconv(conv->to_utf8, &rdptr, &conv->insize, &wrptr, &outsize);
  if (res == (size_t)-1) {
    if (errno == EILSEQ) {
      /* restart from an empty state and return NULL */
      retval = NULL;
      rdptr++;
      conv->insize--;
    }
    else if (errno == EINVAL) {
      /* Do nothing, leave it to next iteration */
    }
    else {
      retval = NULL;
    }
  }

  /* then shift what is left over to the head of the input buffer */
  memmove(inbuf->buffer, rdptr, conv->insize);
  memset(inbuf->buffer + conv->insize, 0, inbuf->size - conv->insize);
  return retval;
}
