/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

/* In low-high order, a space is encoded as 0x20 0x00 */
/* i.e. this is utf-16-le */

%{
#include "gedcom.tab.h"
#include "gedcom.h"
#include "multilex.h"
#include "encoding.h"

#define YY_NO_UNPUT
%}

%s NORMAL
%s EXPECT_TAG

alpha        [A-Za-z_]\x00
digit        [0-9]\x00
delim        \x20\x00
tab          [\t]\x00
hash         #\x00
literal_at   @\x00@\x00
otherchar    [\x21-\x22\x24-\x2F\x3A-\x3F\x5B-\x5E\x60\x7B-\x7E\x80-\xFF]\x00|[\x00-\xFF][\x01-\xFF]
terminator   \x0D\x00|\x0A\x00|\x0D\x00\x0A\x00|\x0A\x00\x0D\x00

any_char     {alpha}|{digit}|{otherchar}|{delim}|{hash}|{literal_at}
any_but_delim {alpha}|{digit}|{otherchar}|{hash}|{literal_at}
non_at       {alpha}|{digit}|{otherchar}|{delim}|{hash}
alphanum     {alpha}|{digit}
gen_delim    {delim}|{tab}

escape       @\x00#\x00{any_char}+@\x00
pointer      @\x00{alphanum}{non_at}+@\x00

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
  gedcom_lval.number = current_level;
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
		      gedcom_lval.number = current_level;
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

