/* Utility functions for UTF-8
   Copyright (C) 2001, 2002 Peter Verthez

   Permission granted to do anything with this file that you want, as long
   as the above copyright is retained in all copies.
   THERE IS NO WARRANTY - USE AT YOUR OWN RISK
*/

/* $Id$ */
/* $Name$ */

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

int utf8_strlen(const char* str)
{
  int num_char = 0;
  
  if (!str) return 0;
  
  while (*str) {
    if ((*str & 0x80) == 0 || (*str & 0xC0) == 0xC0)
      num_char++;
    str++;
  }
  
  return num_char;
}

