/* Age manipulation routines.
   Copyright (C) 2001,2002 The Genes Development Team
   This file is part of the Gedcom parser library.
   Contributed by Peter Verthez <Peter.Verthez@advalvas.be>, 2001.

   The Gedcom parser library is free software; you can redistribute it
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
   02111-1307 USA.  */

/* $Id$ */
/* $Name$ */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "gedcom_internal.h"
#include "buffer.h"
#include "age.h"

struct age_value age_s;
struct age_value def_age_val;

void cleanup_age_buffer();
struct safe_buffer age_buffer = { NULL, 0, NULL, 0, cleanup_age_buffer };

void cleanup_age_buffer()
{
  cleanup_buffer(&age_buffer);
}

void copy_age(struct age_value *to, struct age_value from)
{
  memcpy(to, &from, sizeof(struct age_value));
}

void init_age(struct age_value *age)
{
  age->type = AGE_UNRECOGNIZED;
  age->mod  = AGE_NO_MODIFIER;
  age->years = -1;
  age->months = -1;
  age->days = -1;
}

int parse_numeric_age(struct age_value *age, const char *ptr)
{
  char *endptr;
  while (ptr) {
    long int number = strtol(ptr, &endptr, 10);
    if (errno == ERANGE || number < 0 || number > INT_MAX) {
      gedcom_error(_("Number out of range in age"));
      return 1;
    }
    else {
      ptr = endptr;
      if (*ptr == '\0')
	break;
      else if (*ptr == 'Y' || *ptr == 'y') {
	if (age->years == -1)
	  age->years = number;
	else {
	  gedcom_error(_("Duplicate year indication in age"));
	  return 1;
	}
      }
      else if (*ptr == 'M' || *ptr == 'm') {
	if (age->months == -1)
	  age->months = number;
	else {
	  gedcom_error(_("Duplicate month indication in age"));
	  return 1;
	}
      }
      else if (*ptr == 'D' || *ptr == 'd') {
	if (age->days == -1)
	  age->days = number;
	else {
	  gedcom_error(_("Duplicate day indication in age"));
	  return 1;
	}
      }
      else {
	gedcom_error(_("Unrecognized indication in age: '%s'"), ptr);
	return 1;
      }
      ptr++;
      while (*ptr == ' ') ptr++;
    }
  }
  return 0;
}

struct age_value* gedcom_new_age_value(struct age_value* copy_from)
{
  struct age_value* age_ptr;
  age_ptr = (struct age_value*) malloc(sizeof(struct age_value));
  if (! age_ptr)
    MEMORY_ERROR;
  else {
    if (copy_from)
      memcpy(age_ptr, copy_from, sizeof(struct age_value));
    else 
      init_age(age_ptr);
  }
  return age_ptr;
}

struct age_value gedcom_parse_age(const char* line_value)
{
  const char *ptr = line_value;
  init_age(&age_s);
  init_age(&def_age_val);

  if (*ptr == '<') {
    age_s.mod = AGE_LESS_THAN;
    ptr++;
    while (*ptr == ' ') ptr++;
  }
  else if (*ptr == '>') {
    age_s.mod = AGE_GREATER_THAN;
    ptr++;
    while (*ptr == ' ') ptr++;
  }

  if (isdigit((unsigned char)*ptr)) {
    int result = parse_numeric_age(&age_s, ptr);
    if (result == 0) {
      age_s.type = AGE_NUMERIC;
    }
  }
  else if (!strcasecmp(line_value, "CHILD"))
    age_s.type = AGE_CHILD;
  else if (!strcasecmp(line_value, "INFANT"))
    age_s.type = AGE_INFANT;
  else if (!strcasecmp(line_value, "STILLBORN"))
    age_s.type = AGE_STILLBORN;
  else
    gedcom_error(_("Unrecognized age format"));
  if (age_s.type == AGE_UNRECOGNIZED)
    strncpy(age_s.phrase, line_value, MAX_PHRASE_LEN + 1);
  return age_s;
}

char* gedcom_age_to_string(struct age_value* val)
{
  int num = 0;
  reset_buffer(&age_buffer);

  switch (val->mod) {
    case AGE_LESS_THAN:
      safe_buf_append(&age_buffer, "<"); break;
    case AGE_GREATER_THAN:
      safe_buf_append(&age_buffer, ">"); break;
    default:
      break;
  }

  switch (val->type) {
    case AGE_UNRECOGNIZED:
      return val->phrase; break;
    case AGE_CHILD:
      safe_buf_append(&age_buffer, "CHILD"); break;
    case AGE_INFANT:
      safe_buf_append(&age_buffer, "INFANT"); break;
    case AGE_STILLBORN:
      safe_buf_append(&age_buffer, "STILLBORN"); break;
    case AGE_NUMERIC:
      if (val->years != -1) {
	num = 1;
	safe_buf_append(&age_buffer, "%dy", val->years);
      }
      if (val->months != -1) {
	if (num)
	  safe_buf_append(&age_buffer, " ");
	num = 1;
	safe_buf_append(&age_buffer, "%dm", val->months);
      }
      if (val->days != -1) {
	if (num)
	  safe_buf_append(&age_buffer, " ");
	num = 1;
	safe_buf_append(&age_buffer, "%dd", val->days);
      }
      break;
    default:
      break;
  }
  
  return get_buf_string(&age_buffer);
}
