/* Parser for Gedcom dates.
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

%{
#include <stdlib.h>
#include "date.h"

int _get_day_num(const char* input);
int _get_year_num(Year_type ytype, const char* input1, const char* input2);
  
%}

%union {
  char *string;
  struct date_value date_val;
  struct date date;
}

%token <string> ESC_DATE_GREG
%token <string> ESC_DATE_JULN
%token <string> ESC_DATE_HEBR
%token <string> ESC_DATE_FREN
%token <string> MOD_FROM
%token <string> MOD_TO
%token <string> MOD_BEF
%token <string> MOD_AFT
%token <string> MOD_BET
%token <string> MOD_AND
%token <string> MOD_ABT
%token <string> MOD_CAL
%token <string> MOD_EST
%token <string> MOD_INT
%token <string> MON_JAN
%token <string> MON_FEB
%token <string> MON_MAR
%token <string> MON_APR
%token <string> MON_MAY
%token <string> MON_JUN
%token <string> MON_JUL
%token <string> MON_AUG
%token <string> MON_SEP
%token <string> MON_OCT
%token <string> MON_NOV
%token <string> MON_DEC
%token <string> MON_TSH
%token <string> MON_CSH
%token <string> MON_KSL
%token <string> MON_TVT
%token <string> MON_SHV
%token <string> MON_ADR
%token <string> MON_ADS
%token <string> MON_NSN
%token <string> MON_IYR
%token <string> MON_SVN
%token <string> MON_TMZ
%token <string> MON_AAV
%token <string> MON_ELL
%token <string> MON_VEND
%token <string> MON_BRUM
%token <string> MON_FRIM
%token <string> MON_NIVO
%token <string> MON_PLUV
%token <string> MON_VENT
%token <string> MON_GERM
%token <string> MON_FLOR
%token <string> MON_PRAI
%token <string> MON_MESS
%token <string> MON_THER
%token <string> MON_FRUC
%token <string> MON_COMP
%token <string> OPEN
%token <string> CLOSE
%token <string> TEXT
%token <string> NUMBER
%token <string> SLASH
%token <string> BADTOKEN

%type <date_val> date_value
%type <date_val> date_period
%type <date_val> date_range
%type <date_val> date_approx
%type <date_val> date_interpr
%type <string> date_phrase
%type <date> date

%%

date_value   : date           { make_date_value(DV_NO_MODIFIER,
						$1, def_date, ""); }
             | date_period    
             | date_range
             | date_approx
             | date_interpr
             | date_phrase    { make_date_value(DV_PHRASE,
					        def_date, def_date, $1); }
             | /* empty */
               {
		 /* If empty string: return empty string in 'phrase'
                    member as fallback */
		 /* Note: this can only happen in compatibility mode */
		 make_date_value(DV_PHRASE,
				 def_date, def_date, curr_line_value);
	       }
             | error { /* On error: put entire string in 'phrase' member
			  as fallback */
	               make_date_value(DV_PHRASE,
				       def_date, def_date, curr_line_value);
	             }
             ;

date         : ESC_DATE_GREG date_greg { copy_date(&$$, date_s);
                                         $$.cal = CAL_GREGORIAN; }
             | ESC_DATE_JULN date_juln { copy_date(&$$, date_s);
                                         $$.cal = CAL_JULIAN;  }
             | ESC_DATE_HEBR date_hebr { copy_date(&$$, date_s);
                                         $$.cal = CAL_HEBREW;  }
             | ESC_DATE_FREN date_fren { copy_date(&$$, date_s);
                                         $$.cal = CAL_FRENCH_REV;  }
             | date_greg               { copy_date(&$$, date_s);
                                         $$.cal = CAL_GREGORIAN;  }
             ;

date_period  : MOD_FROM date   { make_date_value(DV_FROM,
						 $2, def_date, ""); }
             | MOD_TO date     { make_date_value(DV_TO,
						 $2, def_date, ""); }
             | MOD_FROM date   { copy_date(&$<date>$, $2); }
	       MOD_TO date
                      { make_date_value(DV_FROM_TO, $<date>3, $5, ""); }
             ;

