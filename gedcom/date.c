/* Date manipulation routines.
   Copyright (C) 2001 The Genes Development Team
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

#include <stdio.h>
#include "gedcom_internal.h"
#include "sdncal.h"
#include "date.h"

struct date_value dv_s;
struct date date_s;

struct date_value def_date_val;
struct date def_date;

int max_month[] = { 12,  /* CAL_GREGORIAN */
		    12,  /* CAL_JULIAN */
		    13,  /* CAL_HEBREW */
		    13,  /* CAL_FRENCH_REV */
		    0    /* CAL_UNKNOWN */
                  };

typedef long int (*cal_func_type) (int, int, int);

cal_func_type cal_func[] = { &GregorianToSdn,   /* CAL_GREGORIAN */
			     &JulianToSdn,      /* CAL_JULIAN */
			     &JewishToSdn,      /* CAL_JEWISH */
			     &FrenchToSdn       /* CAL_FRENCH_REV */
                           };

void copy_date(struct date *to, struct date from)
{
  memcpy(to, &from, sizeof(struct date));
}

void init_date(struct date *d)
{
  d->cal = CAL_UNKNOWN;
  d->day_str[0] = '\0';
  d->month_str[0] = '\0';
  d->year_str[0] = '\0';
  d->day = -1;
  d->month = -1;
  d->year = -1;
  d->year_type = YEAR_SINGLE;
  d->type = DATE_UNRECOGNIZED;
  d->sdn1 = -1;
  d->sdn2 = -1;
}

struct date_value make_date_value(Date_value_type t, struct date d1,
				  struct date d2, char* p)
{
  dv_s.type = t;
  copy_date(&dv_s.date1, d1);
  copy_date(&dv_s.date2, d2);
  strncpy(dv_s.phrase, p, MAX_PHRASE_LEN + 1);
  return dv_s;
}

void make_date_complete(struct date *d)
{
  if (d->cal == CAL_UNKNOWN)
    d->type = DATE_UNRECOGNIZED;
  else {
    struct date end_date;
    cal_func_type to_sdn;
    if (d->day == -1 || d->month == -1 || d->year == -1) {
      d->type = DATE_BOUNDED;
      copy_date(&end_date, *d);
      if (d->month == -1) {
	d->month = 1; end_date.month = 1;
	d->day   = 1; end_date.day   = 1;
	end_date.year += 1;
      }
      else if (d->day == -1) {
	d->day   = 1; end_date.day   = 1;
	end_date.month += 1;
	if (end_date.month > max_month[d->cal]) {
	  end_date.month -= max_month[d->cal];
	  end_date.year  += 1;
	}
      }
    }
    else {
      d->type = DATE_EXACT;
    }

    to_sdn = cal_func[d->cal];
    d->sdn1 = (*to_sdn)(d->year, d->month, d->day);
    if (d->type == DATE_BOUNDED) {
      d->sdn2 = (*to_sdn)(end_date.year, end_date.month, end_date.day);
      d->sdn2 -= 1;
    }
  }
}

struct date_value gedcom_parse_date(char* line_value)
{
  init_date(&date_s);
  init_date(&def_date);
  init_gedcom_date_lex(line_value);
  gedcom_date_parse();
  close_gedcom_date_lex();
  make_date_complete(&dv_s.date1);
  make_date_complete(&dv_s.date2);
  return dv_s;
}

