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

#include "utf8tools.h"
#include <string.h>

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

int utf8_strlen(const char* str)
{
  int num_char = 0;
  
  if (!str) return 0;
  
  while (*str) {
    if ((*str & 0xC0) != 0xC0) num_char++;
    str++;
  }
  
  return num_char;
}

char* next_utf8_char(char* str)
{
  if (!str) return NULL;

  if (*str) {
    str++;
    while (*str && (*str & 0xC0) == 0x80)
      str++;
  }
  return str;
}

char* nth_utf8_char(char* str, int n)
{
  int num_char = 0;
  if (!str) return NULL;

  if (*str) {
    str++;
    while (*str) {
      if ((*str & 0xC0) != 0x80) num_char++;
      if (num_char == n) break;
      str++;
    }
  }
  return str;
}