date_range   : MOD_BEF date    { make_date_value(DV_BEFORE,
						 $2, def_date, ""); }
             | MOD_AFT date    { make_date_value(DV_AFTER,
						 $2, def_date, ""); }
             | MOD_BET date    { copy_date(&$<date>$, $2); }
	       MOD_AND date
                      { make_date_value(DV_BETWEEN, $<date>3, $5, ""); }
             ;

date_approx  : MOD_ABT date    { make_date_value(DV_ABOUT,
						 $2, def_date, ""); }
             | MOD_CAL date    { make_date_value(DV_CALCULATED,
						 $2, def_date, ""); }
             | MOD_EST date    { make_date_value(DV_ESTIMATED,
						 $2, def_date, ""); }
             ;

date_interpr : MOD_INT date date_phrase
                 { make_date_value(DV_INTERPRETED, $2, def_date, $3); }
             ;

date_phrase  : OPEN TEXT CLOSE { $$ = $2; }
             ;

date_greg    : day month_greg year_greg
             | month_greg year_greg
             | year_greg
             ;

date_juln    : day month_greg year
             | month_greg year
             | year
             ;

date_hebr    : day month_hebr year
             | month_hebr year
             | year
             ;

date_fren    : day month_fren year
             | month_fren year
             | year
             ;

day          : NUMBER
               {
		 int d = _get_day_num($1);
		 if (d != -1) {
		   strcpy(date_s.day_str, $1);
		   date_s.day = d;
		 }
	       }
             ;

month_greg   : MON_JAN { strcpy(date_s.month_str, $1);
                         date_s.month = 1; }
             | MON_FEB { strcpy(date_s.month_str, $1);
                         date_s.month = 2; }
             | MON_MAR { strcpy(date_s.month_str, $1);
                         date_s.month = 3; }
             | MON_APR { strcpy(date_s.month_str, $1);
                         date_s.month = 4; }
             | MON_MAY { strcpy(date_s.month_str, $1);
                         date_s.month = 5; }
             | MON_JUN { strcpy(date_s.month_str, $1);
                         date_s.month = 6; }
             | MON_JUL { strcpy(date_s.month_str, $1);
                         date_s.month = 7; }
             | MON_AUG { strcpy(date_s.month_str, $1);
                         date_s.month = 8; }
             | MON_SEP { strcpy(date_s.month_str, $1);
                         date_s.month = 9; }
             | MON_OCT { strcpy(date_s.month_str, $1);
                         date_s.month = 10; }
             | MON_NOV { strcpy(date_s.month_str, $1);
                         date_s.month = 11; }
             | MON_DEC { strcpy(date_s.month_str, $1);
                         date_s.month = 12; }
             ;

month_hebr   : MON_TSH { strcpy(date_s.month_str, $1);
                         date_s.month = 1; }
             | MON_CSH { strcpy(date_s.month_str, $1);
                         date_s.month = 2; }
             | MON_KSL { strcpy(date_s.month_str, $1);
                         date_s.month = 3; }
             | MON_TVT { strcpy(date_s.month_str, $1);
                         date_s.month = 4; }
             | MON_SHV { strcpy(date_s.month_str, $1);
                         date_s.month = 5; }
             | MON_ADR { strcpy(date_s.month_str, $1);
                         date_s.month = 6; }
             | MON_ADS { strcpy(date_s.month_str, $1);
                         date_s.month = 7; }
             | MON_NSN { strcpy(date_s.month_str, $1);
                         date_s.month = 8; }
             | MON_IYR { strcpy(date_s.month_str, $1);
                         date_s.month = 9; }
             | MON_SVN { strcpy(date_s.month_str, $1);
                         date_s.month = 10; }
             | MON_TMZ { strcpy(date_s.month_str, $1);
                         date_s.month = 11; }
             | MON_AAV { strcpy(date_s.month_str, $1);
                         date_s.month = 12; }
             | MON_ELL { strcpy(date_s.month_str, $1);
                         date_s.month = 13; }
             ;

