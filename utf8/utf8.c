/* Utility functions for UTF-8
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

#include <unistr.h>

#include "utf8tools.h"

int is_utf8_string(const char* str)
{
  int expect_bytes = 0;

  if (!str) return 0;
  
  while (*str) {
    if ((*str & 0x80) == 0) {
      /* Looks like an ASCII character */
      if (expect_bytes)
	/* byte of UTF-8 character expected */
	return 0;
      else {
	/* OK, ASCII character expected */
	str++;
      }
    }
    else {
      /* Looks like byte of an UTF-8 character */
      if (expect_bytes) {
	/* expect_bytes already set: first byte of UTF-8 char already seen */
	if ((*str & 0xC0) == 0x80) {
	  /* OK, next byte of UTF-8 character */
	  /* Decrement number of expected bytes */
	  expect_bytes--;
	  str++;
	}
	else {
	  /* again first byte ?!?! */
	  return 0;
	}
      }
      else {
	/* First byte of the UTF-8 character */
	/* count initial one bits and set expect_bytes to 1 less */
	char ch = *str;
	while (ch & 0x80) {
	  expect_bytes++;
	  ch = (ch & 0x7f) << 1;
	}
	expect_bytes--;
	str++;
      }
    }
  }

  return (expect_bytes == 0);
}

size_t utf8_strlen(const char* str)
{
	size_t n = 0;
	ucs4_t c;
	for(const uint8_t* it = str; it; it = u8_next(&c, it))
		n++;
	return --n;
}

const char* next_utf8_char(const char* str)
{
	if(!str)
		return NULL;

	ucs4_t c;
	return u8_next(&c, str);
}

const char* nth_utf8_char(const char* str, size_t n)
{
	const char* s = str;
	while(n-- && (s = next_utf8_char(s)));

	return s;
}
