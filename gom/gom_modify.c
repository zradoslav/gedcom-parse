/* Source code for modifying the gedcom object model.
   Copyright (C) 2002 The Genes Development Team
   This file is part of the Gedcom parser library.
   Contributed by Peter Verthez <Peter.Verthez@advalvas.be>, 2002.

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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utf8tools.h"
#include "gom.h"
#include "gom_internal.h"

void gom_set_unknown(const char* unknown)
{
  convert_set_unknown(unknown);
}

char* gom_get_string(char* data)
{
  return data;
}

char* gom_get_string_for_locale(char* data, int* conversion_failures)
{
  return convert_utf8_to_locale(gom_get_string(data), conversion_failures);
}

char* gom_set_string(char** data, const char* utf8_str)
{
  char* result = NULL;
  char* newptr;

  if (!is_utf8_string(utf8_str)) {
    gedcom_error(_("The input '%s' is not a valid UTF-8 string"), utf8_str);
  }
  else {
    newptr = strdup(utf8_str);
    if (!newptr)
      MEMORY_ERROR;
    else {
      if (*data) free(*data);
      *data = newptr;
      result = *data;
    }
  }
  
  return result;
}

char* gom_set_string_for_locale(char** data, const char* locale_str)
{
  char* result = NULL;
  char* utf8_str = convert_locale_to_utf8(locale_str);
  
  if (!utf8_str)
    gedcom_error(_("The input '%s' is not a valid string for the locale"),
		 locale_str);
  else
    result = gom_set_string(data, utf8_str);

  return result;
}

int update_date(struct date_value** dv, struct tm* tm_ptr)
{
  int result;
  struct date_value* dval = gedcom_new_date_value(NULL);
  dval->type        = DV_NO_MODIFIER;
  dval->date1.cal   = CAL_GREGORIAN;
  dval->date1.day   = tm_ptr->tm_mday;
  dval->date1.month = tm_ptr->tm_mon + 1;
  dval->date1.year  = tm_ptr->tm_year + 1900;
  result = gedcom_normalize_date(DI_FROM_NUMBERS, dval);

  if (result == 0) {
    if (*dv) free(*dv);
    *dv = dval;
  }
  return result;
}

int update_time(char** tv, struct tm* tm_ptr)
{
  char tval[16];
  sprintf(tval, "%02d:%02d:%02d",
	  tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);

  if (gom_set_string(tv, tval))
    return 0;
  else
    return 1;
}