month_fren   : MON_VEND { strcpy(date_s.month_str, $1);
                         date_s.month = 1; }
             | MON_BRUM { strcpy(date_s.month_str, $1);
                         date_s.month = 2; }
             | MON_FRIM { strcpy(date_s.month_str, $1);
                         date_s.month = 3; }
             | MON_NIVO { strcpy(date_s.month_str, $1);
                         date_s.month = 4; }
             | MON_PLUV { strcpy(date_s.month_str, $1);
                         date_s.month = 5; }
             | MON_VENT { strcpy(date_s.month_str, $1);
                         date_s.month = 6; }
             | MON_GERM { strcpy(date_s.month_str, $1);
                         date_s.month = 7; }
             | MON_FLOR { strcpy(date_s.month_str, $1);
                         date_s.month = 8; }
             | MON_PRAI { strcpy(date_s.month_str, $1);
                         date_s.month = 9; }
             | MON_MESS { strcpy(date_s.month_str, $1);
                         date_s.month = 10; }
             | MON_THER { strcpy(date_s.month_str, $1);
                         date_s.month = 11; }
             | MON_FRUC { strcpy(date_s.month_str, $1);
                         date_s.month = 12; }
             | MON_COMP { strcpy(date_s.month_str, $1);
                         date_s.month = 13; }
             ;

year         : NUMBER
                 { int y = _get_year_num(YEAR_SINGLE, $1, NULL);
		   if (y != -1) {
		     strcpy(date_s.year_str, $1);
		     date_s.year = y;
		     date_s.year_type = YEAR_SINGLE;
		   }
		 }
             ;

year_greg    : NUMBER
                 { int y = _get_year_num(YEAR_SINGLE, $1, NULL);
		   if (y != -1) {
		     strcpy(date_s.year_str, $1);
		     date_s.year = y;
		     date_s.year_type = YEAR_SINGLE;
		   }
		 }
             | NUMBER SLASH NUMBER
                 { int y = _get_year_num(YEAR_DOUBLE, $1, $3);
		   if (y != 1) {
		     sprintf(date_s.year_str, "%s/%s", $1, $3);
		     date_s.year = y;
		     date_s.year_type = YEAR_DOUBLE;
		   }
		 }
             ;

%%

int _get_day_num(const char* input)
{
  if (strlen(input) <= MAX_DAY_LEN)
    return atoi(input);
  else {
    gedcom_date_error(_("Too many characters in day '%s'"), input);
    return -1;
  }
}

int get_day_num(const char* input)
{
  int token = get_date_token(input);
  if (token == NUMBER)
    return _get_day_num(input);
  else {
    gedcom_date_error(_("Not a valid day number: '%s'"), input);
    return -1;
  }
}

int begin_month[] =
{ /* CAL_GREGORIAN */   MON_JAN,
  /* CAL_JULIAN */      MON_JAN,
  /* CAL_HEBREW */      MON_TSH,
  /* CAL_FRENCH_REV */  MON_VEND
};

int end_month[] =
{ /* CAL_GREGORIAN */   MON_DEC,
  /* CAL_JULIAN */      MON_DEC,
  /* CAL_HEBREW */      MON_ELL,
  /* CAL_FRENCH_REV */  MON_COMP
};

int get_month_num(Calendar_type cal, const char* input)
{
  int token = get_date_token(input);
  if (token >= begin_month[cal] && token <= end_month[cal])
    return token - begin_month[cal] + 1;
  else {
    gedcom_date_error(_("Not a valid month for the given calendar: '%s'"),
		      input);
    return -1;
  }
}

int _get_year_num(Year_type ytype, const char* input1, const char* input2)
{
  if (ytype == YEAR_SINGLE) {
    if (strlen(input1) <= MAX_YEAR_LEN) {
      return atoi(input1);
    }
    else {
      gedcom_date_error(_("Too many characters in year '%s'"), input1);
      return -1;
    }
  }
  else {
    if (strlen(input1) + strlen(input2) + 1 <= MAX_YEAR_LEN) {
      return atoi(input1) + 1;
    }
    else {
      gedcom_date_error(_("Too many characters in year '%s/%s'"),
			input1, input2);
      return -1;
    }
  }
}

int get_year_num(const char* input, Year_type* ytype)
{
  char *year1, *year2 = NULL;
  int numtok = get_year_tokens(input, &year1, &year2);
  if (numtok) {
    *ytype = (numtok == 1 ? YEAR_SINGLE : YEAR_DOUBLE);
    return _get_year_num (*ytype, year1, year2);
  }
  else {
    gedcom_date_error(_("Not a valid year: '%s'"), input); 
    return -1;
  }
}
