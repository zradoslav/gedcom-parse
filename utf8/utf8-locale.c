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
#include <assert.h>
#include "libcharset.h"

static convert_t locale_conv = NULL;

void convert_set_unknown(const char* unknown)
{
  conversion_set_unknown(locale_conv, unknown);
}

void close_conversion_contexts()
{
  cleanup_utf8_conversion(locale_conv);
}

int open_conversion_contexts()
{
  assert (locale_conv == NULL);
  locale_conv = initialize_utf8_conversion(locale_charset());

  if (locale_conv) {
    atexit(close_conversion_contexts);
    return 0;
  }
  else {
    return -1;
  }
}

char* convert_utf8_to_locale(const char* input, int *conv_fails)
{
  if (!locale_conv)
    open_conversion_contexts();

  return convert_from_utf8(locale_conv, input, conv_fails);
}

char* convert_locale_to_utf8(const char* input)
{
  if (!locale_conv)
    open_conversion_contexts();

  return convert_to_utf8(locale_conv, input);
}
