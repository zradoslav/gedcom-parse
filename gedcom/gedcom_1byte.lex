/* Lexer for 1-byte encoding of Gedcom.
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
#define LEX_SECTION 1  /* include only a specific part of the following file */
#define yymyinit gedcom_1byte_myinit
#include "gedcom_lex_common.c"

static size_t encoding_width = 1;
%}

%s NORMAL
%s EXPECT_TAG

alpha        [A-Za-z_]
digit        [0-9]
delim        " "
tab          [\t]
hash         #
literal_at   @@
normal_at    @
otherchar    [\x21-\x22\x24-\x2F\x3A-\x3F\x5B-\x5E\x60\x7B-\x7E\x80-\xFE]
terminator   \x0D|\x0A|\x0D\x0A|\x0A\x0D

any_char     {alpha}|{digit}|{otherchar}|{delim}|{hash}|{literal_at}
any_but_delim {alpha}|{digit}|{otherchar}|{hash}|{literal_at}
non_at       {alpha}|{digit}|{otherchar}|{delim}|{hash}
alphanum     {alpha}|{digit}
gen_delim    {delim}|{tab}

escape       @#{any_char}+@
pointer      @{alphanum}{non_at}+@

%%

%{
#undef LEX_SECTION
#define LEX_SECTION 2  /* include only a specific part of the following file */
#include "gedcom_lex_common.c"

ACTION_BEFORE_REGEXPS
  
%}

<INITIAL>{gen_delim}* ACTION_INITIAL_WHITESPACE

<INITIAL>0{digit}+    ACTION_0_DIGITS

<INITIAL>{digit}+     ACTION_DIGITS

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
     
<EXPECT_TAG>{alphanum}+   ACTION_ALPHANUM

{delim}                   ACTION_DELIM

{any_but_delim}           ACTION_ANY

{escape}/{non_at}         ACTION_ESCAPE

{pointer}                 ACTION_POINTER

{gen_delim}*{terminator}  ACTION_TERMINATOR

<<EOF>>                   ACTION_EOF

{normal_at}               ACTION_NORMAL_AT

.                         ACTION_UNEXPECTED

%%
#undef LEX_SECTION
#define LEX_SECTION 3  /* include only a specific part of the following file */
#include "gedcom_lex_common.c"

int gedcom_check_token(const char* str, ParseState state, int check_token)
{
  int result = 0;
  int token;
  YY_BUFFER_STATE buffer;

  yy_delete_buffer(YY_CURRENT_BUFFER);
  buffer = yy_scan_string(str);

  if (state == STATE_NORMAL)
    BEGIN(NORMAL);
  else if (state == STATE_INITIAL)
    BEGIN(INITIAL);
  else if (state == STATE_EXPECT_TAG)
    BEGIN(EXPECT_TAG);

  /* Input is UTF-8 coming from the application, so bypass iconv */
  dummy_conv = 1;
  token = yylex();
  if (token != check_token)
    result = 1;
  
  if (token != 0) {
    token = yylex();
    if (token != 0)
      result = 1;
  }
  dummy_conv = 0;
  
  yy_delete_buffer(buffer);
  return result;
}

#ifdef LEXER_TEST
int gedcom_lex()
{
  return gedcom_1byte_lex();
}

int main()
{
  return test_loop(ONE_BYTE, "ASCII");
}
#endif
