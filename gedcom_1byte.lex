/* $Id$ */
/* $Name$ */

%{
#include "gedcom.tab.h"
#include "gedcom.h"
#include "multilex.h"
#include "encoding.h"

#define YY_NO_UNPUT
%}

%s NORMAL
%s EXPECT_TAG

alpha        [A-Za-z_]
digit        [0-9]
delim        " "
tab          [\t]
hash         #
literal_at   @@
otherchar    [\x21-\x22\x24-\x2F\x3A-\x3F\x5B-\x5E\x60\x7B-\x7E\x80-\xFE]
terminator   \x0D|\x0A|\x0D\x0A|\x0A\x0D

any_char     {alpha}|{digit}|{otherchar}|{delim}|{hash}|{literal_at}
any_but_delim {alpha}|{digit}|{otherchar}|{hash}|{literal_at}
non_at       {alpha}|{digit}|{otherchar}|{delim}|{hash}
alphanum     {alpha}|{digit}
gen_delim    {delim}|{tab}

escape       @#{any_char}+@
pointer      @{alphanum}{non_at}+@

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

<INITIAL>0{digit}+ { gedcom_error ("Level number with leading zero");
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

<EXPECT_TAG>ABBR  MKTAGACTION(ABBR)
<EXPECT_TAG>ADDR  MKTAGACTION(ADDR)
<EXPECT_TAG>ADR1  MKTAGACTION(ADR1)
<EXPECT_TAG>ADR2  MKTAGACTION(ADR2)
<EXPECT_TAG>ADOP  MKTAGACTION(ADOP)
<EXPECT_TAG>AFN   MKTAGACTION(AFN)
<EXPECT_TAG>AGE   MKTAGACTION(AGE)
<EXPECT_TAG>AGNC  MKTAGACTION(AGNC)
<EXPECT_TAG>ALIA  MKTAGACTION(ALIA)
<EXPECT_TAG>ANCE  MKTAGACTION(ANCE)
<EXPECT_TAG>ANCI  MKTAGACTION(ANCI)
<EXPECT_TAG>ANUL  MKTAGACTION(ANUL)
<EXPECT_TAG>ASSO  MKTAGACTION(ASSO)
<EXPECT_TAG>AUTH  MKTAGACTION(AUTH)
<EXPECT_TAG>BAPL  MKTAGACTION(BAPL)
<EXPECT_TAG>BAPM  MKTAGACTION(BAPM)
<EXPECT_TAG>BARM  MKTAGACTION(BARM)
<EXPECT_TAG>BASM  MKTAGACTION(BASM)
<EXPECT_TAG>BIRT  MKTAGACTION(BIRT)
<EXPECT_TAG>BLES  MKTAGACTION(BLES)
<EXPECT_TAG>BLOB  MKTAGACTION(BLOB)
<EXPECT_TAG>BURI  MKTAGACTION(BURI)
<EXPECT_TAG>CALN  MKTAGACTION(CALN)
<EXPECT_TAG>CAST  MKTAGACTION(CAST)
<EXPECT_TAG>CAUS  MKTAGACTION(CAUS)
<EXPECT_TAG>CENS  MKTAGACTION(CENS)
<EXPECT_TAG>CHAN  MKTAGACTION(CHAN)
<EXPECT_TAG>CHAR  MKTAGACTION(CHAR)
<EXPECT_TAG>CHIL  MKTAGACTION(CHIL)
<EXPECT_TAG>CHR   MKTAGACTION(CHR)
<EXPECT_TAG>CHRA  MKTAGACTION(CHRA)
<EXPECT_TAG>CITY  MKTAGACTION(CITY)
<EXPECT_TAG>CONC  MKTAGACTION(CONC)
<EXPECT_TAG>CONF  MKTAGACTION(CONF)
<EXPECT_TAG>CONL  MKTAGACTION(CONL)
<EXPECT_TAG>CONT  MKTAGACTION(CONT)
<EXPECT_TAG>COPR  MKTAGACTION(COPR)
<EXPECT_TAG>CORP  MKTAGACTION(CORP)
<EXPECT_TAG>CREM  MKTAGACTION(CREM)
<EXPECT_TAG>CTRY  MKTAGACTION(CTRY)
<EXPECT_TAG>DATA  MKTAGACTION(DATA)
<EXPECT_TAG>DATE  MKTAGACTION(DATE)
<EXPECT_TAG>DEAT  MKTAGACTION(DEAT)
<EXPECT_TAG>DESC  MKTAGACTION(DESC)
<EXPECT_TAG>DESI  MKTAGACTION(DESI)
<EXPECT_TAG>DEST  MKTAGACTION(DEST)
<EXPECT_TAG>DIV   MKTAGACTION(DIV)
<EXPECT_TAG>DIVF  MKTAGACTION(DIVF)
<EXPECT_TAG>DSCR  MKTAGACTION(DSCR)
<EXPECT_TAG>EDUC  MKTAGACTION(EDUC)
<EXPECT_TAG>EMIG  MKTAGACTION(EMIG)
<EXPECT_TAG>ENDL  MKTAGACTION(ENDL)
<EXPECT_TAG>ENGA  MKTAGACTION(ENGA)
<EXPECT_TAG>EVEN  MKTAGACTION(EVEN)
<EXPECT_TAG>FAM   MKTAGACTION(FAM)
<EXPECT_TAG>FAMC  MKTAGACTION(FAMC)
<EXPECT_TAG>FAMF  MKTAGACTION(FAMF)
<EXPECT_TAG>FAMS  MKTAGACTION(FAMS)
<EXPECT_TAG>FCOM  MKTAGACTION(FCOM)
<EXPECT_TAG>FILE  MKTAGACTION(FILE)
<EXPECT_TAG>FORM  MKTAGACTION(FORM)
<EXPECT_TAG>GEDC  MKTAGACTION(GEDC)
<EXPECT_TAG>GIVN  MKTAGACTION(GIVN)
<EXPECT_TAG>GRAD  MKTAGACTION(GRAD)
<EXPECT_TAG>HEAD  MKTAGACTION(HEAD)
<EXPECT_TAG>HUSB  MKTAGACTION(HUSB)
<EXPECT_TAG>IDNO  MKTAGACTION(IDNO)
<EXPECT_TAG>IMMI  MKTAGACTION(IMMI)
<EXPECT_TAG>INDI  MKTAGACTION(INDI)
<EXPECT_TAG>LANG  MKTAGACTION(LANG)
<EXPECT_TAG>LEGA  MKTAGACTION(LEGA)
<EXPECT_TAG>MARB  MKTAGACTION(MARB)
<EXPECT_TAG>MARC  MKTAGACTION(MARC)
<EXPECT_TAG>MARL  MKTAGACTION(MARL)
<EXPECT_TAG>MARR  MKTAGACTION(MARR)
<EXPECT_TAG>MARS  MKTAGACTION(MARS)
<EXPECT_TAG>MEDI  MKTAGACTION(MEDI)
<EXPECT_TAG>NAME  MKTAGACTION(NAME)
<EXPECT_TAG>NATI  MKTAGACTION(NATI)
<EXPECT_TAG>NATU  MKTAGACTION(NATU)
<EXPECT_TAG>NCHI  MKTAGACTION(NCHI)
<EXPECT_TAG>NICK  MKTAGACTION(NICK)
<EXPECT_TAG>NMR   MKTAGACTION(NMR)
<EXPECT_TAG>NOTE  MKTAGACTION(NOTE)
<EXPECT_TAG>NPFX  MKTAGACTION(NPFX)
<EXPECT_TAG>NSFX  MKTAGACTION(NSFX)
<EXPECT_TAG>OBJE  MKTAGACTION(OBJE)
<EXPECT_TAG>OCCU  MKTAGACTION(OCCU)
<EXPECT_TAG>ORDI  MKTAGACTION(ORDI)
<EXPECT_TAG>ORDN  MKTAGACTION(ORDN)
<EXPECT_TAG>PAGE  MKTAGACTION(PAGE)
<EXPECT_TAG>PEDI  MKTAGACTION(PEDI)
<EXPECT_TAG>PHON  MKTAGACTION(PHON)
<EXPECT_TAG>PLAC  MKTAGACTION(PLAC)
<EXPECT_TAG>POST  MKTAGACTION(POST)
<EXPECT_TAG>PROB  MKTAGACTION(PROB)
<EXPECT_TAG>PROP  MKTAGACTION(PROP)
<EXPECT_TAG>PUBL  MKTAGACTION(PUBL)
<EXPECT_TAG>QUAY  MKTAGACTION(QUAY)
<EXPECT_TAG>REFN  MKTAGACTION(REFN)
<EXPECT_TAG>RELA  MKTAGACTION(RELA)
<EXPECT_TAG>RELI  MKTAGACTION(RELI)
<EXPECT_TAG>REPO  MKTAGACTION(REPO)
<EXPECT_TAG>RESI  MKTAGACTION(RESI)
<EXPECT_TAG>RESN  MKTAGACTION(RESN)
<EXPECT_TAG>RETI  MKTAGACTION(RETI)
<EXPECT_TAG>RFN   MKTAGACTION(RFN)
<EXPECT_TAG>RIN   MKTAGACTION(RIN)
<EXPECT_TAG>ROLE  MKTAGACTION(ROLE)
<EXPECT_TAG>SEX   MKTAGACTION(SEX)
<EXPECT_TAG>SLGC  MKTAGACTION(SLGC)
<EXPECT_TAG>SLGS  MKTAGACTION(SLGS)
<EXPECT_TAG>SOUR  MKTAGACTION(SOUR)
<EXPECT_TAG>SPFX  MKTAGACTION(SPFX)
<EXPECT_TAG>SSN   MKTAGACTION(SSN)
<EXPECT_TAG>STAE  MKTAGACTION(STAE)
<EXPECT_TAG>STAT  MKTAGACTION(STAT)
<EXPECT_TAG>SUBM  MKTAGACTION(SUBM)
<EXPECT_TAG>SUBN  MKTAGACTION(SUBN)
<EXPECT_TAG>SURN  MKTAGACTION(SURN)
<EXPECT_TAG>TEMP  MKTAGACTION(TEMP)
<EXPECT_TAG>TEXT  MKTAGACTION(TEXT)
<EXPECT_TAG>TIME  MKTAGACTION(TIME)
<EXPECT_TAG>TITL  MKTAGACTION(TITL)
<EXPECT_TAG>TRLR  MKTAGACTION(TRLR)
<EXPECT_TAG>TYPE  MKTAGACTION(TYPE)
<EXPECT_TAG>VERS  MKTAGACTION(VERS)
<EXPECT_TAG>WIFE  MKTAGACTION(WIFE)
<EXPECT_TAG>WILL  MKTAGACTION(WILL)
     
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
                  /* Due to character conversions, it is possible
		     that the current character will be combined with
		     the next, and so now we don't have a character yet...
		     This is only applicable to the 1byte case (e.g. ANSEL).
		  */
                  if (strlen(gedcom_lval.string) > 0) 
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
  set_encoding_width(ONE_BYTE);
  res = open_conv_to_internal("ASCII");
  if (!res) {
    gedcom_error("Unable to open conversion context: %s",
		 strerror(errno));
    return 1;
  }
  tok = gedcom_1byte_lex();
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
    tok = gedcom_1byte_lex();
  }
  printf("\n");
  close_conv_to_internal();
  return 0;
}
#endif
