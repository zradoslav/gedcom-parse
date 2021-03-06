/* Date manipulation routines.
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

#include "gedcom_internal.h"
#include "sdncal.h"
#include "buffer.h"
#include "compat.h"
#include <string.h>
#include "date.h"

struct date_value dv_s;
struct date date_s;

struct date_value def_date_val;
struct date def_date;

const char* curr_line_value;

void cleanup_date_buffer();
struct safe_buffer date_buffer = { NULL, 0, NULL, 0, cleanup_date_buffer };

void cleanup_date_buffer()
{
  cleanup_buffer(&date_buffer);
}

int max_month[] = { 12,  /* CAL_GREGORIAN */
		    12,  /* CAL_JULIAN */
		    13,  /* CAL_HEBREW */
		    13,  /* CAL_FRENCH_REV */
		    0    /* CAL_UNKNOWN */
                  };

char* month_name[][13] =
{ /* CAL_GREGORIAN */
  { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" },
  /* CAL_JULIAN */
  { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" },
  /* CAL_JEWISH */
  { "TSH", "CSH", "KSL", "TVT", "SHV", "ADR", "ADS",
    "NSN", "IYR", "SVN", "TMZ", "AAV", "ELL" },
  /* CAL_FRENCH_REF */
  { "VEND", "BRUM", "FRIM", "NIVO", "PLUV", "VENT", "GERM",
    "FLOR", "PRAI", "MESS", "THER", "FRUC", "COMP" }
};

typedef long int (*to_sdn_func_type) (int, int, int);
typedef void (*from_sdn_func_type) (long int, int*, int*, int*);

to_sdn_func_type to_sdn_func[] = {
  &GregorianToSdn,   /* CAL_GREGORIAN */
  &JulianToSdn,      /* CAL_JULIAN */
  &JewishToSdn,      /* CAL_JEWISH */
  &FrenchToSdn       /* CAL_FRENCH_REV */
};

from_sdn_func_type from_sdn_func[] = {
  &SdnToGregorian,   /* CAL_GREGORIAN */
  &SdnToJulian,      /* CAL_JULIAN */
  &SdnToJewish,      /* CAL_JEWISH */
  &SdnToFrench       /* CAL_FRENCH_REV */
};

long int checkedCalToSdn(Calendar_type cal, int year, int month, int day)
{
  int y, m, d;
  long int sdn = (*to_sdn_func[cal])(year,month, day);
  if (sdn <= 0)
    return -1;
  else {
    (*from_sdn_func[cal])(sdn, &y, &m, &d);
    if ((year == y) && (month == m) && (day == d))
      return sdn;
    else
      return -1;
  }
}

int checkedSdnToCal(Calendar_type cal, long int sdn,
		    int* year, int* month, int* day)
{
  (*from_sdn_func[cal])(sdn, year, month, day);
  if (*year > 0 && *month > 0 && *day > 0)
    return 1;
  else
    return 0;
}

void copy_date(struct date *to, struct date *from)
{
  memcpy(to, from, sizeof(struct date));
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

struct date_value* make_date_value(Date_value_type t, struct date *d1,
				  struct date *d2, const char* p)
{
  dv_s.type = t;
  copy_date(&dv_s.date1, d1);
  copy_date(&dv_s.date2, d2);
  strncpy(dv_s.phrase, p, MAX_PHRASE_LEN + 1);
  return &dv_s;
}

/* PRE:     d->cal != CAL_UNKNOWN
   INPUT:   d->day, d->month, d->year
   OUTPUT:  d->type, d->sdn1, d->sdn2
*/
int numbers_to_sdn(struct date *d)
{
  int result = 0;
  if (d->cal == CAL_UNKNOWN) {
    d->type = DATE_UNRECOGNIZED;
    gedcom_date_error(_("Cannot compute SDN for unknown calendar type"));
    result = 1;
  }
  else {
    struct date begin_date;
    struct date end_date;
    copy_date(&begin_date, d);
    if (d->day == -1 || d->month == -1 || d->year == -1) {
      d->type = DATE_BOUNDED;
      copy_date(&end_date, d);
      if (begin_date.month == -1) {
	begin_date.month = 1; end_date.month = 1;
	begin_date.day   = 1; end_date.day   = 1;
	end_date.year += 1;
      }
      else if (begin_date.day == -1) {
	begin_date.day   = 1; end_date.day   = 1;
	end_date.month += 1;
	if (end_date.month > max_month[d->cal]) {
	  end_date.month -= max_month[d->cal];
	  end_date.year  += 1;
	}
      }
      else {
	gedcom_date_error(_("Year has to be given in bounded date"));
	result = 1;
      }
    }
    else {
      d->type = DATE_EXACT;
    }

    d->sdn1 = checkedCalToSdn(d->cal, begin_date.year, begin_date.month,
			      begin_date.day);
    if (d->sdn1 == -1) {
      gedcom_date_error(_("Error converting date: year %d, month %d, day %d"),
		   begin_date.year, begin_date.month, begin_date.day);
      result = 1;
    }
    else
      if (d->type == DATE_BOUNDED) {
	d->sdn2 = checkedCalToSdn(d->cal, end_date.year, end_date.month,
				  end_date.day);
	if (d->sdn2 == -1) {
	  gedcom_date_error(_("Error converting date: year %d, month %d, day %d"),
		       end_date.year, end_date.month, end_date.day);
	  result = 1;
	}
	else
	  d->sdn2 -= 1;
      }
  }
  return result;
}

/* PRE:     d->cal != CAL_UNKNOWN
   INPUT:   d->type, d->sdn1, d->sdn2
   OUTPUT:  d->day, d->month, d->year
*/
int sdn_to_numbers(struct date *d)
{
  int result = 0;
  if (d->cal == CAL_UNKNOWN) {
    gedcom_date_error(_("Cannot compute from SDN for unknown calendar type"));
    result = 1;
  }
  else {
    struct date begin_date;
    struct date end_date;

    if (d->sdn1 <= 0) {
      gedcom_date_error(_("SDN 1 should be bigger than zero"));
      result = 1;
    }
    else {
      copy_date(&begin_date, d);
      if (!checkedSdnToCal(d->cal, d->sdn1, &begin_date.year,
			   &begin_date.month, &begin_date.day)) {
	gedcom_date_error(_("SDN 1 isn't a valid date in the given calendar"));
	result = 1;
      }
      else {
	switch (d->type) {
	  case DATE_EXACT:
	    if (d->sdn2 != -1) {
	      gedcom_date_error(_("SDN 2 should be -1 for exact dates"));
	      result = 1;
	    }
	    break;
	  case DATE_BOUNDED:
	    if (d->sdn2 <= 0) {
	      gedcom_date_error(_("SDN 2 should be bigger than zero"));
	      result = 1;
	    }
	    else if (d->sdn2 <= d->sdn1) {
	      gedcom_date_error(_("SDN 2 should be bigger than SDN 1"));
	      result = 1;
	    }
	    else {
	      copy_date(&end_date, d);
	      if (!checkedSdnToCal(d->cal, d->sdn2, &end_date.year,
				   &end_date.month, &end_date.day)) {
		gedcom_date_error(_("SDN 2 isn't a valid date in the given calendar"));
		result = 1;
	      }
	      else {
		if (begin_date.year == end_date.year) {
		  if (begin_date.month == end_date.month) {
		    if (begin_date.day == end_date.day) {
		      /* year, month and day are relevant */
		    }
		    else {
		      /* year and month are relevant */
		      begin_date.day = -1;
		    }
		  }
		  else {
		    /* only year is relevant */
		    begin_date.month = -1;
		    begin_date.day   = -1;
		  }
		}
		else {
		  gedcom_date_error(_("SDN1/SDN2 isn't a bounded date"));
		  result = 1;
		}
	      }
	    }
	    break;
	  default:
	    break;
	}
      }
      d->year  = begin_date.year;
      d->month = begin_date.month;
      d->day   = begin_date.day;
    }
  }
  return result;
}

/* PRE:     d->cal != CAL_UNKNOWN
   INPUT:   d->day_str, d->month_str, d->year_str
   OUTPUT:  d->day, d->month, d->year, d->year_type
*/
int strings_to_numbers(struct date *d)
{
  int result = 0;
  if (d->cal == CAL_UNKNOWN) {
    gedcom_date_error(_("Cannot compute months for unknown calendar type"));
    result = 1;
  }
  else {
    if (d->day_str[0]) {
      d->day = get_day_num(d->day_str);
      if (d->day == -1) result = 1;
    }
    else
      d->day = -1;
    
    if (d->month_str[0]) {
      d->month = get_month_num(d->cal, d->month_str);
      if (d->month == -1) result = 1;
    }
    else
      d->month = -1;
    
    d->year = get_year_num(d->year_str, &d->year_type);
    if (d->year == -1) result = 1;
  }
  
  return result;
}

/* PRE:     d->cal != CAL_UNKNOWN
   INPUT:   d->day, d->month, d->year, d->year_type
   OUTPUT:  d->day_str, d->month_str, d->year_str
*/
int numbers_to_strings(struct date *d)
{
  int result = 0;
  if (d->cal == CAL_UNKNOWN) {
    gedcom_date_error(_("Cannot compute month names for unknown calendar type"));
    result = 1;
  }
  else {
    if (d->day != -1)
      sprintf(d->day_str, "%d", d->day);
    
    if (d->month > 0 && d->month <= max_month[d->cal])
      strcpy(d->month_str, month_name[d->cal][d->month - 1]);
    
    if (d->year_type == YEAR_SINGLE)
      sprintf(d->year_str, "%d", d->year);
    else
      sprintf(d->year_str, "%d/%d", d->year - 1, (d->year % 100));
  }
  return result;
}

/** This function can be called to ensure that an updated date_value is
    consistent, i.e. all its struct fields are consistent with each other.
    Depending on which fields you have updated, you should give the correct
    \c compute_from field.

    The following table gives an overview of the input and output parameters
    (the calendar type \c cal is always an input parameter, and should not be
    \c CAL_UNKNOWN):
     <table border="1" width="100%">
       <tr>
         <th><b>compute_from</b></th>
	 <th><b>input parameters</b></th>
	 <th><b>output parameters</b></th>
       </tr>
       <tr>
         <td><code>DI_FROM_STRINGS</code></td>
	 <td><code>day_str, month_str, year_str</code></td>
	 <td><code>day, month, year, year_type<br>
	     type, sdn1, sdn2</code></td>
       </tr>
       <tr>
         <td><code>DI_FROM_NUMBERS</code></td>
	 <td><code>day, month, year, year_type</code></td>
	 <td><code>day_str, month_str, year_str<br>
	     type, sdn1, sdn2</code></td>
       </tr>
       <tr>
         <td><code>DI_FROM_SDN</code></td>
	 <td><code>type, sdn1, sdn2</code></td>
	 <td><code>day, month, year<br>
	     day_str, month_str, year_str</code></td>
       </tr>
     </table>

    If the type in the date_value is \c DV_PHRASE, no conversions take place,
    otherwise one or both of the date structs are processed according to the
    table above, depending on the type.

    This function could also be used to convert a date from one calendar to
    another, because the serial day number is calendar independent (error
    handling is ignored in this example):
    
    \code
      struct date_value* dv = gedcom_new_date_value(NULL);
      dv->date1.cal = CAL_GREGORIAN;
      dv->date1.day   = 4;
      dv->date1.month = 2;
      dv->date1.year  = 1799;
      dv->date1.year_type = YEAR_SINGLE;
      gedcom_normalize_date(DI_FROM_NUMBERS, dv);

      dv->date1.cal = CAL_FRENCH_REV;
      gedcom_normalize_date(DI_FROM_SDN, dv);
    \endcode

    At the end of this piece of code, the day, month and year are filled in
    according to the French Revolution calendar.

    \param compute_from Determines which fields will be taken as input to
    compute the other fields.
     
    \param val The struct date_value to update (it will be updated in place)

    \retval 0 on success
    \retval >0 on failure
*/
int gedcom_normalize_date(Date_input compute_from, struct date_value *val)
{
  int result = 0;
  if (val->type != DV_PHRASE) {
    switch (compute_from) {
      case DI_FROM_STRINGS:
	result |= strings_to_numbers(&val->date1);
	result |= numbers_to_sdn(&val->date1);
	if (val->type == DV_BETWEEN || val->type == DV_FROM_TO) {
	  result |= strings_to_numbers(&val->date2);
	  result |= numbers_to_sdn(&val->date2);
	}
	break;
      case DI_FROM_NUMBERS:
	result |= numbers_to_strings(&val->date1);
	result |= numbers_to_sdn(&val->date1);
	if (val->type == DV_BETWEEN || val->type == DV_FROM_TO) {
	  result |= numbers_to_strings(&val->date2);
	  result |= numbers_to_sdn(&val->date2);
	}
	break;
      case DI_FROM_SDN:
	result |= sdn_to_numbers(&val->date1);
	result |= numbers_to_strings(&val->date1);
	if (val->type == DV_BETWEEN || val->type == DV_FROM_TO) {
	  result |= sdn_to_numbers(&val->date2);
	  result |= numbers_to_strings(&val->date2);
	}
	break;
      default:
	break;
    }
  }
  return result;
}

/** This function creates a new date_value struct and initializes it properly,
    or copies an existing date value.

    \param copy_from  A given struct date_value to copy (or \c NULL).

    \return If the parameter \c copy_from is NULL, a new value is created and
    given initial values.  If it is non-NULL, the given value is copied into
    a new date value.  In both cases, the new value is returned.
*/
struct date_value* gedcom_new_date_value(const struct date_value* copy_from)
{
  struct date_value* dv_ptr;
  dv_ptr = (struct date_value*) malloc(sizeof(struct date_value));
  if (!dv_ptr)
    MEMORY_ERROR;
  else {
    if (copy_from)
      memcpy(dv_ptr, copy_from, sizeof(struct date_value));
    else {
      dv_ptr->type = DV_NO_MODIFIER;
      init_date(&dv_ptr->date1);
      init_date(&dv_ptr->date2);
      dv_ptr->phrase[0] = '\0';
    }
  }
  return dv_ptr;
}

/** This function allows to convert the given \c line_value into a struct
    date_value.
    
    \param line_value A string containing the date to parse

    \return The parsed date; note that this return value is statically
    allocated, and is thus overwritten on each call.
*/
struct date_value gedcom_parse_date(const char* line_value)
{
  int result = 0;
  init_date(&dv_s.date1);
  init_date(&dv_s.date2);
  init_date(&date_s);
  init_date(&def_date);
  curr_line_value = line_value;
  if (compat_mode(C_NO_REQUIRED_VALUES)
      && !strncmp(curr_line_value, VALUE_IF_MISSING, 2)) {
    gedcom_date_error(_("Empty value changed to '%s'"), VALUE_IF_MISSING);
    result = 1;
  }
  else {
    compat_date_start();
    init_gedcom_date_lex(line_value);
    gedcom_date_parse();
    close_gedcom_date_lex();
    if (compat_date_check(&dv_s, &curr_line_value)) {
      init_gedcom_date_lex(curr_line_value);
      gedcom_date_parse();
      close_gedcom_date_lex();
    }
    if (dv_s.date1.cal != CAL_UNKNOWN)
      result |= numbers_to_sdn(&dv_s.date1);
    if (dv_s.date2.cal != CAL_UNKNOWN)
      result |= numbers_to_sdn(&dv_s.date2);
  }
  if (result != 0) {
    gedcom_date_error(_("Putting date '%s' in 'phrase' member"),
		      curr_line_value);
    make_date_value(DV_PHRASE, &dv_s.date1, &dv_s.date2, curr_line_value);
  }
  return dv_s;
}

void write_date(const struct date* d)
{
  if (! d->year_str[0] || d->year <= 0 || d->sdn1 <= 0)
    gedcom_error(_("Date is not normalized: some fields are invalid"));
  else {
    switch (d->cal) {
      case CAL_GREGORIAN: break;
      case CAL_JULIAN:
	safe_buf_append(&date_buffer, "@#DJULIAN@ "); break;
      case CAL_HEBREW:
	safe_buf_append(&date_buffer, "@#DHEBREW@ "); break;
      case CAL_FRENCH_REV:
	safe_buf_append(&date_buffer, "@#DFRENCH R@ "); break;
      case CAL_UNKNOWN:
	safe_buf_append(&date_buffer, "@#DUNKNOWN@ "); break;
      default:
	break;
    }
    if (d->day_str[0])
      safe_buf_append(&date_buffer, "%s ", d->day_str);
    if (d->month_str[0])
      safe_buf_append(&date_buffer, "%s ", d->month_str);
    safe_buf_append(&date_buffer, "%s", d->year_str);
  }
}

/** This function converts the given struct date_value into its string
    representation.

    \param val  The given parsed date

    \return The string representation of the parsed date; note that this value
    is statically allocated, and is thus overwritten on each call
*/
char* gedcom_date_to_string(const struct date_value* val)
{
  init_buffer(&date_buffer);
  reset_buffer(&date_buffer);
  
  switch (val->type) {
    case DV_NO_MODIFIER:
      write_date(&val->date1); break;
    case DV_BEFORE:
      safe_buf_append(&date_buffer, "BEF ");
      write_date(&val->date1); break;
    case DV_AFTER:
      safe_buf_append(&date_buffer, "AFT ");
      write_date(&val->date1); break;
    case DV_BETWEEN:
      safe_buf_append(&date_buffer, "BET ");
      write_date(&val->date1);
      safe_buf_append(&date_buffer, " AND ");
      write_date(&val->date2); break;
    case DV_FROM:
      safe_buf_append(&date_buffer, "FROM ");
      write_date(&val->date1); break;
    case DV_TO:
      safe_buf_append(&date_buffer, "TO ");
      write_date(&val->date1); break;
    case DV_FROM_TO:
      safe_buf_append(&date_buffer, "FROM ");
      write_date(&val->date1);
      safe_buf_append(&date_buffer, " TO ");
      write_date(&val->date2); break;
    case DV_ABOUT:
      safe_buf_append(&date_buffer, "ABT ");
      write_date(&val->date1); break;
    case DV_CALCULATED:
      safe_buf_append(&date_buffer, "CAL ");
      write_date(&val->date1); break;
    case DV_ESTIMATED:
      safe_buf_append(&date_buffer, "EST ");
      write_date(&val->date1); break;
    case DV_INTERPRETED:
      safe_buf_append(&date_buffer, "INT ");
      write_date(&val->date1);
      safe_buf_append(&date_buffer, " (%s)", val->phrase); break;
    case DV_PHRASE:
      safe_buf_append(&date_buffer, "(%s)", val->phrase); break;
    default:
      break;
  }
  
  return get_buf_string(&date_buffer);
}
