/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

/* In high-low order, a space is encoded as 0x00 0x20 */
/* i.e. this is utf-16-be */

%{
#include "gedcom.tab.h"
#include "gedcom.h"
#include "multilex.h"
#include "encoding.h"

#define YY_NO_UNPUT
%}

%s NORMAL
%s EXPECT_TAG

alpha        \x00[A-Za-z_]
digit        \x00[0-9]
delim        \x00\x20
tab          \x00[\t]
hash         \x00#
literal_at   \x00@\x00@
otherchar    \x00[\x21-\x22\x24-\x2F\x3A-\x3F\x5B-\x5E\x60\x7B-\x7E\x80-\xFF]|[\x01-\xFF][\x00-\xFF]
terminator   \x00\x0D|\x00\x0A|\x00\x0D\x00\x0A|\x00\x0A\x00\x0D

any_char     {alpha}|{digit}|{otherchar}|{delim}|{hash}|{literal_at}
any_but_delim {alpha}|{digit}|{otherchar}|{hash}|{literal_at}
non_at       {alpha}|{digit}|{otherchar}|{delim}|{hash}
alphanum     {alpha}|{digit}
gen_delim    {delim}|{tab}

escape       \x00@\x00#{any_char}+\x00@
pointer      \x00@{alphanum}{non_at}+\x00@

%{
static int current_level=-1;
static int level_diff=MAXGEDCLEVEL;
 
#ifdef LEXER_TEST 
YYSTYPE gedcom_lval;
int line_no = 1; 
#endif
%} 

%%

    /* The GEDCOM level number is converted into a sequence of opening
       and closing brackets.  Simply put, the following GEDCOM fragment:

         0 HEAD
	 1 SOUR genes
	 2 VERS 1.6
	 2 NAME Genes
	 1 DATE 07 OCT 2001
	 ...
	 0 TRLR

       is converted into:

         { HEAD                     (initial)  
	 { SOUR genes               (1 higher: no closing brackets)
	 { VERS 1.6                 (1 higher: no closing brackets)
	 } { NAME Genes             (same level: 1 closing bracket)
	 } } { DATE 07 OCT 2001     (1 lower: 2 closing brackets)
	 ...
	 } { TRLR }

       or more clearly:

         { HEAD
	   { SOUR genes
	     { VERS 1.6 }
	     { NAME Genes } }
	   { DATE 07 OCT 2001
	 ... }
	 { TRLR }

       But because this means that one token is converted into a series
       of tokens, there is some initial code following immediately here
       that returns "pending" tokens. */

