/* Encoding utility from UTF-8 to locale and vice versa
   Copyright (C) 2001, 2002 Peter Verthez

   Permission granted to do anything with this file that you want, as long
   as the above copyright is retained in all copies.
   THERE IS NO WARRANTY - USE AT YOUR OWN RISK
*/

/* $Id$ */
/* $Name$ */

#include <stdlib.h>
#include <iconv.h>
#include <langinfo.h>
#include <assert.h>
#include <errno.h>
#include "utf8-locale.h"

#define INITIAL_OUTSIZE 256

static iconv_t utf8_to_locale = (iconv_t) -1;
static iconv_t locale_to_utf8 = (iconv_t) -1;
static char*   outbuffer = NULL;
static size_t  outbufsize = 0;
static const char* the_unknown = "?";

void convert_set_unknown(const char* unknown)
{
  the_unknown = unknown;
}

void close_conversion_contexts()
{
  iconv_close(utf8_to_locale);
  iconv_close(locale_to_utf8);
  utf8_to_locale = (iconv_t) -1;
  locale_to_utf8 = (iconv_t) -1;
  free(outbuffer);
}

int open_conversion_contexts()
{
  assert(utf8_to_locale == (iconv_t) -1);
  assert(locale_to_utf8 == (iconv_t) -1);
  utf8_to_locale = iconv_open(nl_langinfo(CODESET), "UTF-8");
  if (utf8_to_locale == (iconv_t) -1)
    return -1;
  else {
    locale_to_utf8 = iconv_open("UTF-8", nl_langinfo(CODESET));
    if (locale_to_utf8 == (iconv_t) -1) {
      close_conversion_contexts();
      return -1;
    }
    else {
      outbufsize = INITIAL_OUTSIZE;
      outbuffer = (char*)malloc(outbufsize);
      atexit(close_conversion_contexts);
      return 0;
    }
  }
}

char* convert_utf8_to_locale(char* input)
{
  size_t insize  = strlen(input);
  size_t outsize;
  char   *inptr  = input;
  char   *outptr;
  size_t nconv;

  if (utf8_to_locale == (iconv_t) -1 && (open_conversion_contexts() == -1))
    return NULL;
  assert(utf8_to_locale != (iconv_t) -1);
  /* make sure we start from an empty state */
  iconv(utf8_to_locale, NULL, NULL, NULL, NULL);
  /* set up output buffer (empty it) */
  outptr  = outbuffer;
  outsize = outbufsize;
  memset(outbuffer, 0, outbufsize);
  nconv = iconv(utf8_to_locale, &inptr, &insize, &outptr, &outsize);
  while (nconv == -1) {
    if (errno == E2BIG) {
      /* grow the output buffer */
      size_t outlen;
      outlen     = outptr - outbuffer;
      outbufsize *= 2;
      outbuffer  = realloc(outbuffer, outbufsize);
      outptr     = outbuffer + outlen;
      outsize    = outbufsize - outlen;
      memset(outptr, 0, outsize);
    }
    else if (errno == EILSEQ) {
      /* skip over character */
      const char* unkn_ptr = the_unknown;
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
      /* EBADF is an error which should be captured by the assert above */
      return NULL;
    }
    nconv = iconv(utf8_to_locale, &inptr, &insize, &outptr, &outsize);
  }
  return outbuffer;
}

char* convert_locale_to_utf8(char* input)
{
  size_t insize  = strlen(input);
  size_t outsize;
  char   *inptr  = input;
  char   *outptr;
  size_t nconv;

  if (locale_to_utf8 == (iconv_t) -1 && (open_conversion_contexts() == -1))
    return NULL;
  assert(locale_to_utf8 != (iconv_t) -1);
  /* make sure we start from an empty state */
  iconv(locale_to_utf8, NULL, NULL, NULL, NULL);
  /* set up output buffer (empty it) */
  outptr  = outbuffer;
  outsize = outbufsize;
  memset(outbuffer, 0, outbufsize);
  nconv = iconv(locale_to_utf8, &inptr, &insize, &outptr, &outsize);
  while (nconv == -1) {
    if (errno == E2BIG) {
      /* grow the output buffer */
      size_t outlen;
      outlen     = outptr - outbuffer;
      outbufsize *= 2;
      outbuffer  = realloc(outbuffer, outbufsize);
      outptr     = outbuffer + outlen;
      outsize    = outbufsize - outlen;
      memset(outptr, 0, outsize);
    }
    else {
      /* EILSEQ should not happen, because UTF-8 can represent anything */
      /* EINVAL should not happen, since we convert entire strings */
      /* EBADF is an error which should be captured by the assert above */
      return NULL;
    }
    nconv = iconv(locale_to_utf8, &inptr, &insize, &outptr, &outsize);
  }
  return outbuffer;
}
