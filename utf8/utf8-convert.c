/* Encoding utility from UTF-8 to locale and vice versa
   Copyright (C) 2001, 2002 Peter Verthez

   Permission granted to do anything with this file that you want, as long
   as the above copyright is retained in all copies.
   THERE IS NO WARRANTY - USE AT YOUR OWN RISK
*/

/* $Id$ */
/* $Name$ */

#include "utf8.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include "config.h"

#define INITIAL_OUTSIZE 256
#define DEFAULT_UNKNOWN "?"

struct conv_buffer* create_conv_buffer(int size)
{
  struct conv_buffer* buf = NULL;

  buf = (struct conv_buffer*) malloc(sizeof(struct conv_buffer));
  if (buf) {
    buf->size   = size;
    buf->buffer = (char*)malloc(size);
    if (!buf->buffer)
      buf->size = 0;
  }

  return buf;
}

void free_conv_buffer(struct conv_buffer* buf)
{
  if (buf) {
    free(buf->buffer);
    free(buf);
  }
}

char* grow_conv_buffer(struct conv_buffer* buf, char* curr_pos)
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

convert_t initialize_utf8_conversion(const char* charset)
{
  struct convert *conv = NULL;
  int cleanup = 0;

  conv = (struct convert *)malloc(sizeof(struct convert));
  if (conv) {
    /* Unless reset to 0 at the end, this will force cleanup */
    cleanup = 1;
    /* First initialize to default values */
    conv->from_utf8  = (iconv_t)-1;
    conv->to_utf8    = (iconv_t)-1;
    conv->outbuf     = NULL;
    conv->unknown    = NULL;

    /* Now initialize everything to what it should be */
    conv->from_utf8 = iconv_open(charset, "UTF-8");
    if (conv->from_utf8 != (iconv_t)-1) {
      conv->to_utf8 = iconv_open("UTF-8", charset);
      if (conv->to_utf8 != (iconv_t)-1) {
	conv->outbuf = create_conv_buffer(INITIAL_OUTSIZE);
	if (conv->outbuf) {
	  conv->unknown = strdup(DEFAULT_UNKNOWN);
	  if (conv->unknown)
	    cleanup = 0;    /* All successful */
	}
      }
    }
  }

  if (cleanup) {
    cleanup_utf8_conversion(conv);
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

void cleanup_utf8_conversion(convert_t conv)
{
  if (conv) {
    if (conv->from_utf8 != (iconv_t)-1)
      iconv_close(conv->from_utf8);
    if (conv->to_utf8 != (iconv_t)-1)
      iconv_close(conv->to_utf8);
    if (conv->outbuf)
      free_conv_buffer(conv->outbuf);
    if (conv->unknown)
      free(conv->unknown);
    free(conv);
  }
}

char* convert_from_utf8(convert_t conv, const char* input, int* conv_fails)
{
  size_t insize = strlen(input);
  size_t outsize;
  ICONV_CONST char* inptr  = (ICONV_CONST char*) input;
  char   *outptr;
  size_t nconv;
  struct conv_buffer* outbuf;

  if (!conv) {
    if (conv_fails != NULL) *conv_fails = insize;
    return NULL;
  }
  /* make sure we start from an empty state */
  iconv(conv->from_utf8, NULL, NULL, NULL, NULL);
  if (conv_fails != NULL) *conv_fails = 0;
  /* set up output buffer (empty it) */
  outbuf  = conv->outbuf;
  outptr  = outbuf->buffer;
  outsize = outbuf->size;
  memset(outbuf->buffer, 0, outbuf->size);
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
  return outbuf->buffer;
}

char* convert_to_utf8(convert_t conv, const char* input)
{
  size_t insize  = strlen(input);
  size_t outsize;
  ICONV_CONST char *inptr  = (ICONV_CONST char*) input;
  char   *outptr;
  size_t nconv;
  struct conv_buffer* outbuf;

  if (!conv)
    return NULL;
  /* make sure we start from an empty state */
  iconv(conv->to_utf8, NULL, NULL, NULL, NULL);
  /* set up output buffer (empty it) */
  outbuf  = conv->outbuf;
  outptr  = outbuf->buffer;
  outsize = outbuf->size;
  memset(outbuf->buffer, 0, outbuf->size);
  nconv = iconv(conv->to_utf8, &inptr, &insize, &outptr, &outsize);
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
    nconv = iconv(conv->to_utf8, &inptr, &insize, &outptr, &outsize);
  }
  return outbuf->buffer;  
}
