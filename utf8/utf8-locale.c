/* Encoding utility from UTF-8 to locale and vice versa
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
#include <assert.h>
#include <string.h>
#include "libcharset.h"

static convert_t locale_conv = NULL;

void close_conversion_contexts()
{
  cleanup_utf8_conversion(locale_conv);
}

int open_conversion_contexts()
{
  assert (locale_conv == NULL);
  locale_conv = initialize_utf8_conversion(locale_charset(), 0);

  if (locale_conv) {
    atexit(close_conversion_contexts);
    return 0;
  }
  else {
    return -1;
  }
}

void convert_set_unknown(const char* unknown)
{
  if (!locale_conv)
    open_conversion_contexts();
  conversion_set_unknown(locale_conv, unknown);
}

char* convert_utf8_to_locale(const char* input, int *conv_fails)
{
  if (!locale_conv)
    open_conversion_contexts();

  return convert_from_utf8(locale_conv, input, conv_fails, NULL);
}

char* convert_locale_to_utf8(const char* input)
{
  if (!locale_conv)
    open_conversion_contexts();

  return convert_to_utf8(locale_conv, input, strlen(input));
}