<EXPECT_TAG>A\x00B\x00B\x00R\x00  MKTAGACTION(ABBR)
<EXPECT_TAG>A\x00D\x00D\x00R\x00  MKTAGACTION(ADDR)
<EXPECT_TAG>A\x00D\x00R\x001\x00  MKTAGACTION(ADR1)
<EXPECT_TAG>A\x00D\x00R\x002\x00  MKTAGACTION(ADR2)
<EXPECT_TAG>A\x00D\x00O\x00P\x00  MKTAGACTION(ADOP)
<EXPECT_TAG>A\x00F\x00N\x00   MKTAGACTION(AFN)
<EXPECT_TAG>A\x00G\x00E\x00   MKTAGACTION(AGE)
<EXPECT_TAG>A\x00G\x00N\x00C\x00  MKTAGACTION(AGNC)
<EXPECT_TAG>A\x00L\x00I\x00A\x00  MKTAGACTION(ALIA)
<EXPECT_TAG>A\x00N\x00C\x00E\x00  MKTAGACTION(ANCE)
<EXPECT_TAG>A\x00N\x00C\x00I\x00  MKTAGACTION(ANCI)
<EXPECT_TAG>A\x00N\x00U\x00L\x00  MKTAGACTION(ANUL)
<EXPECT_TAG>A\x00S\x00S\x00O\x00  MKTAGACTION(ASSO)
<EXPECT_TAG>A\x00U\x00T\x00H\x00  MKTAGACTION(AUTH)
<EXPECT_TAG>B\x00A\x00P\x00L\x00  MKTAGACTION(BAPL)
<EXPECT_TAG>B\x00A\x00P\x00M\x00  MKTAGACTION(BAPM)
<EXPECT_TAG>B\x00A\x00R\x00M\x00  MKTAGACTION(BARM)
<EXPECT_TAG>B\x00A\x00S\x00M\x00  MKTAGACTION(BASM)
<EXPECT_TAG>B\x00I\x00R\x00T\x00  MKTAGACTION(BIRT)
<EXPECT_TAG>B\x00L\x00E\x00S\x00  MKTAGACTION(BLES)
<EXPECT_TAG>B\x00L\x00O\x00B\x00  MKTAGACTION(BLOB)
<EXPECT_TAG>B\x00U\x00R\x00I\x00  MKTAGACTION(BURI)
<EXPECT_TAG>C\x00A\x00L\x00N\x00  MKTAGACTION(CALN)
<EXPECT_TAG>C\x00A\x00S\x00T\x00  MKTAGACTION(CAST)
<EXPECT_TAG>C\x00A\x00U\x00S\x00  MKTAGACTION(CAUS)
<EXPECT_TAG>C\x00E\x00N\x00S\x00  MKTAGACTION(CENS)
<EXPECT_TAG>C\x00H\x00A\x00N\x00  MKTAGACTION(CHAN)
<EXPECT_TAG>C\x00H\x00A\x00R\x00  MKTAGACTION(CHAR)
<EXPECT_TAG>C\x00H\x00I\x00L\x00  MKTAGACTION(CHIL)
<EXPECT_TAG>C\x00H\x00R\x00   MKTAGACTION(CHR)
<EXPECT_TAG>C\x00H\x00R\x00A\x00  MKTAGACTION(CHRA)
<EXPECT_TAG>C\x00I\x00T\x00Y\x00  MKTAGACTION(CITY)
<EXPECT_TAG>C\x00O\x00N\x00C\x00  MKTAGACTION(CONC)
<EXPECT_TAG>C\x00O\x00N\x00F\x00  MKTAGACTION(CONF)
<EXPECT_TAG>C\x00O\x00N\x00L\x00  MKTAGACTION(CONL)
<EXPECT_TAG>C\x00O\x00N\x00T\x00  MKTAGACTION(CONT)
<EXPECT_TAG>C\x00O\x00P\x00R\x00  MKTAGACTION(COPR)
<EXPECT_TAG>C\x00O\x00R\x00P\x00  MKTAGACTION(CORP)
<EXPECT_TAG>C\x00R\x00E\x00M\x00  MKTAGACTION(CREM)
<EXPECT_TAG>C\x00T\x00R\x00Y\x00  MKTAGACTION(CTRY)
<EXPECT_TAG>D\x00A\x00T\x00A\x00  MKTAGACTION(DATA)
<EXPECT_TAG>D\x00A\x00T\x00E\x00  MKTAGACTION(DATE)
<EXPECT_TAG>D\x00E\x00A\x00T\x00  MKTAGACTION(DEAT)
<EXPECT_TAG>D\x00E\x00S\x00C\x00  MKTAGACTION(DESC)
<EXPECT_TAG>D\x00E\x00S\x00I\x00  MKTAGACTION(DESI)
<EXPECT_TAG>D\x00E\x00S\x00T\x00  MKTAGACTION(DEST)
<EXPECT_TAG>D\x00I\x00V\x00   MKTAGACTION(DIV)
<EXPECT_TAG>D\x00I\x00V\x00F\x00  MKTAGACTION(DIVF)
<EXPECT_TAG>D\x00S\x00C\x00R\x00  MKTAGACTION(DSCR)
<EXPECT_TAG>E\x00D\x00U\x00C\x00  MKTAGACTION(EDUC)
<EXPECT_TAG>E\x00M\x00I\x00G\x00  MKTAGACTION(EMIG)
<EXPECT_TAG>E\x00N\x00D\x00L\x00  MKTAGACTION(ENDL)
<EXPECT_TAG>E\x00N\x00G\x00A\x00  MKTAGACTION(ENGA)
<EXPECT_TAG>E\x00V\x00E\x00N\x00  MKTAGACTION(EVEN)
<EXPECT_TAG>F\x00A\x00M\x00   MKTAGACTION(FAM)
<EXPECT_TAG>F\x00A\x00M\x00C\x00  MKTAGACTION(FAMC)
<EXPECT_TAG>F\x00A\x00M\x00F\x00  MKTAGACTION(FAMF)
<EXPECT_TAG>F\x00A\x00M\x00S\x00  MKTAGACTION(FAMS)
<EXPECT_TAG>F\x00C\x00O\x00M\x00  MKTAGACTION(FCOM)
<EXPECT_TAG>F\x00I\x00L\x00E\x00  MKTAGACTION(FILE)
<EXPECT_TAG>F\x00O\x00R\x00M\x00  MKTAGACTION(FORM)
<EXPECT_TAG>G\x00E\x00D\x00C\x00  MKTAGACTION(GEDC)
<EXPECT_TAG>G\x00I\x00V\x00N\x00  MKTAGACTION(GIVN)
<EXPECT_TAG>G\x00R\x00A\x00D\x00  MKTAGACTION(GRAD)
<EXPECT_TAG>H\x00E\x00A\x00D\x00  MKTAGACTION(HEAD)
<EXPECT_TAG>H\x00U\x00S\x00B\x00  MKTAGACTION(HUSB)
<EXPECT_TAG>I\x00D\x00N\x00O\x00  MKTAGACTION(IDNO)
<EXPECT_TAG>I\x00M\x00M\x00I\x00  MKTAGACTION(IMMI)
<EXPECT_TAG>I\x00N\x00D\x00I\x00  MKTAGACTION(INDI)
<EXPECT_TAG>L\x00A\x00N\x00G\x00  MKTAGACTION(LANG)
<EXPECT_TAG>L\x00E\x00G\x00A\x00  MKTAGACTION(LEGA)
<EXPECT_TAG>M\x00A\x00R\x00B\x00  MKTAGACTION(MARB)
<EXPECT_TAG>M\x00A\x00R\x00C\x00  MKTAGACTION(MARC)
<EXPECT_TAG>M\x00A\x00R\x00L\x00  MKTAGACTION(MARL)
<EXPECT_TAG>M\x00A\x00R\x00R\x00  MKTAGACTION(MARR)
<EXPECT_TAG>M\x00A\x00R\x00S\x00  MKTAGACTION(MARS)
<EXPECT_TAG>M\x00E\x00D\x00I\x00  MKTAGACTION(MEDI)
<EXPECT_TAG>N\x00A\x00M\x00E\x00  MKTAGACTION(NAME)
<EXPECT_TAG>N\x00A\x00T\x00I\x00  MKTAGACTION(NATI)
<EXPECT_TAG>N\x00A\x00T\x00U\x00  MKTAGACTION(NATU)
<EXPECT_TAG>N\x00C\x00H\x00I\x00  MKTAGACTION(NCHI)
<EXPECT_TAG>N\x00I\x00C\x00K\x00  MKTAGACTION(NICK)
<EXPECT_TAG>N\x00M\x00R\x00   MKTAGACTION(NMR)
<EXPECT_TAG>N\x00O\x00T\x00E\x00  MKTAGACTION(NOTE)
<EXPECT_TAG>N\x00P\x00F\x00X\x00  MKTAGACTION(NPFX)
<EXPECT_TAG>N\x00S\x00F\x00X\x00  MKTAGACTION(NSFX)
<EXPECT_TAG>O\x00B\x00J\x00E\x00  MKTAGACTION(OBJE)
<EXPECT_TAG>O\x00C\x00C\x00U\x00  MKTAGACTION(OCCU)
<EXPECT_TAG>O\x00R\x00D\x00I\x00  MKTAGACTION(ORDI)
<EXPECT_TAG>O\x00R\x00D\x00N\x00  MKTAGACTION(ORDN)
<EXPECT_TAG>P\x00A\x00G\x00E\x00  MKTAGACTION(PAGE)
<EXPECT_TAG>P\x00E\x00D\x00I\x00  MKTAGACTION(PEDI)
<EXPECT_TAG>P\x00H\x00O\x00N\x00  MKTAGACTION(PHON)
<EXPECT_TAG>P\x00L\x00A\x00C\x00  MKTAGACTION(PLAC)
<EXPECT_TAG>P\x00O\x00S\x00T\x00  MKTAGACTION(POST)
<EXPECT_TAG>P\x00R\x00O\x00B\x00  MKTAGACTION(PROB)
<EXPECT_TAG>P\x00R\x00O\x00P\x00  MKTAGACTION(PROP)
<EXPECT_TAG>P\x00U\x00B\x00L\x00  MKTAGACTION(PUBL)
<EXPECT_TAG>Q\x00U\x00A\x00Y\x00  MKTAGACTION(QUAY)
<EXPECT_TAG>R\x00E\x00F\x00N\x00  MKTAGACTION(REFN)
<EXPECT_TAG>R\x00E\x00L\x00A\x00  MKTAGACTION(RELA)
<EXPECT_TAG>R\x00E\x00L\x00I\x00  MKTAGACTION(RELI)
<EXPECT_TAG>R\x00E\x00P\x00O\x00  MKTAGACTION(REPO)
<EXPECT_TAG>R\x00E\x00S\x00I\x00  MKTAGACTION(RESI)
<EXPECT_TAG>R\x00E\x00S\x00N\x00  MKTAGACTION(RESN)
<EXPECT_TAG>R\x00E\x00T\x00I\x00  MKTAGACTION(RETI)
<EXPECT_TAG>R\x00F\x00N\x00   MKTAGACTION(RFN)
<EXPECT_TAG>R\x00I\x00N\x00   MKTAGACTION(RIN)
<EXPECT_TAG>R\x00O\x00L\x00E\x00  MKTAGACTION(ROLE)
<EXPECT_TAG>S\x00E\x00X\x00   MKTAGACTION(SEX)
<EXPECT_TAG>S\x00L\x00G\x00C\x00  MKTAGACTION(SLGC)
<EXPECT_TAG>S\x00L\x00G\x00S\x00  MKTAGACTION(SLGS)
<EXPECT_TAG>S\x00O\x00U\x00R\x00  MKTAGACTION(SOUR)
<EXPECT_TAG>S\x00P\x00F\x00X\x00  MKTAGACTION(SPFX)
<EXPECT_TAG>S\x00S\x00N\x00   MKTAGACTION(SSN)
<EXPECT_TAG>S\x00T\x00A\x00E\x00  MKTAGACTION(STAE)
<EXPECT_TAG>S\x00T\x00A\x00T\x00  MKTAGACTION(STAT)
<EXPECT_TAG>S\x00U\x00B\x00M\x00  MKTAGACTION(SUBM)
<EXPECT_TAG>S\x00U\x00B\x00N\x00  MKTAGACTION(SUBN)
<EXPECT_TAG>S\x00U\x00R\x00N\x00  MKTAGACTION(SURN)
<EXPECT_TAG>T\x00E\x00M\x00P\x00  MKTAGACTION(TEMP)
<EXPECT_TAG>T\x00E\x00X\x00T\x00  MKTAGACTION(TEXT)
<EXPECT_TAG>T\x00I\x00M\x00E\x00  MKTAGACTION(TIME)
<EXPECT_TAG>T\x00I\x00T\x00L\x00  MKTAGACTION(TITL)
<EXPECT_TAG>T\x00R\x00L\x00R\x00  MKTAGACTION(TRLR)
<EXPECT_TAG>T\x00Y\x00P\x00E\x00  MKTAGACTION(TYPE)
<EXPECT_TAG>V\x00E\x00R\x00S\x00  MKTAGACTION(VERS)
<EXPECT_TAG>W\x00I\x00F\x00E\x00  MKTAGACTION(WIFE)
<EXPECT_TAG>W\x00I\x00L\x00L\x00  MKTAGACTION(WILL)
     
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
  set_encoding_width(TWO_BYTE_LOHI);
  res = open_conv_to_internal("UNICODE");
  if (!res) {
    gedcom_error("Unable to open conversion context: %s",
		 strerror(errno));
    return 1;
  }
  tok = gedcom_lohi_lex();
  while (tok) {
    switch(tok) {
      case BADTOKEN: printf("BADTOKEN "); break;
      case OPEN: printf("OPEN(%d) ", gedcom_lval.number); break;
      case CLOSE: printf("CLOSE "); break;
      case ESCAPE: printf("ESCAPE(%s) ", gedcom_lval.string); break;
      case DELIM: printf("DELIM "); break;
      case ANYCHAR: printf("%s ", gedcom_lval.string); break;
      case POINTER: printf("POINTER(%s) ", gedcom_lval.string); break;
      case USERTAG: printf("USERTAG(%s) ", gedcom_lval.string); break;
      default: printf("TAG(%s) ", gedcom_lval.string); break;
    }
    tok = gedcom_lohi_lex();
  }
  printf("\n");
  close_conv_to_internal();
  return 0;
}
#endif
