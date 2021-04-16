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
#include <string.h>

#include <uniconv.h>

void convert_set_unknown(const char* unknown)
{
	conversion_set_unknown(NULL, unknown);
}

const char* convert_utf8_to_locale(const char* input, int *conv_fails)
{
	size_t result_len;
	const char* result_str;

	result_str = u8_conv_to_encoding(locale_charset(), iconveh_question_mark, input, strlen(input),
	                                 NULL, NULL, &result_len);

	printf("intentionally memleaked: %zu b\n", result_len);
	return result_str;
}

const char* convert_locale_to_utf8(const char* input)
{
	size_t result_len;
	const char* result_str;

	result_str = u8_conv_from_encoding(locale_charset(), iconveh_question_mark, input, strlen(input),
	                                 NULL, NULL, &result_len);

	printf("intentionally memleaked: %zu b\n", result_len);
	return result_str;
}