%{
char string_buf[MAXGEDCLINELEN+1];
 
if (level_diff < 1) {
  level_diff++;
  return CLOSE;
}
else if (level_diff == 1) {
  level_diff++;
  return OPEN;
}
else {
  /* out of brackets... */
}

#define TO_INTERNAL(str) to_internal(str, yyleng) 

#define MKTAGACTION(tag) \
  { gedcom_lval.string = TO_INTERNAL(yytext); \
    BEGIN(NORMAL); \
    return TAG_##tag; }

%}

<INITIAL>{gen_delim}* /* ignore leading whitespace (also tabs) */

<INITIAL>\x00[0]{digit}+ { gedcom_error ("Level number with leading zero");
                           return BADTOKEN;
                         }

<INITIAL>{digit}+ { int level = atoi(TO_INTERNAL(yytext));
                    if ((level < 0) || (level > MAXGEDCLEVEL)) {
		      gedcom_error ("Level number out of range [0..%d]",
				    MAXGEDCLEVEL);
		      return BADTOKEN;
		    }
                    level_diff = level - current_level;
		    BEGIN(EXPECT_TAG);
		    current_level = level;
		    if (level_diff < 1) {
		      level_diff++;
		      return CLOSE;
		    }
		    else if (level_diff == 1) {
		      level_diff++;
		      return OPEN;
		    }
		    else {
		      /* should never happen (error to GEDCOM spec) */
		      gedcom_error ("GEDCOM level number is %d higher than "
				    "previous",
				    level_diff);
		      return BADTOKEN;
		    }
                  }

<EXPECT_TAG>\x00A\x00B\x00B\x00R  MKTAGACTION(ABBR)
<EXPECT_TAG>\x00A\x00D\x00D\x00R  MKTAGACTION(ADDR)
<EXPECT_TAG>\x00A\x00D\x00R\x001  MKTAGACTION(ADR1)
<EXPECT_TAG>\x00A\x00D\x00R\x002  MKTAGACTION(ADR2)
<EXPECT_TAG>\x00A\x00D\x00O\x00P  MKTAGACTION(ADOP)
<EXPECT_TAG>\x00A\x00F\x00N   MKTAGACTION(AFN)
<EXPECT_TAG>\x00A\x00G\x00E   MKTAGACTION(AGE)
<EXPECT_TAG>\x00A\x00G\x00N\x00C  MKTAGACTION(AGNC)
<EXPECT_TAG>\x00A\x00L\x00I\x00A  MKTAGACTION(ALIA)
<EXPECT_TAG>\x00A\x00N\x00C\x00E  MKTAGACTION(ANCE)
<EXPECT_TAG>\x00A\x00N\x00C\x00I  MKTAGACTION(ANCI)
<EXPECT_TAG>\x00A\x00N\x00U\x00L  MKTAGACTION(ANUL)
<EXPECT_TAG>\x00A\x00S\x00S\x00O  MKTAGACTION(ASSO)
<EXPECT_TAG>\x00A\x00U\x00T\x00H  MKTAGACTION(AUTH)
<EXPECT_TAG>\x00B\x00A\x00P\x00L  MKTAGACTION(BAPL)
<EXPECT_TAG>\x00B\x00A\x00P\x00M  MKTAGACTION(BAPM)
<EXPECT_TAG>\x00B\x00A\x00R\x00M  MKTAGACTION(BARM)
<EXPECT_TAG>\x00B\x00A\x00S\x00M  MKTAGACTION(BASM)
<EXPECT_TAG>\x00B\x00I\x00R\x00T  MKTAGACTION(BIRT)
<EXPECT_TAG>\x00B\x00L\x00E\x00S  MKTAGACTION(BLES)
<EXPECT_TAG>\x00B\x00L\x00O\x00B  MKTAGACTION(BLOB)
<EXPECT_TAG>\x00B\x00U\x00R\x00I  MKTAGACTION(BURI)
<EXPECT_TAG>\x00C\x00A\x00L\x00N  MKTAGACTION(CALN)
<EXPECT_TAG>\x00C\x00A\x00S\x00T  MKTAGACTION(CAST)
<EXPECT_TAG>\x00C\x00A\x00U\x00S  MKTAGACTION(CAUS)
<EXPECT_TAG>\x00C\x00E\x00N\x00S  MKTAGACTION(CENS)
<EXPECT_TAG>\x00C\x00H\x00A\x00N  MKTAGACTION(CHAN)
<EXPECT_TAG>\x00C\x00H\x00A\x00R  MKTAGACTION(CHAR)
<EXPECT_TAG>\x00C\x00H\x00I\x00L  MKTAGACTION(CHIL)
<EXPECT_TAG>\x00C\x00H\x00R   MKTAGACTION(CHR)
<EXPECT_TAG>\x00C\x00H\x00R\x00A  MKTAGACTION(CHRA)
<EXPECT_TAG>\x00C\x00I\x00T\x00Y  MKTAGACTION(CITY)
<EXPECT_TAG>\x00C\x00O\x00N\x00C  MKTAGACTION(CONC)
<EXPECT_TAG>\x00C\x00O\x00N\x00F  MKTAGACTION(CONF)
<EXPECT_TAG>\x00C\x00O\x00N\x00L  MKTAGACTION(CONL)
<EXPECT_TAG>\x00C\x00O\x00N\x00T  MKTAGACTION(CONT)
<EXPECT_TAG>\x00C\x00O\x00P\x00R  MKTAGACTION(COPR)
<EXPECT_TAG>\x00C\x00O\x00R\x00P  MKTAGACTION(CORP)
<EXPECT_TAG>\x00C\x00R\x00E\x00M  MKTAGACTION(CREM)
<EXPECT_TAG>\x00C\x00T\x00R\x00Y  MKTAGACTION(CTRY)
<EXPECT_TAG>\x00D\x00A\x00T\x00A  MKTAGACTION(DATA)
<EXPECT_TAG>\x00D\x00A\x00T\x00E  MKTAGACTION(DATE)
<EXPECT_TAG>\x00D\x00E\x00A\x00T  MKTAGACTION(DEAT)
<EXPECT_TAG>\x00D\x00E\x00S\x00C  MKTAGACTION(DESC)
<EXPECT_TAG>\x00D\x00E\x00S\x00I  MKTAGACTION(DESI)
<EXPECT_TAG>\x00D\x00E\x00S\x00T  MKTAGACTION(DEST)
<EXPECT_TAG>\x00D\x00I\x00V   MKTAGACTION(DIV)
<EXPECT_TAG>\x00D\x00I\x00V\x00F  MKTAGACTION(DIVF)
<EXPECT_TAG>\x00D\x00S\x00C\x00R  MKTAGACTION(DSCR)
<EXPECT_TAG>\x00E\x00D\x00U\x00C  MKTAGACTION(EDUC)
<EXPECT_TAG>\x00E\x00M\x00I\x00G  MKTAGACTION(EMIG)
<EXPECT_TAG>\x00E\x00N\x00D\x00L  MKTAGACTION(ENDL)
<EXPECT_TAG>\x00E\x00N\x00G\x00A  MKTAGACTION(ENGA)
<EXPECT_TAG>\x00E\x00V\x00E\x00N  MKTAGACTION(EVEN)
<EXPECT_TAG>\x00F\x00A\x00M   MKTAGACTION(FAM)
<EXPECT_TAG>\x00F\x00A\x00M\x00C  MKTAGACTION(FAMC)
<EXPECT_TAG>\x00F\x00A\x00M\x00F  MKTAGACTION(FAMF)
<EXPECT_TAG>\x00F\x00A\x00M\x00S  MKTAGACTION(FAMS)
<EXPECT_TAG>\x00F\x00C\x00O\x00M  MKTAGACTION(FCOM)
<EXPECT_TAG>\x00F\x00I\x00L\x00E  MKTAGACTION(FILE)
<EXPECT_TAG>\x00F\x00O\x00R\x00M  MKTAGACTION(FORM)
<EXPECT_TAG>\x00G\x00E\x00D\x00C  MKTAGACTION(GEDC)
<EXPECT_TAG>\x00G\x00I\x00V\x00N  MKTAGACTION(GIVN)
<EXPECT_TAG>\x00G\x00R\x00A\x00D  MKTAGACTION(GRAD)
<EXPECT_TAG>\x00H\x00E\x00A\x00D  MKTAGACTION(HEAD)
<EXPECT_TAG>\x00H\x00U\x00S\x00B  MKTAGACTION(HUSB)
<EXPECT_TAG>\x00I\x00D\x00N\x00O  MKTAGACTION(IDNO)
<EXPECT_TAG>\x00I\x00M\x00M\x00I  MKTAGACTION(IMMI)
<EXPECT_TAG>\x00I\x00N\x00D\x00I  MKTAGACTION(INDI)
<EXPECT_TAG>\x00L\x00A\x00N\x00G  MKTAGACTION(LANG)
<EXPECT_TAG>\x00L\x00E\x00G\x00A  MKTAGACTION(LEGA)
<EXPECT_TAG>\x00M\x00A\x00R\x00B  MKTAGACTION(MARB)
<EXPECT_TAG>\x00M\x00A\x00R\x00C  MKTAGACTION(MARC)
<EXPECT_TAG>\x00M\x00A\x00R\x00L  MKTAGACTION(MARL)
<EXPECT_TAG>\x00M\x00A\x00R\x00R  MKTAGACTION(MARR)
<EXPECT_TAG>\x00M\x00A\x00R\x00S  MKTAGACTION(MARS)
<EXPECT_TAG>\x00M\x00E\x00D\x00I  MKTAGACTION(MEDI)
<EXPECT_TAG>\x00N\x00A\x00M\x00E  MKTAGACTION(NAME)
<EXPECT_TAG>\x00N\x00A\x00T\x00I  MKTAGACTION(NATI)
<EXPECT_TAG>\x00N\x00A\x00T\x00U  MKTAGACTION(NATU)
<EXPECT_TAG>\x00N\x00C\x00H\x00I  MKTAGACTION(NCHI)
<EXPECT_TAG>\x00N\x00I\x00C\x00K  MKTAGACTION(NICK)
<EXPECT_TAG>\x00N\x00M\x00R   MKTAGACTION(NMR)
<EXPECT_TAG>\x00N\x00O\x00T\x00E  MKTAGACTION(NOTE)
<EXPECT_TAG>\x00N\x00P\x00F\x00X  MKTAGACTION(NPFX)
<EXPECT_TAG>\x00N\x00S\x00F\x00X  MKTAGACTION(NSFX)
<EXPECT_TAG>\x00O\x00B\x00J\x00E  MKTAGACTION(OBJE)
<EXPECT_TAG>\x00O\x00C\x00C\x00U  MKTAGACTION(OCCU)
<EXPECT_TAG>\x00O\x00R\x00D\x00I  MKTAGACTION(ORDI)
<EXPECT_TAG>\x00O\x00R\x00D\x00N  MKTAGACTION(ORDN)
<EXPECT_TAG>\x00P\x00A\x00G\x00E  MKTAGACTION(PAGE)
<EXPECT_TAG>\x00P\x00E\x00D\x00I  MKTAGACTION(PEDI)
<EXPECT_TAG>\x00P\x00H\x00O\x00N  MKTAGACTION(PHON)
<EXPECT_TAG>\x00P\x00L\x00A\x00C  MKTAGACTION(PLAC)
<EXPECT_TAG>\x00P\x00O\x00S\x00T  MKTAGACTION(POST)
<EXPECT_TAG>\x00P\x00R\x00O\x00B  MKTAGACTION(PROB)
<EXPECT_TAG>\x00P\x00R\x00O\x00P  MKTAGACTION(PROP)
<EXPECT_TAG>\x00P\x00U\x00B\x00L  MKTAGACTION(PUBL)
<EXPECT_TAG>\x00Q\x00U\x00A\x00Y  MKTAGACTION(QUAY)
<EXPECT_TAG>\x00R\x00E\x00F\x00N  MKTAGACTION(REFN)
<EXPECT_TAG>\x00R\x00E\x00L\x00A  MKTAGACTION(RELA)
<EXPECT_TAG>\x00R\x00E\x00L\x00I  MKTAGACTION(RELI)
<EXPECT_TAG>\x00R\x00E\x00P\x00O  MKTAGACTION(REPO)
<EXPECT_TAG>\x00R\x00E\x00S\x00I  MKTAGACTION(RESI)
<EXPECT_TAG>\x00R\x00E\x00S\x00N  MKTAGACTION(RESN)
<EXPECT_TAG>\x00R\x00E\x00T\x00I  MKTAGACTION(RETI)
<EXPECT_TAG>\x00R\x00F\x00N   MKTAGACTION(RFN)
<EXPECT_TAG>\x00R\x00I\x00N   MKTAGACTION(RIN)
<EXPECT_TAG>\x00R\x00O\x00L\x00E  MKTAGACTION(ROLE)
<EXPECT_TAG>\x00S\x00E\x00X   MKTAGACTION(SEX)
<EXPECT_TAG>\x00S\x00L\x00G\x00C  MKTAGACTION(SLGC)
<EXPECT_TAG>\x00S\x00L\x00G\x00S  MKTAGACTION(SLGS)
<EXPECT_TAG>\x00S\x00O\x00U\x00R  MKTAGACTION(SOUR)
<EXPECT_TAG>\x00S\x00P\x00F\x00X  MKTAGACTION(SPFX)
<EXPECT_TAG>\x00S\x00S\x00N   MKTAGACTION(SSN)
<EXPECT_TAG>\x00S\x00T\x00A\x00E  MKTAGACTION(STAE)
<EXPECT_TAG>\x00S\x00T\x00A\x00T  MKTAGACTION(STAT)
<EXPECT_TAG>\x00S\x00U\x00B\x00M  MKTAGACTION(SUBM)
<EXPECT_TAG>\x00S\x00U\x00B\x00N  MKTAGACTION(SUBN)
<EXPECT_TAG>\x00S\x00U\x00R\x00N  MKTAGACTION(SURN)
<EXPECT_TAG>\x00T\x00E\x00M\x00P  MKTAGACTION(TEMP)
<EXPECT_TAG>\x00T\x00E\x00X\x00T  MKTAGACTION(TEXT)
<EXPECT_TAG>\x00T\x00I\x00M\x00E  MKTAGACTION(TIME)
<EXPECT_TAG>\x00T\x00I\x00T\x00L  MKTAGACTION(TITL)
<EXPECT_TAG>\x00T\x00R\x00L\x00R  MKTAGACTION(TRLR)
<EXPECT_TAG>\x00T\x00Y\x00P\x00E  MKTAGACTION(TYPE)
<EXPECT_TAG>\x00V\x00E\x00R\x00S  MKTAGACTION(VERS)
<EXPECT_TAG>\x00W\x00I\x00F\x00E  MKTAGACTION(WIFE)
<EXPECT_TAG>\x00W\x00I\x00L\x00L  MKTAGACTION(WILL)
     
<EXPECT_TAG>{alphanum}+ { if (strlen(yytext) > MAXGEDCTAGLEN) {
                            gedcom_error("Tag '%s' too long, max %d chars");
                            return BADTOKEN;
                          }
                          strncpy(string_buf, yytext, MAXGEDCTAGLEN+1);
			  gedcom_lval.string = TO_INTERNAL(string_buf);
			  BEGIN(NORMAL);
			  return USERTAG;
                        }

{delim}      { gedcom_lval.string = TO_INTERNAL(yytext);
               return DELIM;
             }

{any_but_delim} { gedcom_lval.string = TO_INTERNAL(yytext);
                  return ANYCHAR;
                }

{escape}/{non_at}  { gedcom_lval.string = TO_INTERNAL(yytext);
                     return ESCAPE;
                   }

{pointer}    { gedcom_lval.string = TO_INTERNAL(yytext);
               return POINTER;
             }

   /* Due to the conversion of level numbers into brackets, the
      terminator is not important, so no token is returned here.
      Although not strictly according to the GEDCOM spec, we'll ignore
      whitespace just before the terminator.
   */

{gen_delim}*{terminator} { line_no++; BEGIN(INITIAL); }

   /* Eventually we have to return 1 closing bracket (for the trailer).
      We can detect whether we have sent the closing bracket using the
      level_diff (at eof, first it is 2, then we increment it ourselves) */

<<EOF>> { if (level_diff == 2) {
	    level_diff++;
            return CLOSE;
          }
          else {
	    yyterminate();
	  }
        } 

.  { gedcom_error("Unexpected character: '%s' (0x%02x)",
		  yytext, yytext[0]);
     return BADTOKEN;
   }

%%

int yywrap()
{
  return 1;
}

#ifdef LEXER_TEST

int main()
{
  int tok, res;
  init_encodings();
  set_encoding_width(TWO_BYTE_HILO);
  res = open_conv_to_internal("UNICODE");
  if (!res) {
    gedcom_error("Unable to open conversion context: %s",
		 strerror(errno));
    return 1;
  }
  tok = gedcom_hilo_lex();
  while (tok) {
    switch(tok) {
      case BADTOKEN: printf("BADTOKEN "); break;
      case OPEN: printf("OPEN "); break;
      case CLOSE: printf("CLOSE "); break;
      case ESCAPE: printf("ESCAPE(%s) ", gedcom_lval.string); break;
      case DELIM: printf("DELIM "); break;
      case ANYCHAR: printf("%s ", gedcom_lval.string); break;
      case POINTER: printf("POINTER(%s) ", gedcom_lval.string); break;
      case USERTAG: printf("USERTAG(%s) ", gedcom_lval.string); break;
      default: printf("TAG(%s) ", gedcom_lval.string); break;
    }
    tok = gedcom_hilo_lex();
  }
  printf("\n");
  close_conv_to_internal();
  return 0;
}
#endif
