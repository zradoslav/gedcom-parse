/* Lexer for Gedcom dates
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
#include "date.h"
#include "gedcom_date.tabgen.h"
  
#define YY_NO_UNPUT

static char buf[MAX_DATE_TOKEN][MAX_PHRASE_LEN+1];
static int token_nr = 0; 
%}

%option case-insensitive
%s PHRASE

%%

%{

#define SIMPLE_RETURN(TOKEN) \
  { if (token_nr >= MAX_DATE_TOKEN) { \
      gedcom_date_error(_("Date token stack overflow")); \
      return BADTOKEN; \
    } \
    memset(buf[token_nr], 0, MAX_PHRASE_LEN+1); \
    strncpy(buf[token_nr], yytext, yyleng); \
    gedcom_date_lval.string = buf[token_nr++]; \
    return TOKEN; \
  }

#define ACTION_UNEXPECTED \
  { gedcom_date_error(_("Unexpected input")); \
    return BADTOKEN; \
  }

#define UNKNOWN_CALENDAR_TYPE \
  { gedcom_date_error(_("Unknown calendar type")); \
    return BADTOKEN; \
  }
%}

[ \t]+          /* ignore whitespace between tokens */

@#DGREGORIAN@   SIMPLE_RETURN(ESC_DATE_GREG)
@#DJULIAN@      SIMPLE_RETURN(ESC_DATE_JULN)
@#DHEBREW@      SIMPLE_RETURN(ESC_DATE_HEBR)
"@#DFRENCH R@"  SIMPLE_RETURN(ESC_DATE_FREN)
@#.+@           UNKNOWN_CALENDAR_TYPE
     
FROM            SIMPLE_RETURN(MOD_FROM)
TO              SIMPLE_RETURN(MOD_TO)  
BEF             SIMPLE_RETURN(MOD_BEF) 
AFT             SIMPLE_RETURN(MOD_AFT) 
BET             SIMPLE_RETURN(MOD_BET) 
AND             SIMPLE_RETURN(MOD_AND) 
ABT             SIMPLE_RETURN(MOD_ABT) 
CAL             SIMPLE_RETURN(MOD_CAL) 
EST             SIMPLE_RETURN(MOD_EST) 
INT             SIMPLE_RETURN(MOD_INT)
     
JAN             SIMPLE_RETURN(MON_JAN) 
FEB             SIMPLE_RETURN(MON_FEB) 
MAR             SIMPLE_RETURN(MON_MAR) 
APR             SIMPLE_RETURN(MON_APR) 
MAY             SIMPLE_RETURN(MON_MAY) 
JUN             SIMPLE_RETURN(MON_JUN) 
JUL             SIMPLE_RETURN(MON_JUL) 
AUG             SIMPLE_RETURN(MON_AUG) 
SEP             SIMPLE_RETURN(MON_SEP) 
OCT             SIMPLE_RETURN(MON_OCT) 
NOV             SIMPLE_RETURN(MON_NOV) 
DEC             SIMPLE_RETURN(MON_DEC) 
TSH             SIMPLE_RETURN(MON_TSH) 
CSH             SIMPLE_RETURN(MON_CSH) 
KSL             SIMPLE_RETURN(MON_KSL) 
TVT             SIMPLE_RETURN(MON_TVT) 
SHV             SIMPLE_RETURN(MON_SHV) 
ADR             SIMPLE_RETURN(MON_ADR) 
ADS             SIMPLE_RETURN(MON_ADS) 
NSN             SIMPLE_RETURN(MON_NSN) 
IYR             SIMPLE_RETURN(MON_IYR) 
SVN             SIMPLE_RETURN(MON_SVN) 
TMZ             SIMPLE_RETURN(MON_TMZ) 
AAV             SIMPLE_RETURN(MON_AAV) 
ELL             SIMPLE_RETURN(MON_ELL) 
VEND            SIMPLE_RETURN(MON_VEND)
BRUM            SIMPLE_RETURN(MON_BRUM)
FRIM            SIMPLE_RETURN(MON_FRIM)
NIVO            SIMPLE_RETURN(MON_NIVO)
PLUV            SIMPLE_RETURN(MON_PLUV)
VENT            SIMPLE_RETURN(MON_VENT)
GERM            SIMPLE_RETURN(MON_GERM)
FLOR            SIMPLE_RETURN(MON_FLOR)
PRAI            SIMPLE_RETURN(MON_PRAI)
MESS            SIMPLE_RETURN(MON_MESS)
THER            SIMPLE_RETURN(MON_THER)
FRUC            SIMPLE_RETURN(MON_FRUC)
COMP            SIMPLE_RETURN(MON_COMP)

"("             { BEGIN(PHRASE);
                  SIMPLE_RETURN(OPEN); 
                }
<PHRASE>[^\)]*  SIMPLE_RETURN(TEXT);
<PHRASE>")"     { BEGIN(INITIAL); 
                  SIMPLE_RETURN(CLOSE);
                }
     
"/"             SIMPLE_RETURN(SLASH)
[0-9]+          SIMPLE_RETURN(NUMBER)

.               ACTION_UNEXPECTED

%%

int get_date_token(const char* str)
{
  int token;
  YY_BUFFER_STATE buffer;

  token_nr = 0;
  yy_delete_buffer(YY_CURRENT_BUFFER);
  buffer = yy_scan_string(str);
  token = yylex();
  yy_delete_buffer(buffer);
  return token;
}

int get_year_tokens(const char* str, char** year1, char** year2)
{
  int token;
  YY_BUFFER_STATE buffer;

  token_nr = 0;
  yy_delete_buffer(YY_CURRENT_BUFFER);
  buffer = yy_scan_string(str);

  token = yylex();
  switch (token) {
    case NUMBER: {
      *year1 = buf[token_nr - 1];
      token  = yylex();
      switch (token) {
	case SLASH: {
	  token = yylex();
	  switch (token) {
	    case NUMBER: {
	      *year2 = buf[token_nr - 1];
	      return 2;
	    }
	    default:  return 0;
	  }
	  break;
	}
	case 0:   return 1;
	default:  return 0;
      }
      break;
    }
    case 0:   return 0;
    default:  return 0;
  }
}

int yywrap()
{
  return 1;
}

static YY_BUFFER_STATE hndl;

void init_gedcom_date_lex(const char* string)
{
  token_nr = 0;
  hndl = yy_scan_string(string);
}

void close_gedcom_date_lex()
{
  yy_delete_buffer(hndl);
}
