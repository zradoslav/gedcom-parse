/* Parser for Gedcom.
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

/* WARNING: THIS PARSER RELIES HEAVILY ON SOME FEATURES OF BISON.
   DON'T TRY TO USE IT WITH YACC, IT WON'T WORK...
*/

/* Design of the parser:
   ---------------------
   In general, a GEDCOM file contains records, each consisting of a line
   (which we'll call a section), hierarchically containing other lines
   (subsections of the section).

   This means that in general we have:

     A 'record' is a 'section' (sect) containing 'subsections' (subs)
     Each 'subsection' (sub) is again a specific 'section' (sect)

   In parser notation, this means:

     record : sect

     sect   : <some prefix> subs <some suffix>

     subs   : <empty> | subs sub

     sub    : sect_a | sect_b | ...

   This pattern is repeated throughout the parser for the different types of
   sections.
   

   Cardinality of the subsections:
   -------------------------------
   Note that in the above, the order of the subsections is of no importance.
   Indeed, this is the case in the GEDCOM grammar.  However, this also makes
   it difficult to check whether there are not too many subsections of a
   specific type, or whether a mandatory subsection is indeed there.

   Suppose there is a section A that can contain 0 or 1 B section and
   2 C sections.

   This can be expressed in parser notation as follows:

     A    : CC | BCC | CBC | CCB

   So, cardinality is indeed expressable.  However, as the number of subsection
   types and the limits grow bigger (and even theoretically limitless), listing
   all possible permutations becomes quickly unfeasible.

   Much simpler is to say:

     A    : subs
     subs : <empty> | subs sub
     sub  : B | C

   and then check the cardinality in the semantic actions, which is the
   solution chosen in the parser below, using the following macros:

    - OPEN(<parent>)
         Make a new context for the <parent> tag to count child tags in
	 
    - OCCUR2(<child>, <min>, <max>)
         Express that the <child> tag should occur at least <min> times and
	 at most <max> tags within its parent

	 What this actually does is the following.  It increments the counter
	 for that tag and then checks whether the maximum is exceeded.  If so,
	 then a parser error is produced.  The minimum is not actually checked
	 by this macro, but it makes the statements more declarative.

    - OCCUR1(<child>, <min>)
         Express that the <child> tag should occur at least <min> times within
	 its parent (no upper limit)

	 Actually, this only increments the counter for the tag, but it looks
	 very like the previous macro.

	 If the minimum is 0, it is not necessary to express this constraint.

    - CHECKn(<child1>, ..., <childn>)
         This closes the context for the parent tag and checks whether the
	 given <child> tags did effectively occur within the parent (i.e.
	 these are the tags that were mandatory).

	 Since the <min> values above are always 0 or 1 in GEDCOM, this is
	 sufficient.  All sub-tags that declare a minimum of 1 in the OCCUR
	 macros should be listed in this macro here.

	 The macros CHECK0 to CHECK4 are defined like this (the first one
	 has no arguments and is only used to close the parent context; note
	 that this is necessary for correct functioning).

   Example of usage:

     Only sections that have subsections need to use these macros.  This can
     be done like this (the OPEN and CHECK macros are used as mid-rule
     actions around the subsections):

       head_sect : OPEN DELIM TAG_HEAD
                   { OPEN(HEAD) }
                   head_subs
                   { CHECK1(SOUR) }
                   CLOSE { <semantic actions> }
		  
       head_subs : <empty>
                 | head_subs head_sub
                 ;

       head_sub  : head_sour_sect  { OCCUR2(SOUR, 1, 1) }
                 | head_dest_sect  { OCCUR2(DEST, 0, 1) }
                 | head_date_sect  { OCCUR2(DATE, 0, 1) }
		 ;
*/

/* General notes:

   - The syntax analysis doesn't handle the contents of the line values;
     this is done in the semantic analysis.

 */

%{
#include "gedcom_internal.h"
#include "multilex.h"
#include "encoding.h"
#include "interface.h"
#include "date.h"

int  count_level    = 0;
int  fail           = 0;
int  compat_enabled = 1;
int  gedcom_high_level_debug = 0; 
int  compatibility  = 0; 
Gedcom_err_mech error_mechanism = IMMED_FAIL;
Gedcom_val_struct val; 
 
char line_item_buf[MAXGEDCLINELEN * UTF_FACTOR + 1];
char *line_item_buf_ptr;

enum _COMPAT {
  C_FTREE = 0x01
};

/* These are defined at the bottom of the file */ 
void push_countarray();
void set_parenttag(char* tag);
char* get_parenttag(); 
void set_parentctxt(Gedcom_ctxt ctxt);
Gedcom_ctxt get_parentctxt();
void pop_countarray();
int  count_tag(int tag);
int  check_occurrence(int tag);
void set_compatibility(char* program);
int  compat_mode(int flags); 

#define CLEAR_BUFFER(BUF)                                                     \
     memset(BUF, 0, sizeof(BUF));
 
#define HANDLE_ERROR                                                          \
     { if (error_mechanism == IMMED_FAIL) {                                   \
	 YYABORT;                                                             \
       }                                                                      \
       else if (error_mechanism == DEFER_FAIL) {                              \
         yyerrok; fail = 1;                                                   \
       }                                                                      \
       else if (error_mechanism == IGNORE_ERRORS) {                           \
	 yyerrok;                                                             \
       }                                                                      \
     }
#define START(PARENTTAG,PARENTCTXT)                                           \
     { ++count_level;                                                         \
       set_parenttag(#PARENTTAG);                                             \
       set_parentctxt(PARENTCTXT);                                            \
       push_countarray();                                                     \
     }
#define PARENT                                                              \
     get_parentctxt()
#define CHK(TAG)                                                              \
     { if (!check_occurrence(TAG_##TAG)) {                                    \
         char* parenttag = get_parenttag();                                   \
         gedcom_error(_("The tag '%s' is mandatory within '%s', but missing"),\
		      #TAG, parenttag);                                       \
         HANDLE_ERROR;                                                        \
       }                                                                      \
     }
#define POP                                                                   \
     { pop_countarray();                                                      \
       --count_level;                                                         \
     }
#define CHECK0 POP; 
#define CHECK1(TAG1) { CHK(TAG1); POP; }
#define CHECK2(TAG1,TAG2)                                                     \
     { CHK(TAG1); CHK(TAG2); POP; }
#define CHECK3(TAG1,TAG2,TAG3)                                                \
     { CHK(TAG1); CHK(TAG2); CHK(TAG3); POP; }
#define CHECK4(TAG1,TAG2,TAG3,TAG4)                                           \
     { CHK(TAG1); CHK(TAG2); CHK(TAG3); CHK(TAG4); POP; } 
#define OCCUR1(CHILDTAG, MIN) { count_tag(TAG_##CHILDTAG); } 
#define OCCUR2(CHILDTAG, MIN, MAX)                                            \
     { int num = count_tag(TAG_##CHILDTAG);                                   \
       if (num > MAX) {                                                       \
         char* parenttag = get_parenttag();                                   \
         gedcom_error(_("The tag '%s' can maximally occur %d time(s) within '%s'"),                                                                          \
		      #CHILDTAG, MAX, parenttag);                             \
         HANDLE_ERROR;                                                        \
       }                                                                      \
     }
#define INVALID_TAG(CHILDTAG)                                                 \
     { char* parenttag = get_parenttag();                                     \
       gedcom_error(_("The tag '%s' is not a valid tag within '%s'"),         \
		    CHILDTAG, parenttag);                                     \
       HANDLE_ERROR;                                                          \
     }
#define INVALID_TOP_TAG(CHILDTAG)                                             \
     { gedcom_error(_("The tag '%s' is not a valid top-level tag"),           \
		    CHILDTAG); \
       HANDLE_ERROR; \
     }

%}

%union {
  int  number;
  char *string;
  Gedcom_ctxt ctxt;
}

%token_table
%expect 300

%token <string> BADTOKEN
%token <number> OPEN
%token <string> CLOSE
%token <string> ESCAPE
%token <string> DELIM
%token <string> ANYCHAR
%token <string> POINTER
%token <string> USERTAG
%token <string> TAG_ABBR
%token <string> TAG_ADDR
%token <string> TAG_ADR1
%token <string> TAG_ADR2
%token <string> TAG_ADOP
%token <string> TAG_AFN
%token <string> TAG_AGE
%token <string> TAG_AGNC
%token <string> TAG_ALIA
%token <string> TAG_ANCE
%token <string> TAG_ANCI
%token <string> TAG_ANUL
%token <string> TAG_ASSO
%token <string> TAG_AUTH
%token <string> TAG_BAPL
%token <string> TAG_BAPM
%token <string> TAG_BARM
%token <string> TAG_BASM
%token <string> TAG_BIRT
%token <string> TAG_BLES
%token <string> TAG_BLOB
%token <string> TAG_BURI
%token <string> TAG_CALN
%token <string> TAG_CAST
%token <string> TAG_CAUS
%token <string> TAG_CENS
%token <string> TAG_CHAN
%token <string> TAG_CHAR
%token <string> TAG_CHIL
%token <string> TAG_CHR
%token <string> TAG_CHRA
%token <string> TAG_CITY
%token <string> TAG_CONC
%token <string> TAG_CONF
%token <string> TAG_CONL
%token <string> TAG_CONT
%token <string> TAG_COPR
%token <string> TAG_CORP
%token <string> TAG_CREM
%token <string> TAG_CTRY
%token <string> TAG_DATA
%token <string> TAG_DATE
%token <string> TAG_DEAT
%token <string> TAG_DESC
%token <string> TAG_DESI
%token <string> TAG_DEST
%token <string> TAG_DIV
%token <string> TAG_DIVF
%token <string> TAG_DSCR
%token <string> TAG_EDUC
%token <string> TAG_EMIG
%token <string> TAG_ENDL
%token <string> TAG_ENGA
%token <string> TAG_EVEN
%token <string> TAG_FAM
%token <string> TAG_FAMC
%token <string> TAG_FAMF
%token <string> TAG_FAMS
%token <string> TAG_FCOM
%token <string> TAG_FILE
%token <string> TAG_FORM
%token <string> TAG_GEDC
%token <string> TAG_GIVN
%token <string> TAG_GRAD
%token <string> TAG_HEAD
%token <string> TAG_HUSB
%token <string> TAG_IDNO
%token <string> TAG_IMMI
%token <string> TAG_INDI
%token <string> TAG_LANG
%token <string> TAG_LEGA
%token <string> TAG_MARB
%token <string> TAG_MARC
%token <string> TAG_MARL
%token <string> TAG_MARR
%token <string> TAG_MARS
%token <string> TAG_MEDI
%token <string> TAG_NAME
%token <string> TAG_NATI
%token <string> TAG_NATU
%token <string> TAG_NCHI
%token <string> TAG_NICK
%token <string> TAG_NMR
%token <string> TAG_NOTE
%token <string> TAG_NPFX
%token <string> TAG_NSFX
%token <string> TAG_OBJE
%token <string> TAG_OCCU
%token <string> TAG_ORDI
%token <string> TAG_ORDN
%token <string> TAG_PAGE
%token <string> TAG_PEDI
%token <string> TAG_PHON
%token <string> TAG_PLAC
%token <string> TAG_POST
%token <string> TAG_PROB
%token <string> TAG_PROP
%token <string> TAG_PUBL
%token <string> TAG_QUAY
%token <string> TAG_REFN
%token <string> TAG_RELA
%token <string> TAG_RELI
%token <string> TAG_REPO
%token <string> TAG_RESI
%token <string> TAG_RESN
%token <string> TAG_RETI
%token <string> TAG_RFN
%token <string> TAG_RIN
%token <string> TAG_ROLE
%token <string> TAG_SEX
%token <string> TAG_SLGC
%token <string> TAG_SLGS
%token <string> TAG_SOUR
%token <string> TAG_SPFX
%token <string> TAG_SSN
%token <string> TAG_STAE
%token <string> TAG_STAT
%token <string> TAG_SUBM
%token <string> TAG_SUBN
%token <string> TAG_SURN
%token <string> TAG_TEMP
%token <string> TAG_TEXT
%token <string> TAG_TIME
%token <string> TAG_TITL
%token <string> TAG_TRLR
%token <string> TAG_TYPE
%token <string> TAG_VERS
%token <string> TAG_WIFE
%token <string> TAG_WILL

%type <string> anystdtag
%type <string> anytoptag
%type <string> line_item
%type <string> line_value
%type <string> mand_line_item
%type <string> mand_pointer
%type <string> note_line_item
%type <string> anychar
%type <string> opt_xref
%type <string> opt_value
%type <ctxt> head_sect

%%

file        : head_sect records trlr_sect
               { if (fail == 1) YYABORT; }
            ;

records     : /* empty */
            | records record
            ;

record      : fam_rec
            | indiv_rec
            | multim_rec
            | note_rec
            | repos_rec
            | source_rec
            | submis_rec
            | submit_rec
            | no_std_rec
	    ;

/*********************************************************************/
/**** Header                                                      ****/
/*********************************************************************/
head_sect    : OPEN DELIM TAG_HEAD
               { $<ctxt>$ = start_record(REC_HEAD, $1, NULL, $3);
	         START(HEAD, $<ctxt>$) }
               head_subs
               { if (compat_mode(C_FTREE))
		   CHECK3(SOUR, GEDC, CHAR)
	         else
		   CHECK4(SOUR, SUBM, GEDC, CHAR)
	       }
               CLOSE
               { end_record(REC_HEAD, $<ctxt>4); }
             ;

head_subs    : /* empty */
             | head_subs head_sub
             ;

head_sub     : head_sour_sect  { OCCUR2(SOUR, 1, 1) }
             | head_dest_sect  { OCCUR2(DEST, 0, 1) }
             | head_date_sect  { OCCUR2(DATE, 0, 1) }
             | head_subm_sect  { OCCUR2(SUBM, 1, 1) }
             | head_subn_sect  { OCCUR2(SUBN, 0, 1) }
             | head_file_sect  { OCCUR2(FILE, 0, 1) }
             | head_copr_sect  { OCCUR2(COPR, 0, 1) }
             | head_gedc_sect  { OCCUR2(GEDC, 1, 1) }
             | head_char_sect  { OCCUR2(CHAR, 1, 1) }
             | head_lang_sect  { OCCUR2(LANG, 0, 1) }
             | head_plac_sect  { OCCUR2(PLAC, 0, 1) }
             | head_note_sect  { OCCUR2(NOTE, 0, 1) }
             | no_std_sub
	     ;

/* HEAD.SOUR */
head_sour_sect : OPEN DELIM TAG_SOUR mand_line_item 
                 { set_compatibility($4);
		   $<ctxt>$ = start_element(ELT_HEAD_SOUR, PARENT,
					    $1, $3, $4,
					    GEDCOM_MAKE_STRING($4));
		   START(SOUR, $<ctxt>$)
		 }
                 head_sour_subs
                 { CHECK0 }
		 CLOSE
                 { end_element(ELT_HEAD_SOUR, PARENT, $<ctxt>5, NULL); }
               ;

head_sour_subs : /* empty */
               | head_sour_subs head_sour_sub
               ;

head_sour_sub : head_sour_vers_sect  { OCCUR2(VERS, 0, 1) }
              | head_sour_name_sect  { OCCUR2(NAME, 0, 1) }
              | head_sour_corp_sect  { OCCUR2(CORP, 0, 1) } 
              | head_sour_data_sect  { OCCUR2(DATA, 0, 1) }
              | no_std_sub
              ;

head_sour_vers_sect : OPEN DELIM TAG_VERS mand_line_item
                      { $<ctxt>$ = start_element(ELT_HEAD_SOUR_VERS, PARENT,
						 $1, $3, $4,
						 GEDCOM_MAKE_STRING($4));
		        START(VERS, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_SOUR_VERS,
				    PARENT, $<ctxt>5, NULL);
		      }
                    ;
head_sour_name_sect : OPEN DELIM TAG_NAME mand_line_item
                      { $<ctxt>$ = start_element(ELT_HEAD_SOUR_NAME, PARENT,
						 $1, $3, $4,
						 GEDCOM_MAKE_STRING($4));
			START(NAME, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_SOUR_NAME,
				    PARENT, $<ctxt>5, NULL);
		      }
                    ;
head_sour_corp_sect : OPEN DELIM TAG_CORP mand_line_item 
                      { $<ctxt>$ = start_element(ELT_HEAD_SOUR_CORP, PARENT,
						 $1, $3, $4, 
						 GEDCOM_MAKE_STRING($4));
			START(CORP, $<ctxt>$)
		      }
                      head_sour_corp_subs
		      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_SOUR_CORP,
				    PARENT, $<ctxt>5, NULL);
		      }
                    ;

head_sour_corp_subs : /* empty */
                    | head_sour_corp_subs head_sour_corp_sub
                    ;

head_sour_corp_sub : addr_struc_sub  /* 0:1 */
                   | no_std_sub
                   ;

head_sour_data_sect : OPEN DELIM TAG_DATA mand_line_item 
                      { $<ctxt>$ = start_element(ELT_HEAD_SOUR_DATA, PARENT,
						 $1, $3, $4, 
						 GEDCOM_MAKE_STRING($4));
			START(DATA, $<ctxt>$)
		      }
                      head_sour_data_subs
                      { CHECK0 }
		      CLOSE
                      { end_element(ELT_HEAD_SOUR_DATA,
				    PARENT, $<ctxt>5, NULL);
		      }
                    ;

head_sour_data_subs : /* empty */
                    | head_sour_data_subs head_sour_data_sub
                    ;

head_sour_data_sub : head_sour_data_date_sect  { OCCUR2(DATE, 0, 1) }
                   | head_sour_data_copr_sect  { OCCUR2(COPR, 0, 1) }
                   | no_std_sub
                   ;

head_sour_data_date_sect : OPEN DELIM TAG_DATE mand_line_item
                           { struct date_value dv = gedcom_parse_date($4);
			     $<ctxt>$ = start_element(ELT_HEAD_SOUR_DATA_DATE,
						      PARENT, $1, $3, $4, 
						      GEDCOM_MAKE_DATE(dv));
			     START(DATE, $<ctxt>$)
			   }
                           no_std_subs
                           { CHECK0 }
                           CLOSE
                           { end_element(ELT_HEAD_SOUR_DATA_DATE,
					 PARENT, $<ctxt>5, NULL);
			   }
                         ;
head_sour_data_copr_sect : OPEN DELIM TAG_COPR mand_line_item
                           { $<ctxt>$ = start_element(ELT_HEAD_SOUR_DATA_COPR,
						      PARENT, $1, $3, $4, 
						      GEDCOM_MAKE_STRING($4));
			     START(COPR, $<ctxt>$)
			   }
                           no_std_subs
                           { CHECK0 }
                           CLOSE
                           { end_element(ELT_HEAD_SOUR_DATA_COPR,
					 PARENT, $<ctxt>5, NULL);
			   }
                         ;

/* HEAD.DEST */
head_dest_sect : OPEN DELIM TAG_DEST mand_line_item
                 { $<ctxt>$ = start_element(ELT_HEAD_DEST,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(DEST, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_DEST,
			       PARENT, $<ctxt>5, NULL);
		 }
               ;

/* HEAD.DATE */
head_date_sect : OPEN DELIM TAG_DATE mand_line_item 
                 { struct date_value dv = gedcom_parse_date($4);
		   $<ctxt>$ = start_element(ELT_HEAD_DATE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_DATE(dv));
		   START(DATE, $<ctxt>$)
		 }
                 head_date_subs
		 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_DATE,
			       PARENT, $<ctxt>5, NULL);
		 }
               ;

head_date_subs : /* empty */
               | head_date_subs head_date_sub
               ;

head_date_sub  : head_date_time_sect  { OCCUR2(TIME, 0, 1) }
               | no_std_sub
               ;

head_date_time_sect : OPEN DELIM TAG_TIME mand_line_item
                      { $<ctxt>$ = start_element(ELT_HEAD_DATE_TIME,
						 PARENT, $1, $3, $4, 
						 GEDCOM_MAKE_STRING($4));
		        START(TIME, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_DATE_TIME,
				    PARENT, $<ctxt>5, NULL);
		      }
                    ;

/* HEAD.SUBM */
head_subm_sect : OPEN DELIM TAG_SUBM mand_pointer
                 { $<ctxt>$ = start_element(ELT_HEAD_SUBM,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(SUBM, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_SUBM,
			       PARENT, $<ctxt>5, NULL);
		 }
               ;
/* HEAD.SUBN */
head_subn_sect : OPEN DELIM TAG_SUBN mand_pointer 
                 { $<ctxt>$ = start_element(ELT_HEAD_SUBN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(SUBN, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_SUBN,
			       PARENT, $<ctxt>5, NULL);
		 }
               ;
/* HEAD.FILE */
head_file_sect : OPEN DELIM TAG_FILE mand_line_item 
                 { $<ctxt>$ = start_element(ELT_HEAD_FILE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(FILE, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_FILE, PARENT, $<ctxt>5, NULL);
		 }
               ;
/* HEAD.COPR */
head_copr_sect : OPEN DELIM TAG_COPR mand_line_item 
                 { $<ctxt>$ = start_element(ELT_HEAD_COPR,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(COPR, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_COPR, PARENT, $<ctxt>5, NULL);
		 }
               ;
/* HEAD.GEDC */
head_gedc_sect : OPEN DELIM TAG_GEDC
                 { $<ctxt>$ = start_element(ELT_HEAD_GEDC,
					    PARENT, $1, $3, NULL, NULL);
		   START(GEDC, $<ctxt>$)
		 }
                 head_gedc_subs
		 { CHECK2(VERS, FORM) }
                 CLOSE
                 { end_element(ELT_HEAD_GEDC, PARENT, $<ctxt>4, NULL);
		 }
               ;

head_gedc_subs : /* empty */
               | head_gedc_subs head_gedc_sub
               ;

head_gedc_sub  : head_gedc_vers_sect  { OCCUR2(VERS, 1, 1) }
               | head_gedc_form_sect  { OCCUR2(FORM, 1, 1) }
               | no_std_sub
               ;
head_gedc_vers_sect : OPEN DELIM TAG_VERS mand_line_item  
                      { $<ctxt>$ = start_element(ELT_HEAD_GEDC_VERS,
						 PARENT, $1, $3, $4, 
						 GEDCOM_MAKE_STRING($4));
		        START(VERS, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_GEDC_VERS,
				    PARENT, $<ctxt>5, NULL);
		      }
                    ;
head_gedc_form_sect : OPEN DELIM TAG_FORM mand_line_item   
                      { $<ctxt>$ = start_element(ELT_HEAD_GEDC_FORM,
						 PARENT, $1, $3, $4, 
						 GEDCOM_MAKE_STRING($4));
		        START(FORM, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_GEDC_FORM,
				    PARENT, $<ctxt>5, NULL);
		      }
                    ;

/* HEAD.CHAR */
head_char_sect : OPEN DELIM TAG_CHAR mand_line_item 
                 { if (open_conv_to_internal($4) == 0) YYERROR;
		   $<ctxt>$ = start_element(ELT_HEAD_CHAR,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(CHAR, $<ctxt>$)
		 }
                 head_char_subs
		 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_CHAR, PARENT, $<ctxt>5, NULL);
		 }
               ;

head_char_subs : /* empty */
               | head_char_subs head_char_sub
               ;

head_char_sub  : head_char_vers_sect  { OCCUR2(VERS, 0, 1) }
               | no_std_sub
               ;
head_char_vers_sect : OPEN DELIM TAG_VERS mand_line_item   
                      { $<ctxt>$ = start_element(ELT_HEAD_CHAR_VERS,
						 PARENT, $1, $3, $4, 
						 GEDCOM_MAKE_STRING($4));
		        START(VERS, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_CHAR_VERS,
				    PARENT, $<ctxt>5, NULL);
		      }
                    ;

/* HEAD.LANG */
head_lang_sect : OPEN DELIM TAG_LANG mand_line_item   
                 { $<ctxt>$ = start_element(ELT_HEAD_LANG,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(LANG, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_LANG, PARENT, $<ctxt>5, NULL);
		 }
               ;
/* HEAD.PLAC */
head_plac_sect : OPEN DELIM TAG_PLAC
                 { $<ctxt>$ = start_element(ELT_HEAD_PLAC,
					    PARENT, $1, $3, NULL, NULL);
		   START(PLAC, $<ctxt>$)
		 }
                 head_plac_subs
		 { CHECK1(FORM) }
                 CLOSE
                 { end_element(ELT_HEAD_PLAC, PARENT, $<ctxt>4, NULL);
		 }
               ;

head_plac_subs : /* empty */
               | head_plac_subs head_plac_sub
               ;

head_plac_sub  : head_plac_form_sect  { OCCUR2(FORM, 1, 1) }
               | no_std_sub
               ;
head_plac_form_sect : OPEN DELIM TAG_FORM mand_line_item   
                      { $<ctxt>$ = start_element(ELT_HEAD_PLAC_FORM,
						 PARENT, $1, $3, $4, 
						 GEDCOM_MAKE_STRING($4));
		        START(FORM, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_PLAC_FORM,
				    PARENT, $<ctxt>5, NULL);
		      }
                    ;

/* HEAD.NOTE */
head_note_sect : OPEN DELIM TAG_NOTE mand_line_item 
                 { $<ctxt>$ = start_element(ELT_HEAD_NOTE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(NOTE, $<ctxt>$)
		 }
                 head_note_subs
		 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_NOTE, PARENT, $<ctxt>5, NULL);
		 }
               ;

head_note_subs : /* empty */
               | head_note_subs head_note_sub
               ;

head_note_sub  : continuation_sub  /* 0:M */
               | no_std_sub
               ;

/*********************************************************************/
/**** Trailer                                                     ****/
/*********************************************************************/
/* Don't need callbacks here, there is no information... */
trlr_sect   : OPEN DELIM TAG_TRLR CLOSE { }
            ;

/*********************************************************************/
/**** Family record                                               ****/
/*********************************************************************/
fam_rec      : OPEN DELIM POINTER DELIM TAG_FAM
               { $<ctxt>$ = start_record(REC_FAM, $1, $3, $5);
		 START(FAM, $<ctxt>$) }
               fam_subs
	       { CHECK0 }
               CLOSE
               { end_record(REC_FAM, $<ctxt>6); }
             ;

fam_subs     : /* empty */
             | fam_subs fam_sub
             ;

fam_sub      : fam_event_struc_sub  /* 0:M */
             | fam_husb_sect  { OCCUR2(HUSB, 0, 1) }
             | fam_wife_sect  { OCCUR2(WIFE, 0, 1) }
             | fam_chil_sect  /* 0:M */
             | fam_nchi_sect  { OCCUR2(NCHI, 0, 1) }
             | fam_subm_sect  /* 0:M */
             | lds_spouse_seal_sub  /* 0:M */
             | source_cit_sub  /* 0:M */
             | multim_link_sub  /* 0:M */
             | note_struc_sub  /* 0:M */
             | ident_struc_sub  /* 0:1 */
             | change_date_sub  /* 0:1 */
             | no_std_sub
             ;

/* FAM.HUSB */
fam_husb_sect : OPEN DELIM TAG_HUSB mand_pointer    
                { $<ctxt>$ = start_element(ELT_FAM_HUSB,
					   PARENT, $1, $3, $4, 
					   GEDCOM_MAKE_STRING($4));
		  START(HUSB, $<ctxt>$)
		}
                no_std_subs
                { CHECK0 }
                CLOSE
                { end_element(ELT_FAM_HUSB, PARENT, $<ctxt>5, NULL);
		}
              ;

/* FAM.WIFE */
fam_wife_sect : OPEN DELIM TAG_WIFE mand_pointer 
                { $<ctxt>$ = start_element(ELT_FAM_WIFE,
					   PARENT, $1, $3, $4, 
					   GEDCOM_MAKE_STRING($4));
		  START(WIFE, $<ctxt>$)
		}
                no_std_subs
                { CHECK0 }
                CLOSE
                { end_element(ELT_FAM_WIFE, PARENT, $<ctxt>5, NULL);
		}
              ;

/* FAM.CHIL */
fam_chil_sect : OPEN DELIM TAG_CHIL mand_pointer
                { $<ctxt>$ = start_element(ELT_FAM_CHIL,
					   PARENT, $1, $3, $4, 
					   GEDCOM_MAKE_STRING($4));
		  START(CHIL, $<ctxt>$) 
		} 
		no_std_subs 
		{ CHECK0 } 
		CLOSE
                { end_element(ELT_FAM_CHIL, PARENT, $<ctxt>5, NULL);
		}
              ;

/* FAM.NCHI */
fam_nchi_sect : OPEN DELIM TAG_NCHI mand_line_item    
                { $<ctxt>$ = start_element(ELT_FAM_NCHI,
					   PARENT, $1, $3, $4, 
					   GEDCOM_MAKE_STRING($4));
		  START(NCHI, $<ctxt>$)  
		}  
		no_std_subs  
		{ CHECK0 }  
		CLOSE
                { end_element(ELT_FAM_NCHI, PARENT, $<ctxt>5, NULL);
		}
              ;

/* FAM.SUBM */
fam_subm_sect : OPEN DELIM TAG_SUBM mand_pointer
                { $<ctxt>$ = start_element(ELT_FAM_SUBM,
					   PARENT, $1, $3, $4, 
					   GEDCOM_MAKE_STRING($4));
		  START(SUBM, $<ctxt>$)   
		}   
		no_std_subs   
		{ CHECK0 }   
		CLOSE
                { end_element(ELT_FAM_SUBM, PARENT, $<ctxt>5, NULL);
		}
              ;

/*********************************************************************/
/**** Individual record                                           ****/
/*********************************************************************/
indiv_rec   : OPEN DELIM POINTER DELIM TAG_INDI
              { $<ctxt>$ = start_record(REC_INDI, $1, $3, $5);
		START(INDI, $<ctxt>$) }
              indi_subs
	      { CHECK0 }
              CLOSE
              { end_record(REC_INDI, $<ctxt>6); }
            ;

indi_subs   : /* empty */
            | indi_subs indi_sub
            ;

indi_sub    : indi_resn_sect  { OCCUR2(RESN, 0, 1) }
            | pers_name_struc_sub  /* 0:M */
            | indi_sex_sect  { OCCUR2(SEX, 0, 1) }
            | indiv_even_struc_sub  /* 0:M */
            | indiv_attr_struc_sub  /* 0:M */
            | lds_indiv_ord_sub  /* 0:M */
            | chi_fam_link_sub  /* 0:M */
            | spou_fam_link_sub  /* 0:M */
            | indi_subm_sect  /* 0:M */
            | assoc_struc_sub  /* 0:M */
            | indi_alia_sect  /* 0:M */
            | indi_anci_sect  /* 0:M */
            | indi_desi_sect  /* 0:M */
            | source_cit_sub  /* 0:M */
            | multim_link_sub  /* 0:M */
            | note_struc_sub  /* 0:M */
            | indi_rfn_sect  { OCCUR2(RFN, 0, 1) }
            | indi_afn_sect  /* 0:M */
            | ident_struc_sub  /* 0:1 */
            | change_date_sub  /* 0:1 */
	    | ftree_addr_sect { if (!compat_mode(C_FTREE))
	                          INVALID_TAG("ADDR");
	                      }
	    | no_std_sub
            ;

/* INDI.RESN */
indi_resn_sect : OPEN DELIM TAG_RESN mand_line_item     
                 { $<ctxt>$ = start_element(ELT_INDI_RESN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(RESN, $<ctxt>$)    
		 }    
		 no_std_subs     
		 { CHECK0 }     
		 CLOSE     
		 { end_element(ELT_INDI_RESN, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* INDI.SEX */
indi_sex_sect  : OPEN DELIM TAG_SEX mand_line_item     
                 { $<ctxt>$ = start_element(ELT_INDI_SEX,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(SEX, $<ctxt>$)     
		 }     
		 no_std_subs     
		 { CHECK0 }     
		 CLOSE     
		 { end_element(ELT_INDI_SEX, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* INDI.SUBM */
indi_subm_sect : OPEN DELIM TAG_SUBM mand_pointer 
                 { $<ctxt>$ = start_element(ELT_INDI_SUBM,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(SUBM, $<ctxt>$)      
		 }      
		 no_std_subs      
		 { CHECK0 }      
		 CLOSE      
		 { end_element(ELT_INDI_SUBM, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* INDI.ALIA */
indi_alia_sect : OPEN DELIM TAG_ALIA mand_pointer
                 { $<ctxt>$ = start_element(ELT_INDI_ALIA,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(ALIA, $<ctxt>$)       
		 }       
		 no_std_subs       
		 { CHECK0 }       
		 CLOSE       
		 { end_element(ELT_INDI_ALIA, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* INDI.ANCI */
indi_anci_sect : OPEN DELIM TAG_ANCI mand_pointer
                 { $<ctxt>$ = start_element(ELT_INDI_ANCI,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(ANCI, $<ctxt>$)        
		 }        
		 no_std_subs        
		 { CHECK0 }        
		 CLOSE        
		 { end_element(ELT_INDI_ANCI, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* INDI.DESI */
indi_desi_sect : OPEN DELIM TAG_DESI mand_pointer
                 { $<ctxt>$ = start_element(ELT_INDI_DESI,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(DESI, $<ctxt>$)         
		 }         
		 no_std_subs         
		 { CHECK0 }         
		 CLOSE         
		 { end_element(ELT_INDI_DESI, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* INDI.RFN */
indi_rfn_sect  : OPEN DELIM TAG_RFN mand_line_item     
                 { $<ctxt>$ = start_element(ELT_INDI_RFN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(RFN, $<ctxt>$)          
		 }          
		 no_std_subs          
		 { CHECK0 }          
		 CLOSE          
		 { end_element(ELT_INDI_RFN, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* INDI.AFN */
indi_afn_sect  : OPEN DELIM TAG_AFN mand_line_item      
                 { $<ctxt>$ = start_element(ELT_INDI_AFN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(AFN, $<ctxt>$)           
		 }           
		 no_std_subs           
		 { CHECK0 }           
		 CLOSE           
		 { end_element(ELT_INDI_AFN, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* INDI.ADDR (Only for 'ftree' compatibility) */
ftree_addr_sect : OPEN DELIM TAG_ADDR opt_line_item
                  { START(ADDR, NULL) } no_std_subs { CHECK0 } CLOSE { }

/*********************************************************************/
/**** Multimedia record                                           ****/
/*********************************************************************/
multim_rec  : OPEN DELIM POINTER DELIM TAG_OBJE
              { $<ctxt>$ = start_record(REC_OBJE, $1, $3, $5);
		START(OBJE, $<ctxt>$) }
              obje_subs
	      { CHECK2(FORM, BLOB) }
              CLOSE
              { end_record(REC_OBJE, $<ctxt>6); }
            ;

obje_subs   : /* empty */
            | obje_subs obje_sub
            ;

obje_sub    : obje_form_sect  { OCCUR2(FORM, 1, 1) }
            | obje_titl_sect  { OCCUR2(TITL, 0, 1) }
            | note_struc_sub  /* 0:M */
            | obje_blob_sect  { OCCUR2(BLOB, 1, 1) }
            | obje_obje_sect  { OCCUR2(OBJE, 0, 1) }
            | ident_struc_sub  /* 0:1 */
            | change_date_sub  /* 0:1 */
            | no_std_sub
            ;

/* OBJE.FORM */
obje_form_sect : OPEN DELIM TAG_FORM mand_line_item       
                 { $<ctxt>$ = start_element(ELT_OBJE_FORM,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(FORM, $<ctxt>$)            
		 }            
		 no_std_subs            
		 { CHECK0 }            
		 CLOSE            
		 { end_element(ELT_OBJE_FORM, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* OBJE.TITL */
obje_titl_sect : OPEN DELIM TAG_TITL mand_line_item       
                 { $<ctxt>$ = start_element(ELT_OBJE_TITL,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(TITL, $<ctxt>$)             
		 }             
		 no_std_subs             
		 { CHECK0 }             
		 CLOSE             
		 { end_element(ELT_OBJE_TITL, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* OBJE.BLOB */
obje_blob_sect : OPEN DELIM TAG_BLOB
                 { $<ctxt>$ = start_element(ELT_OBJE_BLOB,
					    PARENT, $1, $3, NULL, NULL);
		   START(BLOB, $<ctxt>$)              
		 }
                 obje_blob_subs
		 { CHECK1(CONT) }
                 CLOSE              
		 { end_element(ELT_OBJE_BLOB, PARENT, $<ctxt>4, NULL);
		 }
               ;

obje_blob_subs : /* empty */
               | obje_blob_subs obje_blob_sub
               ;

obje_blob_sub  : obje_blob_cont_sect  { OCCUR1(CONT, 1) }
               | no_std_sub
               ;

obje_blob_cont_sect : OPEN DELIM TAG_CONT mand_line_item        
                      { $<ctxt>$ = start_element(ELT_OBJE_BLOB_CONT,
					         PARENT, $1, $3, $4, 
						 GEDCOM_MAKE_STRING($4));
		        START(CONT, $<ctxt>$)               
		      }                
		      no_std_subs                
		      { CHECK0 }                
		      CLOSE                
		      { end_element(ELT_OBJE_BLOB_CONT, PARENT,
				    $<ctxt>5, NULL);
		      }
                    ;

/* OBJE.OBJE */
obje_obje_sect : OPEN DELIM TAG_OBJE mand_pointer 
                 { $<ctxt>$ = start_element(ELT_OBJE_OBJE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(OBJE, $<ctxt>$)  
                 }  
                 no_std_subs  
                 { CHECK0 }  
                 CLOSE  
                 { end_element(ELT_OBJE_OBJE, PARENT, $<ctxt>5, NULL);
		 }
               ;

/*********************************************************************/
/**** Note record                                                 ****/
/*********************************************************************/
note_rec    : OPEN DELIM POINTER DELIM TAG_NOTE note_line_item
              { $<ctxt>$ = start_record(REC_NOTE, $1, $3, $5);
		START(NOTE, $<ctxt>$) }
              note_subs
	      { CHECK0 }
              CLOSE
              { end_record(REC_NOTE, $<ctxt>6); }
            ;

note_line_item : /* empty */
                   { if (!compat_mode(C_FTREE)) {
		       gedcom_error(_("Missing value")); YYERROR;
		     }
		   }
               | DELIM line_item
                   { gedcom_debug_print("==Val: %s==", $2);
		     $$ = $2; }
               ;

note_subs   : /* empty */
            | note_subs note_sub
            ;

note_sub    : continuation_sub  /* 0:M */
            | source_cit_sub  /* 0:M */
            | ident_struc_sub  /* 0:1 */
            | change_date_sub  /* 0:1 */
            | no_std_sub
            ;

/*********************************************************************/
/**** Repository record                                           ****/
/*********************************************************************/
repos_rec   : OPEN DELIM POINTER DELIM TAG_REPO
              { $<ctxt>$ = start_record(REC_REPO, $1, $3, $5);
		START(REPO, $<ctxt>$) }
              repo_subs
	      { CHECK0 }
              CLOSE
              { end_record(REC_REPO, $<ctxt>6); }
            ;

repo_subs   : /* empty */
            | repo_subs repo_sub
            ;

repo_sub    : repo_name_sect  { OCCUR2(NAME, 0, 1) }
            | addr_struc_sub  /* 0:1 */
            | note_struc_sub  /* 0:M */
            | ident_struc_sub  /* 0:1 */
            | change_date_sub  /* 0:1 */
            | no_std_sub
            ;

/* REPO.NAME */
repo_name_sect : OPEN DELIM TAG_NAME mand_line_item         
                 { $<ctxt>$ = start_element(ELT_REPO_NAME,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(NAME, $<ctxt>$)          
                 }          
                 no_std_subs          
                 { CHECK0 }          
                 CLOSE          
                 { end_element(ELT_REPO_NAME, PARENT, $<ctxt>5, NULL);
		 }
               ;

/*********************************************************************/
/**** Source record                                               ****/
/*********************************************************************/
source_rec  : OPEN DELIM POINTER DELIM TAG_SOUR
              { $<ctxt>$ = start_record(REC_SOUR, $1, $3, $5);
		START(SOUR, $<ctxt>$) }
              sour_subs
	      { CHECK0 }
              CLOSE
              { end_record(REC_SOUR, $<ctxt>6); }
            ;

sour_subs   : /* empty */
            | sour_subs sour_sub
            ;

sour_sub    : sour_data_sect  { OCCUR2(DATA, 0, 1) }
            | sour_auth_sect  { OCCUR2(AUTH, 0, 1) }
            | sour_titl_sect  { OCCUR2(TITL, 0, 1) }
            | sour_abbr_sect  { OCCUR2(ABBR, 0, 1) }
            | sour_publ_sect  { OCCUR2(PUBL, 0, 1) }
            | sour_text_sect  { OCCUR2(TEXT, 0, 1) }
            | source_repos_cit_sub  /* 0:1 */
            | multim_link_sub  /* 0:M */
            | note_struc_sub  /* 0:M */
            | ident_struc_sub  /* 0:1 */
            | change_date_sub  /* 0:1 */
            | no_std_sub
            ;

/* SOUR.DATA */
sour_data_sect : OPEN DELIM TAG_DATA
                 { $<ctxt>$ = start_element(ELT_SOUR_DATA,
					    PARENT, $1, $3, NULL, NULL);
		   START(DATA, $<ctxt>$) 
                 }
                 sour_data_subs
		 { CHECK0 }
                 CLOSE 
                 { end_element(ELT_SOUR_DATA, PARENT, $<ctxt>4, NULL);
		 }
               ;

sour_data_subs : /* empty */
               | sour_data_subs sour_data_sub
               ;

sour_data_sub  : sour_data_even_sect  /* 0:M */
               | sour_data_agnc_sect  { OCCUR2(AGNC, 0, 1) }
               | note_struc_sub  /* 0:M */
	       | no_std_sub
               ;

sour_data_even_sect : OPEN DELIM TAG_EVEN mand_line_item 
                      { $<ctxt>$ = start_element(ELT_SOUR_DATA_EVEN,
						 PARENT, $1, $3, $4, 
						 GEDCOM_MAKE_STRING($4));
		        START(EVEN, $<ctxt>$)  
                      }
                      sour_data_even_subs
		      { CHECK0 }
                      CLOSE  
                      { end_element(ELT_SOUR_DATA_EVEN, PARENT,
				    $<ctxt>5, NULL);
		      }
                    ;

sour_data_even_subs : /* empty */
                    | sour_data_even_subs sour_data_even_sub
                    ;

sour_data_even_sub  : sour_data_even_date_sect { OCCUR2(DATE, 0, 1) }
                    | sour_data_even_plac_sect { OCCUR2(PLAC, 0, 1) }
                    | no_std_sub
                    ;

sour_data_even_date_sect : OPEN DELIM TAG_DATE mand_line_item          
                           { struct date_value dv = gedcom_parse_date($4);
			     $<ctxt>$ = start_element(ELT_SOUR_DATA_EVEN_DATE,
						      PARENT, $1, $3, $4, 
						      GEDCOM_MAKE_DATE(dv));
		             START(DATE, $<ctxt>$)           
                           }           
                           no_std_subs           
                           { CHECK0 }           
                           CLOSE           
                           { end_element(ELT_SOUR_DATA_EVEN_DATE, PARENT,
					 $<ctxt>5, NULL);
		           }
                         ;

sour_data_even_plac_sect : OPEN DELIM TAG_PLAC mand_line_item          
                           { $<ctxt>$ = start_element(ELT_SOUR_DATA_EVEN_PLAC,
						      PARENT, $1, $3, $4, 
						      GEDCOM_MAKE_STRING($4));
		             START(PLAC, $<ctxt>$)           
                           }           
                           no_std_subs           
                           { CHECK0 }           
                           CLOSE           
                           { end_element(ELT_SOUR_DATA_EVEN_PLAC, PARENT,
					 $<ctxt>5, NULL);
		           }
                         ;

sour_data_agnc_sect : OPEN DELIM TAG_AGNC mand_line_item          
                      { $<ctxt>$ = start_element(ELT_SOUR_DATA_AGNC,
						 PARENT, $1, $3, $4, 
						 GEDCOM_MAKE_STRING($4));
		        START(AGNC, $<ctxt>$)           
                      }           
                      no_std_subs           
                      { CHECK0 }           
                      CLOSE           
                      { end_element(ELT_SOUR_DATA_AGNC, PARENT,
				    $<ctxt>5, NULL);
		      }
                    ;

/* SOUR.AUTH */
sour_auth_sect : OPEN DELIM TAG_AUTH mand_line_item
                 { $<ctxt>$ = start_element(ELT_SOUR_AUTH,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(AUTH, $<ctxt>$) 
                 }
                 sour_auth_subs
		 { CHECK0 }
                 CLOSE 
                 { end_element(ELT_SOUR_AUTH, PARENT, $<ctxt>5, NULL);
		 }
               ;

sour_auth_subs : /* empty */
               | sour_auth_subs sour_auth_sub
               ;

sour_auth_sub  : continuation_sub  /* 0:M */
               | no_std_sub
               ;

/* SOUR.TITL */
sour_titl_sect : OPEN DELIM TAG_TITL mand_line_item  
                 { $<ctxt>$ = start_element(ELT_SOUR_TITL,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(TITL, $<ctxt>$)   
                 }
                 sour_titl_subs 
		 { CHECK0 }
                 CLOSE   
                 { end_element(ELT_SOUR_TITL, PARENT, $<ctxt>5, NULL);
		 }
               ;

sour_titl_subs : /* empty */
               | sour_titl_subs sour_titl_sub
               ;

sour_titl_sub  : continuation_sub  /* 0:M */
               | no_std_sub
               ;

/* SOUR.ABBR */
sour_abbr_sect : OPEN DELIM TAG_ABBR mand_line_item           
                 { $<ctxt>$ = start_element(ELT_SOUR_ABBR,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(ABBR, $<ctxt>$)            
                 }            
                 no_std_subs            
                 { CHECK0 }            
                 CLOSE            
                 { end_element(ELT_SOUR_ABBR, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* SOUR.PUBL */
sour_publ_sect : OPEN DELIM TAG_PUBL mand_line_item  
                 { $<ctxt>$ = start_element(ELT_SOUR_PUBL,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(PUBL, $<ctxt>$)            
                 }
                 sour_publ_subs  
		 { CHECK0 }
                 CLOSE            
                 { end_element(ELT_SOUR_PUBL, PARENT, $<ctxt>5, NULL);
		 }
               ;

sour_publ_subs : /* empty */
               | sour_publ_subs sour_publ_sub
               ;

sour_publ_sub  : continuation_sub  /* 0:M */
               | no_std_sub
               ;

/* SOUR.TEXT */
sour_text_sect : OPEN DELIM TAG_TEXT mand_line_item   
                 { $<ctxt>$ = start_element(ELT_SOUR_TEXT,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(TEXT, $<ctxt>$)    
                 }
                 sour_text_subs  
		 { CHECK0 }
                 CLOSE    
                 { end_element(ELT_SOUR_TEXT, PARENT, $<ctxt>5, NULL);
		 }
               ;

sour_text_subs : /* empty */
               | sour_text_subs sour_text_sub
               ;

sour_text_sub  : continuation_sub  /* 0:M */
               | no_std_sub
               ;

/*********************************************************************/
/**** Submission record                                           ****/
/*********************************************************************/
submis_rec  : OPEN DELIM POINTER DELIM TAG_SUBN    
              { $<ctxt>$ = start_record(REC_SUBN, $1, $3, $5);
		START(SUBN, $<ctxt>$) }
              subn_subs
	      { CHECK0 }
              CLOSE
              { end_record(REC_SUBN, $<ctxt>6); }
            ;

subn_subs   : /* empty */
            | subn_subs subn_sub
            ;

subn_sub    : subn_subm_sect  { OCCUR2(SUBM, 0, 1) }
            | subn_famf_sect  { OCCUR2(FAMF, 0, 1) }
            | subn_temp_sect  { OCCUR2(TEMP, 0, 1) }
            | subn_ance_sect  { OCCUR2(ANCE, 0, 1) }
            | subn_desc_sect  { OCCUR2(DESC, 0, 1) }
            | subn_ordi_sect  { OCCUR2(ORDI, 0, 1) }
            | subn_rin_sect  { OCCUR2(RIN, 0, 1) }
            | no_std_sub
            ;

/* SUBN.SUBM */
subn_subm_sect : OPEN DELIM TAG_SUBM mand_pointer
                 { $<ctxt>$ = start_element(ELT_SUBN_SUBM,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(SUBM, $<ctxt>$) 
                 } 
                 no_std_subs 
                 { CHECK0 } 
                 CLOSE 
                 { end_element(ELT_SUBN_SUBM, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* SUBN.FAMF */
subn_famf_sect : OPEN DELIM TAG_FAMF mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_FAMF,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(FAMF, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_FAMF, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* SUBN.TEMP */
subn_temp_sect : OPEN DELIM TAG_TEMP mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_TEMP,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(TEMP, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_TEMP, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* SUBN.ANCE */
subn_ance_sect : OPEN DELIM TAG_ANCE mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_ANCE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(ANCE, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_ANCE, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* SUBN.DESC */
subn_desc_sect : OPEN DELIM TAG_DESC mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_DESC,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(DESC, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_DESC, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* SUBN.ORDI */
subn_ordi_sect : OPEN DELIM TAG_ORDI mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_ORDI,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(ORDI, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_ORDI, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* SUBN.RIN */
subn_rin_sect  : OPEN DELIM TAG_RIN mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_RIN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(RIN, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_RIN, PARENT, $<ctxt>5, NULL);
		 }
               ;

/*********************************************************************/
/**** Submitter record                                            ****/
/*********************************************************************/
submit_rec : OPEN DELIM POINTER DELIM TAG_SUBM    
             { $<ctxt>$ = start_record(REC_SUBM, $1, $3, $5);
		START(SUBM, $<ctxt>$) }
             subm_subs
	     { CHECK1(NAME) }
             CLOSE
             { end_record(REC_SUBM, $<ctxt>6); }
           ;

subm_subs  : /* empty */
           | subm_subs subm_sub
           ;

subm_sub   : subm_name_sect  { OCCUR2(NAME, 0, 1) }
           | addr_struc_sub  /* 0:1 */
           | multim_link_sub  /* 0:M */
           | subm_lang_sect  { OCCUR2(LANG, 0, 3) }
           | subm_rfn_sect  { OCCUR2(RFN, 0, 1) }
           | subm_rin_sect  { OCCUR2(RIN, 0, 1) }
           | change_date_sub  /* 0:1 */
           | no_std_sub
           ;

/* SUBM.NAME */
subm_name_sect : OPEN DELIM TAG_NAME mand_line_item             
                 { $<ctxt>$ = start_element(ELT_SUBM_NAME,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(NAME, $<ctxt>$)              
                 }              
                 no_std_subs              
                 { CHECK0 }              
                 CLOSE              
                 { end_element(ELT_SUBM_NAME, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* SUBM.LANG */
subm_lang_sect : OPEN DELIM TAG_LANG mand_line_item             
                 { $<ctxt>$ = start_element(ELT_SUBM_LANG,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(LANG, $<ctxt>$)              
                 }              
                 no_std_subs              
                 { CHECK0 }              
                 CLOSE              
                 { end_element(ELT_SUBM_LANG, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* SUBM.RFN */
subm_rfn_sect  : OPEN DELIM TAG_RFN mand_line_item             
                 { $<ctxt>$ = start_element(ELT_SUBM_RFN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(RFN, $<ctxt>$)              
                 }              
                 no_std_subs              
                 { CHECK0 }              
                 CLOSE              
                 { end_element(ELT_SUBM_RFN, PARENT, $<ctxt>5, NULL);
		 }
               ;

/* SUBM.RIN */
subm_rin_sect  : OPEN DELIM TAG_RIN mand_line_item             
                 { $<ctxt>$ = start_element(ELT_SUBM_RIN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(RIN, $<ctxt>$)              
                 }              
                 no_std_subs              
                 { CHECK0 }              
                 CLOSE              
                 { end_element(ELT_SUBM_RIN, PARENT, $<ctxt>5, NULL);
		 }
               ;

/*********************************************************************/
/**** Substructures                                               ****/
/*********************************************************************/

/* ADDRESS STRUCTURE */
addr_struc_sub : addr_sect { OCCUR2(ADDR, 0, 1) }
               | phon_sect { OCCUR2(PHON, 0, 3) }
               ;

addr_sect   : OPEN DELIM TAG_ADDR mand_line_item 
              { $<ctxt>$ = start_element(ELT_SUB_ADDR,
					 PARENT, $1, $3, $4, 
					 GEDCOM_MAKE_STRING($4));
	        START(ADDR, $<ctxt>$)  
              }
              addr_subs
	      { CHECK0 }
              CLOSE  
              { end_element(ELT_SUB_ADDR, PARENT, $<ctxt>5, NULL);
	      }
            ;

addr_subs   : /* empty */
            | addr_subs addr_sub
            ;

addr_sub    : addr_cont_sect  /* 0:M */
            | addr_adr1_sect  { OCCUR2(ADR1, 0, 1) }
            | addr_adr2_sect  { OCCUR2(ADR2, 0, 1) }
            | addr_city_sect  { OCCUR2(CITY, 0, 1) }
            | addr_stae_sect  { OCCUR2(STAE, 0, 1) }
            | addr_post_sect  { OCCUR2(POST, 0, 1) }
            | addr_ctry_sect  { OCCUR2(CTRY, 0, 1) }
            | no_std_sub
            ;

addr_cont_sect : OPEN DELIM TAG_CONT mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_CONT,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(CONT, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_CONT, PARENT, $<ctxt>5, NULL);
		 }
               ;
addr_adr1_sect : OPEN DELIM TAG_ADR1 mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_ADR1,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(ADR1, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_ADR1, PARENT, $<ctxt>5, NULL);
		 }
               ;
addr_adr2_sect : OPEN DELIM TAG_ADR2 mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_ADR2,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(ADR2, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_ADR2, PARENT, $<ctxt>5, NULL);
		 }
               ;
addr_city_sect : OPEN DELIM TAG_CITY mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_CITY,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(CITY, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_CITY, PARENT, $<ctxt>5, NULL);
		 }
               ;
addr_stae_sect : OPEN DELIM TAG_STAE mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_STAE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(STAE, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_STAE, PARENT, $<ctxt>5, NULL);
		 }
               ;
addr_post_sect : OPEN DELIM TAG_POST mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_POST,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(POST, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_POST, PARENT, $<ctxt>5, NULL);
		 }
               ;
addr_ctry_sect : OPEN DELIM TAG_CTRY mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_CTRY,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING($4));
		   START(CTRY, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_CTRY, PARENT, $<ctxt>5, NULL);
		 }
               ;

phon_sect   : OPEN DELIM TAG_PHON mand_line_item              
              { $<ctxt>$ = start_element(ELT_SUB_PHON,
					 PARENT, $1, $3, $4, 
					 GEDCOM_MAKE_STRING($4));
	        START(PHON, $<ctxt>$)               
              }               
              no_std_subs               
              { CHECK0 }               
              CLOSE               
              { end_element(ELT_SUB_PHON, PARENT, $<ctxt>5, NULL);
	      }
            ;

/* ASSOCIATION STRUCTURE */
assoc_struc_sub : asso_sect /* 0:M */
                ;

asso_sect : OPEN DELIM TAG_ASSO mand_pointer
            { START(ASSO, NULL) }
            asso_subs
	    { CHECK2(TYPE,RELA) }
            CLOSE { }
          ;

asso_subs : /* empty */
          | asso_type_sect  { OCCUR2(TYPE, 1, 1) }
          | asso_rela_sect  { OCCUR2(RELA, 1, 1) }
          | note_struc_sub
          | source_cit_sub
	  | no_std_sub
          ;

asso_type_sect : OPEN DELIM TAG_TYPE mand_line_item               
                 { START(TYPE, NULL) } no_std_subs { CHECK0 } CLOSE { }
               ;

asso_rela_sect : OPEN DELIM TAG_RELA mand_line_item               
                 { START(RELA, NULL) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* CHANGE DATE */
change_date_sub : change_date_chan_sect  { OCCUR2(CHAN, 0, 1) }
                ;

change_date_chan_sect : OPEN DELIM TAG_CHAN
                        { START(CHAN, NULL) }
                        change_date_chan_subs
			{ CHECK1(DATE) }
                        CLOSE { }
                      ;

change_date_chan_subs : /* empty */
                      | change_date_chan_subs change_date_chan_sub
                      ;

change_date_chan_sub  : change_date_date_sect  { OCCUR2(DATE, 1, 1) }
                      | note_struc_sub
		      | no_std_sub
                      ;

change_date_date_sect : OPEN DELIM TAG_DATE mand_line_item 
                        { START(DATE, NULL) }
                        change_date_date_subs
			{ CHECK0 }
                        CLOSE { }
                      ;

change_date_date_subs : /* empty */
                      | change_date_date_subs change_date_date_sub
                      ;

change_date_date_sub : change_date_date_time_sect  { OCCUR2(TIME, 0, 1) }
                     | no_std_sub
                     ;

change_date_date_time_sect : OPEN DELIM TAG_TIME mand_line_item
                             { START(TIME, NULL) } no_std_subs { CHECK0 } CLOSE { }
                           ;

/* CHILD TO FAMILY LINK */
chi_fam_link_sub : famc_sect  /* 0:M */
                 ;

famc_sect : OPEN DELIM TAG_FAMC mand_pointer
            { START(FAMC, NULL) }
            famc_subs
	    { CHECK0 }
            CLOSE { }
          ;

famc_subs : /* empty */
          | famc_subs famc_sub
          ;

famc_sub  : famc_pedi_sect  /* 0:M */
          | note_struc_sub
          | no_std_sub
          ;

famc_pedi_sect : OPEN DELIM TAG_PEDI mand_line_item 
                 { START(PEDI, NULL) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* CONTINUATION SUBSECTIONS */
continuation_sub : cont_sect  /* 0:M */
                 | conc_sect  /* 0:M */
                 ;

cont_sect : OPEN DELIM TAG_CONT mand_line_item 
            { START(CONT, NULL) } no_std_subs { CHECK0 } CLOSE { }
          ;

conc_sect : OPEN DELIM TAG_CONC mand_line_item 
            { START(CONC, NULL) } no_std_subs { CHECK0 } CLOSE { }
          ; 

/* EVENT DETAIL */
event_detail_sub : event_detail_type_sect  { OCCUR2(TYPE, 0, 1) }
                 | event_detail_date_sect  { OCCUR2(DATE, 0, 1) }
                 | place_struc_sub
                 | addr_struc_sub
                 | event_detail_age_sect  { OCCUR2(AGE, 0, 1) }
                 | event_detail_agnc_sect  { OCCUR2(AGNC, 0, 1) }
                 | event_detail_caus_sect  { OCCUR2(CAUS, 0, 1) }
                 | source_cit_sub
                 | multim_link_sub
                 | note_struc_sub
                 ;

event_detail_type_sect : OPEN DELIM TAG_TYPE mand_line_item 
                         { START(TYPE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                       ;
event_detail_date_sect : OPEN DELIM TAG_DATE mand_line_item 
                         { START(DATE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                       ;
event_detail_age_sect  : OPEN DELIM TAG_AGE mand_line_item 
                         { START(AGE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                       ;
event_detail_agnc_sect : OPEN DELIM TAG_AGNC mand_line_item 
                         { START(AGNC, NULL) } no_std_subs { CHECK0 } CLOSE { }
                       ;
event_detail_caus_sect : OPEN DELIM TAG_CAUS mand_line_item 
                         { START(CAUS, NULL) } no_std_subs { CHECK0 } CLOSE { }
                       ;

/* FAMILY EVENT STRUCTURE */
fam_event_struc_sub : fam_event_sect
                    | fam_gen_even_sect  /* 0:M */
                    ;

fam_event_sect : OPEN DELIM fam_event_tag opt_value fam_event_subs
                 { CHECK0 }
                 CLOSE { }
               ;

fam_event_tag : TAG_ANUL { START(ANUL, NULL) }
              | TAG_CENS { START(CENS, NULL) }
              | TAG_DIV { START(DIV, NULL) }
              | TAG_DIVF { START(DIVF, NULL) }
              | TAG_ENGA { START(ENGA, NULL) }
              | TAG_MARR { START(MARR, NULL) }
              | TAG_MARB { START(MARB, NULL) }
              | TAG_MARC { START(MARC, NULL) }
              | TAG_MARL { START(MARL, NULL) }
              | TAG_MARS { START(MARS, NULL) }
              ;

fam_event_subs : /* empty */
               | fam_event_subs fam_event_sub
               ;

fam_event_sub : event_detail_sub
              | fam_even_husb_sect  { OCCUR2(HUSB, 0, 1) }
              | fam_even_wife_sect  { OCCUR2(WIFE, 0, 1) }
              | no_std_sub
              ;

fam_even_husb_sect : OPEN DELIM TAG_HUSB
                     { START(HUSB, NULL) }
                     fam_even_husb_subs
		     { CHECK1(AGE) }
                     CLOSE { }
                   ;

fam_even_husb_subs : /* empty */
                   | fam_even_husb_subs fam_even_husb_sub
                   ;

fam_even_husb_sub : fam_even_husb_age_sect  { OCCUR2(AGE, 1, 1) }
                  | no_std_sub
                  ;

fam_even_husb_age_sect : OPEN DELIM TAG_AGE mand_line_item  
                         { START(AGE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                       ;

fam_even_wife_sect : OPEN DELIM TAG_WIFE
                     { START(HUSB, NULL) }
                     fam_even_husb_subs
		     { CHECK1(AGE) }
                     CLOSE { }
                   ;

fam_gen_even_sect : OPEN DELIM TAG_EVEN
                    { START(EVEN, NULL) }
                    fam_gen_even_subs
		    { CHECK0 }
                    CLOSE { }
                  ;

fam_gen_even_subs : /* empty */
                  | fam_gen_even_subs fam_gen_even_sub
                  ;

fam_gen_even_sub : event_detail_sub
                 | fam_even_husb_sect  { OCCUR2(HUSB, 0, 1) }
                 | fam_even_wife_sect  { OCCUR2(WIFE, 0, 1) }
                 | no_std_sub
                 ;

/* IDENTIFICATION STRUCTURE */
ident_struc_sub : ident_refn_sect  /* 0:M */
                | ident_rin_sect  { OCCUR2(RIN, 0, 1) }
                ;

ident_refn_sect : OPEN DELIM TAG_REFN mand_line_item 
                  { START(REFN, NULL) }
                  ident_refn_subs
		  { CHECK0 }
                  CLOSE { }
                ;

ident_refn_subs : /* empty */
                | ident_refn_subs ident_refn_sub
                ;

ident_refn_sub  : ident_refn_type_sect  { OCCUR2(TYPE, 0, 1) }
                | no_std_sub
                ;

ident_refn_type_sect : OPEN DELIM TAG_TYPE mand_line_item   
                       { START(TYPE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                     ;

ident_rin_sect  : OPEN DELIM TAG_RIN mand_line_item   
                  { START(RIN, NULL) } no_std_subs { CHECK0 } CLOSE { }
                ;

/* INDIVIDUAL ATTRIBUTE STRUCTURE */
indiv_attr_struc_sub : indiv_cast_sect  /* 0:M */
                     | indiv_dscr_sect  /* 0:M */
                     | indiv_educ_sect  /* 0:M */
                     | indiv_idno_sect  /* 0:M */
                     | indiv_nati_sect  /* 0:M */
                     | indiv_nchi_sect  /* 0:M */
                     | indiv_nmr_sect  /* 0:M */
                     | indiv_occu_sect  /* 0:M */
                     | indiv_prop_sect  /* 0:M */
                     | indiv_reli_sect  /* 0:M */
                     | indiv_resi_sect  /* 0:M */
                     | indiv_ssn_sect  /* 0:M */
                     | indiv_titl_sect  /* 0:M */
                     ;

indiv_cast_sect : OPEN DELIM TAG_CAST mand_line_item 
                  { START(CAST, NULL) }
                  indiv_attr_event_subs
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_dscr_sect : OPEN DELIM TAG_DSCR mand_line_item 
                  { START(DSCR, NULL) }
                  indiv_attr_event_subs
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_educ_sect : OPEN DELIM TAG_EDUC mand_line_item  
                  { START(EDUC, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_idno_sect : OPEN DELIM TAG_IDNO mand_line_item 
                  { START(IDNO, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_nati_sect : OPEN DELIM TAG_NATI mand_line_item 
                  { START(NATI, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_nchi_sect : OPEN DELIM TAG_NCHI mand_line_item 
                  { START(NCHI, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_nmr_sect  : OPEN DELIM TAG_NMR mand_line_item 
                  { START(NMR, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_occu_sect : OPEN DELIM TAG_OCCU mand_line_item 
                  { START(OCCU, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_prop_sect : OPEN DELIM TAG_PROP mand_line_item 
                  { START(PROP, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_reli_sect : OPEN DELIM TAG_RELI mand_line_item 
                  { START(RELI, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_resi_sect : OPEN DELIM TAG_RESI 
                  { START(RESI, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_ssn_sect  : OPEN DELIM TAG_SSN mand_line_item 
                  { START(SSN, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_titl_sect : OPEN DELIM TAG_TITL mand_line_item 
                  { START(TITL, NULL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;

indiv_attr_event_subs : /* empty */
                      | indiv_attr_event_subs indiv_attr_event_sub
                      ;

indiv_attr_event_sub  : event_detail_sub
                      | no_std_sub
                      ;

/* INDIVIDUAL EVENT STRUCTURE */
indiv_even_struc_sub : indiv_birt_sect
                     | indiv_gen_sect
                     | indiv_adop_sect  /* 0:M */
                     | indiv_even_sect  /* 0:M */
                     ;

indiv_birt_sect : OPEN DELIM indiv_birt_tag opt_value indiv_birt_subs
                  { CHECK0 }
                  CLOSE { }
                ;

indiv_birt_tag  : TAG_BIRT { START(BIRT, NULL) }
                | TAG_CHR { START(CHR, NULL) }
                ;

indiv_birt_subs : /* empty */
                | indiv_birt_subs indiv_birt_sub
                ;

indiv_birt_sub  : event_detail_sub
                | indiv_birt_famc_sect  { OCCUR2(FAMC,0, 1) }
                | no_std_sub
                ;

indiv_birt_famc_sect : OPEN DELIM TAG_FAMC mand_pointer
                       { START(FAMC, NULL) } no_std_subs { CHECK0 } CLOSE { }
                     ;

indiv_gen_sect  : OPEN DELIM indiv_gen_tag opt_value indiv_gen_subs
                  { CHECK0 }
                  CLOSE { }
                ;

indiv_gen_tag   : TAG_DEAT { START(DEAT, NULL) }
                | TAG_BURI { START(BURI, NULL) }
                | TAG_CREM { START(CREM, NULL) }
                | TAG_BAPM { START(BAPM, NULL) }
                | TAG_BARM { START(BARM, NULL) }
                | TAG_BASM { START(BASM, NULL) }
                | TAG_BLES { START(BLES, NULL) }
                | TAG_CHRA { START(CHRA, NULL) }
                | TAG_CONF { START(CONF, NULL) }
                | TAG_FCOM { START(FCOM, NULL) }
                | TAG_ORDN { START(ORDN, NULL) }
                | TAG_NATU { START(NATU, NULL) }
                | TAG_EMIG { START(EMIG, NULL) }
                | TAG_IMMI { START(IMMI, NULL) }
                | TAG_CENS { START(CENS, NULL) }
                | TAG_PROB { START(PROB, NULL) }
                | TAG_WILL { START(WILL, NULL) }
                | TAG_GRAD { START(GRAD, NULL) }
                | TAG_RETI { START(RETI, NULL) }
                ;

indiv_gen_subs  : /* empty */
                | indiv_gen_subs indiv_gen_sub
                ;

indiv_gen_sub   : event_detail_sub
                | no_std_sub
                ;

indiv_adop_sect : OPEN DELIM TAG_ADOP opt_value 
                  { START(ADOP, NULL) }
                  indiv_adop_subs
		  { CHECK0 }
                  CLOSE { }
                ;

indiv_adop_subs : /* empty */
                | indiv_adop_subs indiv_adop_sub
                ;

indiv_adop_sub  : event_detail_sub
                | indiv_adop_famc_sect  { OCCUR2(FAMC,0, 1) }
                | no_std_sub
                ;

indiv_adop_famc_sect : OPEN DELIM TAG_FAMC mand_pointer
                       { START(FAMC, NULL) }
                       indiv_adop_famc_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

indiv_adop_famc_subs : /* empty */
                     | indiv_adop_famc_subs indiv_adop_famc_sub
                     ;

indiv_adop_famc_sub  : indiv_adop_famc_adop_sect  { OCCUR2(ADOP,0, 1) }
                     | no_std_sub
                     ;

indiv_adop_famc_adop_sect : OPEN DELIM TAG_ADOP mand_line_item   
                            { START(ADOP, NULL) } no_std_subs { CHECK0 } CLOSE { }
                          ;

indiv_even_sect : OPEN DELIM TAG_EVEN
                  { START(EVEN, NULL) }
                  indiv_gen_subs
		  { CHECK0 }
                  CLOSE { }
                ;

/* LDS INDIVIDUAL ORDINANCE */
lds_indiv_ord_sub : lio_bapl_sect  /* 0:M */
                  | lio_slgc_sect  /* 0:M */
                  ;

lio_bapl_sect : OPEN DELIM lio_bapl_tag lio_bapl_subs
                { CHECK0 }
                CLOSE { }
              ;

lio_bapl_tag  : TAG_BAPL { START(BAPL, NULL) }
              | TAG_CONL { START(CONL, NULL) }
              | TAG_ENDL { START(ENDL, NULL) }
              ;

lio_bapl_subs : /* empty */
              | lio_bapl_subs lio_bapl_sub
              ;

lio_bapl_sub  : lio_bapl_stat_sect  { OCCUR2(STAT, 0, 1) }
              | lio_bapl_date_sect  { OCCUR2(DATE, 0, 1) }
              | lio_bapl_temp_sect  { OCCUR2(TEMP, 0, 1) }
              | lio_bapl_plac_sect  { OCCUR2(PLAC, 0, 1) }
              | source_cit_sub
              | note_struc_sub
	      | no_std_sub
              ;

lio_bapl_stat_sect : OPEN DELIM TAG_STAT mand_line_item   
                     { START(STAT, NULL) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lio_bapl_date_sect : OPEN DELIM TAG_DATE mand_line_item   
                     { START(DATE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lio_bapl_temp_sect : OPEN DELIM TAG_TEMP mand_line_item   
                     { START(TEMP, NULL) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lio_bapl_plac_sect : OPEN DELIM TAG_PLAC mand_line_item   
                     { START(PLAC, NULL) } no_std_subs { CHECK0 } CLOSE { }
                   ;

lio_slgc_sect : OPEN DELIM TAG_SLGC
                { START(SLGC, NULL) }
                lio_slgc_subs
		{ CHECK1(FAMC) }
                CLOSE { }
              ;

lio_slgc_subs : /* empty */
              | lio_slgc_subs lio_slgc_sub
              ;

lio_slgc_sub  : lio_bapl_sub
              | lio_slgc_famc_sect  { OCCUR2(FAMC, 1, 1) }
              ;

lio_slgc_famc_sect : OPEN DELIM TAG_FAMC mand_pointer
                     { START(FAMC, NULL) } no_std_subs { CHECK0 } CLOSE { }
                   ;

/* LDS SPOUSE SEALING */
lds_spouse_seal_sub : lss_slgs_sect
                    ;

lss_slgs_sect : OPEN DELIM TAG_SLGS
                { START(SLGS, NULL) }
                lss_slgs_subs
		{ CHECK0 }
                CLOSE { }
              ;

lss_slgs_subs : /* empty */
              | lss_slgs_subs lss_slgs_sub
              ;

lss_slgs_sub  : lss_slgs_stat_sect  { OCCUR2(STAT, 0, 1) }
              | lss_slgs_date_sect  { OCCUR2(DATE, 0, 1) }
              | lss_slgs_temp_sect  { OCCUR2(TEMP, 0, 1) }
              | lss_slgs_plac_sect  { OCCUR2(PLAC, 0, 1) }
              | source_cit_sub
              | note_struc_sub
	      | no_std_sub
              ;

lss_slgs_stat_sect : OPEN DELIM TAG_STAT mand_line_item   
                     { START(STAT, NULL) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lss_slgs_date_sect : OPEN DELIM TAG_DATE mand_line_item   
                     { START(DATE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lss_slgs_temp_sect : OPEN DELIM TAG_TEMP mand_line_item   
                     { START(TEMP, NULL) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lss_slgs_plac_sect : OPEN DELIM TAG_PLAC mand_line_item   
                     { START(PLAC, NULL) } no_std_subs { CHECK0 } CLOSE { }
                   ;

/* MULTIMEDIA LINK */
multim_link_sub : multim_obje_link_sect
                | multim_obje_emb_sect
                ;

multim_obje_link_sect : OPEN DELIM TAG_OBJE DELIM POINTER    
                        { START(OBJE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                      ;

multim_obje_emb_sect : OPEN DELIM TAG_OBJE
                       { START(OBJE, NULL) }
                       multim_obje_emb_subs
		       { CHECK2(FORM,FILE) }
                       CLOSE { }
                     ;

multim_obje_emb_subs : /* empty */
                     | multim_obje_emb_subs multim_obje_emb_sub
                     ;

multim_obje_emb_sub : multim_obje_form_sect  { OCCUR2(FORM, 1, 1) }
                    | multim_obje_titl_sect  { OCCUR2(TITL, 0, 1) }
                    | multim_obje_file_sect  { OCCUR2(FILE, 1, 1) }
                    | note_struc_sub
		    | no_std_sub
                    ;

multim_obje_form_sect : OPEN DELIM TAG_FORM mand_line_item    
                        { START(FORM, NULL) } no_std_subs { CHECK0 } CLOSE { }
                      ;
multim_obje_titl_sect : OPEN DELIM TAG_TITL mand_line_item    
                        { START(TITL, NULL) } no_std_subs { CHECK0 } CLOSE { }
                      ;
multim_obje_file_sect : OPEN DELIM TAG_FILE mand_line_item    
                        { START(FILE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                      ;

/* NOTE STRUCTURE */
note_struc_sub : note_struc_link_sect  /* 0:M */
               | note_struc_emb_sect  /* 0:M */
               ;

note_struc_link_sect : OPEN DELIM TAG_NOTE DELIM POINTER
                       { START(NOTE, NULL) }
                       note_struc_link_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

note_struc_link_subs : /* empty */
                     | note_struc_link_subs note_struc_link_sub
                     ;

note_struc_link_sub : source_cit_sub
                    | no_std_sub
                    ;

note_struc_emb_sect : OPEN DELIM TAG_NOTE opt_line_item
                      { START(NOTE, NULL) }
                      note_struc_emb_subs
		      { CHECK0 }
                      CLOSE { }
                    ;

note_struc_emb_subs : /* empty */
                    | note_struc_emb_subs note_struc_emb_sub
                    ;

note_struc_emb_sub  : continuation_sub
                    | source_cit_sub
                    | no_std_sub
                    ;

/* PERSONAL NAME STRUCTURE */
pers_name_struc_sub : pers_name_sect /* 0:M */
                    ;

pers_name_sect : OPEN DELIM TAG_NAME mand_line_item 
                 { START(NAME, NULL) }
                 pers_name_subs
		 { CHECK0 }
                 CLOSE { }
               ;

pers_name_subs : /* empty */
               | pers_name_subs pers_name_sub
               ;

pers_name_sub  : pers_name_npfx_sect  { OCCUR2(NPFX, 0, 1) }
               | pers_name_givn_sect  { OCCUR2(GIVN, 0, 1) }
               | pers_name_nick_sect  { OCCUR2(NICK, 0, 1) }
               | pers_name_spfx_sect  { OCCUR2(SPFX, 0, 1) }
               | pers_name_surn_sect  { OCCUR2(SURN, 0, 1) }
               | pers_name_nsfx_sect  { OCCUR2(NSFX, 0, 1) }
               | source_cit_sub
               | note_struc_sub
	       | no_std_sub
               ;

pers_name_npfx_sect : OPEN DELIM TAG_NPFX mand_line_item    
                      { START(NPFX, NULL) } no_std_subs { CHECK0 } CLOSE { }
                    ;
pers_name_givn_sect : OPEN DELIM TAG_GIVN mand_line_item    
                      { START(GIVN, NULL) } no_std_subs { CHECK0 } CLOSE { }
                    ;
pers_name_nick_sect : OPEN DELIM TAG_NICK mand_line_item    
                      { START(NICK, NULL) } no_std_subs { CHECK0 } CLOSE { }
                    ;
pers_name_spfx_sect : OPEN DELIM TAG_SPFX mand_line_item    
                      { START(SPFX, NULL) } no_std_subs { CHECK0 } CLOSE { }
                    ;
pers_name_surn_sect : OPEN DELIM TAG_SURN mand_line_item    
                      { START(SURN, NULL) } no_std_subs { CHECK0 } CLOSE { }
                    ;
pers_name_nsfx_sect : OPEN DELIM TAG_NSFX mand_line_item    
                      { START(NSFX, NULL) } no_std_subs { CHECK0 } CLOSE { }
                    ;

/* PLACE STRUCTURE */
place_struc_sub : place_struc_plac_sect /* 0:M */
                ;

place_struc_plac_sect : OPEN DELIM TAG_PLAC mand_line_item 
                        { START(PLAC, NULL) }
                        place_struc_plac_subs
			{ CHECK0 }
                        CLOSE { }
                      ;

place_struc_plac_subs : /* empty */
                      | place_struc_plac_subs place_struc_plac_sub
                      ;

place_struc_plac_sub : place_plac_form_sect  { OCCUR2(FORM, 0, 1) }
                     | source_cit_sub
                     | note_struc_sub
		     | no_std_sub
                     ;

place_plac_form_sect : OPEN DELIM TAG_FORM mand_line_item    
                       { START(FORM, NULL) } no_std_subs { CHECK0 } CLOSE { }
                     ;

/* SOURCE_CITATION */
source_cit_sub : source_cit_link_sect /* 0:M */
               | source_cit_emb_sect /* 0:M */
               ;

source_cit_link_sect : OPEN DELIM TAG_SOUR DELIM POINTER
                       { START(SOUR, NULL) }
                       source_cit_link_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

source_cit_link_subs : /* empty */
                     | source_cit_link_subs source_cit_link_sub
                     ;

source_cit_link_sub : source_cit_page_sect  { OCCUR2(PAGE, 0, 1) }
                    | source_cit_even_sect  { OCCUR2(EVEN, 0, 1) }
                    | source_cit_data_sect  { OCCUR2(DATA, 0, 1) }
                    | source_cit_quay_sect  { OCCUR2(QUAY, 0, 1) }
                    | multim_link_sub
                    | note_struc_sub
		    | no_std_sub
                    ;

source_cit_page_sect : OPEN DELIM TAG_PAGE mand_line_item    
                       { START(PAGE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                     ;

source_cit_even_sect : OPEN DELIM TAG_EVEN mand_line_item 
                       { START(EVEN, NULL) }
                       source_cit_even_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

source_cit_even_subs : /* empty */
                     | source_cit_even_subs source_cit_even_sub
                     ;

source_cit_even_sub  : source_cit_even_role_sect  { OCCUR2(ROLE, 0, 1) }
                     | no_std_sub
                     ;

source_cit_even_role_sect : OPEN DELIM TAG_ROLE mand_line_item    
                          { START(ROLE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                          ;

source_cit_data_sect : OPEN DELIM TAG_DATA
                       { START(DATA, NULL) }
                       source_cit_data_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

source_cit_data_subs : /* empty */
                     | source_cit_data_subs source_cit_data_sub
                     ;

source_cit_data_sub : source_cit_data_date_sect  { OCCUR2(DATE, 0, 1) }
                    | source_cit_text_sect  /* 0:M */
		    | no_std_sub
                    ;

source_cit_data_date_sect : OPEN DELIM TAG_DATE mand_line_item    
                            { START(DATE, NULL) } no_std_subs { CHECK0 } CLOSE { }
                          ;

source_cit_text_sect : OPEN DELIM TAG_TEXT mand_line_item 
                       { START(TEXT, NULL) }
                       source_cit_text_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

source_cit_text_subs : /* empty */
                     | source_cit_text_subs source_cit_text_sub
                     ;

source_cit_text_sub : continuation_sub
                    | no_std_sub
                    ;

source_cit_quay_sect : OPEN DELIM TAG_QUAY mand_line_item    
                       { START(QUAY, NULL) } no_std_subs { CHECK0 } CLOSE { }
                     ;

source_cit_emb_sect : OPEN DELIM TAG_SOUR mand_line_item
                      { START(SOUR, NULL) }
                      source_cit_emb_subs
		      { CHECK0 }
                      CLOSE { }
                    ;

source_cit_emb_subs : /* empty */
                    | source_cit_emb_subs source_cit_emb_sub
                    ;

source_cit_emb_sub : continuation_sub
                   | source_cit_text_sect  /* 0:M */
                   | note_struc_sub
                   | no_std_sub
                   ;

/* SOURCE REPOSITORY CITATION */
source_repos_cit_sub : source_repos_repo_sect  { OCCUR2(REPO, 0, 1) }
                     ;

source_repos_repo_sect : OPEN DELIM TAG_REPO mand_pointer
                         { START(REPO, NULL) }
                         source_repos_repo_subs
			 { CHECK0 }
                         CLOSE { }
                       ;

source_repos_repo_subs : /* empty */
                       | source_repos_repo_subs source_repos_repo_sub
                       ;

source_repos_repo_sub  : note_struc_sub
                       | caln_sect  /* 0:M */
                       | no_std_sub
                       ;

caln_sect : OPEN DELIM TAG_CALN mand_line_item 
            { START(CALN, NULL) }
            caln_subs
	    { CHECK0 }
            CLOSE { }
          ;

caln_subs : /* empty */
          | caln_subs caln_sub
          ;

caln_sub  : caln_medi_sect  { OCCUR2(MEDI, 0, 1) }
          | no_std_sub
          ;

caln_medi_sect : OPEN DELIM TAG_MEDI mand_line_item    
                 { START(MEDI, NULL) } no_std_subs { CHECK0 } CLOSE { }
               ;
 
/* SPOUSE TO FAMILY LINK */
spou_fam_link_sub : spou_fam_fams_sect  /* 0:M */
                  ;

spou_fam_fams_sect : OPEN DELIM TAG_FAMS mand_pointer
                     { START(FAMS, NULL) }
                     spou_fam_fams_subs
		     { CHECK0 }
                     CLOSE { }
                   ;

spou_fam_fams_subs : /* empty */
                   | spou_fam_fams_subs spou_fam_fams_sub
                   ;

spou_fam_fams_sub  : note_struc_sub
                   | no_std_sub
                   ;

/*********************************************************************/
/**** Special values                                              ****/
/*********************************************************************/

/*********************************************************************/
/**** General                                                     ****/
/*********************************************************************/

no_std_subs : /* empty */
            | no_std_subs no_std_sub
            ;

no_std_sub  : user_sect /* 0:M */
	    | gen_sect
	    | error error_subs CLOSE  { HANDLE_ERROR }
	    ;

no_std_rec  : user_rec /* 0:M */
	    | gen_rec
	    | error error_subs CLOSE  { HANDLE_ERROR }
	    ;

user_rec    : OPEN DELIM opt_xref USERTAG 
              { if ($4[0] != '_') {
		  gedcom_error(_("Undefined tag (and not a valid user tag): %s"),
			       $4);
		  YYERROR;
	        }
	      }
              opt_value
              { $<ctxt>$ = start_record(REC_USER, $1, $3, $4);
	        START($4, $<ctxt>$)
	      }
	      user_sects
              { CHECK0 }
	      CLOSE
              { end_record(REC_USER, $<ctxt>7); }
            ;
user_sect   : OPEN DELIM opt_xref USERTAG 
              { if ($4[0] != '_') {
		  gedcom_error(_("Undefined tag (and not a valid user tag): %s"),
			       $4);
		  YYERROR;
	        }
	      }
              opt_value
              { $<ctxt>$ = start_element(ELT_USER, PARENT, $1, $4, $6,  
					 GEDCOM_MAKE_STRING($6));
		START($4, $<ctxt>$);
	      }
	      user_sects
              { CHECK0 }
	      CLOSE
              { end_element(ELT_USER, PARENT, $<ctxt>7, NULL);
	      }
            ;

user_sects   : /* empty */     { }
            | user_sects user_sect { }
            ;

opt_xref    : /* empty */        { $$ = NULL; }
            | POINTER DELIM        { $$ = $1; }
            ;

opt_value   : /* empty */        { $$ = NULL; }
            | DELIM line_value        { $$ = $2; }
            ;

line_value  : POINTER        { $$ = $1; }
            | line_item        { $$ = $1; }
            ;

mand_pointer : /* empty */ { gedcom_error(_("Missing pointer")); YYERROR; }
             | DELIM POINTER { gedcom_debug_print("==Ptr: %s==", $2);
                               $$ = $2; }
             ;

mand_line_item : /* empty */ { gedcom_error(_("Missing value")); YYERROR; }
               | DELIM line_item { gedcom_debug_print("==Val: %s==", $2);
                                   $$ = $2; }
               ;

opt_line_item : /* empty */ { }
              | DELIM line_item { }
              ;

line_item   : anychar  { size_t i;
		         CLEAR_BUFFER(line_item_buf);
			 line_item_buf_ptr = line_item_buf;
			 /* The following also takes care of '@@' */
			 if (!strncmp($1, "@@", 3))
			   *line_item_buf_ptr++ = '@';
			 else
			   for (i=0; i < strlen($1); i++)
			     *line_item_buf_ptr++ = $1[i];
			 $$ = line_item_buf;
                       }
            | ESCAPE   { CLEAR_BUFFER(line_item_buf);
	                 line_item_buf_ptr = line_item_buf;
			 for (i=0; i < strlen($1); i++)
			   *line_item_buf_ptr++ = $1[i];
			 $$ = line_item_buf;
	               }
            | line_item anychar
                  { size_t i;
		    /* The following also takes care of '@@' */
		    if (!strncmp($2, "@@", 3))
		      *line_item_buf_ptr++ = '@';
		    else
		      for (i=0; i < strlen($2); i++)
			*line_item_buf_ptr++ = $2[i];
		    $$ = line_item_buf;
		  }
            | line_item ESCAPE
                  { for (i=0; i < strlen($2); i++)
		      *line_item_buf_ptr++ = $2[i];
		    $$ = line_item_buf;
		  }
            ;

anychar     : ANYCHAR        { }
            | DELIM        { }
            ;

error_subs  : /* empty */
            | error_subs error_sect
            ;

error_sect  : OPEN DELIM opt_xref anytag opt_value error_subs CLOSE { }

gen_sect    : OPEN DELIM opt_xref anystdtag
              { INVALID_TAG($4); }
              opt_value opt_sects CLOSE
              { }
            ;

gen_rec : gen_rec_top
        | gen_rec_norm
        ;

gen_rec_norm : OPEN DELIM opt_xref anystdtag
               { INVALID_TOP_TAG($4) }
               opt_value opt_sects CLOSE
               { }
             ;

gen_rec_top : OPEN DELIM anytoptag
              { gedcom_error(_("Missing cross-reference")); YYERROR; }
              opt_value opt_sects CLOSE
                { }
            ;

opt_sects   : /* empty */     { }
            | opt_sects gen_sect { }
            ;

anytag      : USERTAG { }
            | anystdtag { }
            ;

anytoptag   : TAG_FAM
            | TAG_INDI
            | TAG_OBJE
            | TAG_NOTE
            | TAG_REPO
            | TAG_SOUR
            | TAG_SUBN
            | TAG_SUBM
            ;

anystdtag   : TAG_ABBR
            | TAG_ADDR
            | TAG_ADR1
            | TAG_ADR2   { }
            | TAG_ADOP   { }
            | TAG_AFN   { }
            | TAG_AGE   { }
            | TAG_AGNC   { }
            | TAG_ALIA   { }
            | TAG_ANCE   { }
            | TAG_ANCI   { }
            | TAG_ANUL   { }
            | TAG_ASSO   { }
            | TAG_AUTH   { }
            | TAG_BAPL   { }
            | TAG_BAPM   { }
            | TAG_BARM   { }
            | TAG_BASM   { }
            | TAG_BIRT   { }
            | TAG_BLES   { }
            | TAG_BLOB   { }
            | TAG_BURI   { }
            | TAG_CALN   { }
            | TAG_CAST   { }
            | TAG_CAUS   { }
            | TAG_CENS   { }
            | TAG_CHAN   { }
            | TAG_CHAR   { }
            | TAG_CHIL   { }
            | TAG_CHR   { }
            | TAG_CHRA   { }
            | TAG_CITY   { }
            | TAG_CONC   { }
            | TAG_CONF   { }
            | TAG_CONL   { }
            | TAG_CONT   { }
            | TAG_COPR   { }
            | TAG_CORP   { }
            | TAG_CREM   { }
            | TAG_CTRY   { }
            | TAG_DATA   { }
            | TAG_DATE   { }
            | TAG_DEAT   { }
            | TAG_DESC   { }
            | TAG_DESI   { }
            | TAG_DEST   { }
            | TAG_DIV   { }
            | TAG_DIVF   { }
            | TAG_DSCR   { }
            | TAG_EDUC   { }
            | TAG_EMIG   { }
            | TAG_ENDL   { }
            | TAG_ENGA   { }
            | TAG_EVEN   { }
            | TAG_FAM    { }
            | TAG_FAMC   { }
            | TAG_FAMS   { }
            | TAG_FCOM   { }
            | TAG_FILE   { }
            | TAG_FORM   { }
            | TAG_GEDC   { }
            | TAG_GIVN   { }
            | TAG_GRAD   { }
            | TAG_HEAD   { }
            | TAG_HUSB   { }
            | TAG_IDNO   { }
            | TAG_IMMI   { }
            | TAG_INDI   { }
            | TAG_LANG   { }
            | TAG_LEGA   { }
            | TAG_MARB   { }
            | TAG_MARC   { }
            | TAG_MARL   { }
            | TAG_MARR   { }
            | TAG_MARS   { }
            | TAG_MEDI   { }
            | TAG_NAME   { }
            | TAG_NATI   { }
            | TAG_NCHI   { }
            | TAG_NICK   { }
            | TAG_NMR   { }
            | TAG_NOTE   { }
            | TAG_NPFX   { }
            | TAG_NSFX   { }
            | TAG_OBJE   { }
            | TAG_OCCU   { }
            | TAG_ORDI   { }
            | TAG_ORDN   { }
            | TAG_PAGE   { }
            | TAG_PEDI   { }
            | TAG_PHON   { }
            | TAG_PLAC   { }
            | TAG_POST   { }
            | TAG_PROB   { }
            | TAG_PROP   { }
            | TAG_PUBL   { }
            | TAG_QUAY   { }
            | TAG_REFN   { }
            | TAG_RELA   { }
            | TAG_RELI   { }
            | TAG_REPO   { }
            | TAG_RESI   { }
            | TAG_RESN   { }
            | TAG_RETI   { }
            | TAG_RFN   { }
            | TAG_RIN   { }
            | TAG_ROLE   { }
            | TAG_SEX   { }
            | TAG_SLGC   { }
            | TAG_SLGS   { }
            | TAG_SOUR   { }
            | TAG_SPFX   { }
            | TAG_SSN   { }
            | TAG_STAE   { }
            | TAG_STAT   { }
            | TAG_SUBM   { }
            | TAG_SUBN   { }
            | TAG_SURN   { }
            | TAG_TEMP   { }
            | TAG_TEXT   { }
            | TAG_TIME   { }
            | TAG_TITL   { }
            | TAG_TRLR   { }
            | TAG_TYPE   { }
            | TAG_VERS   { }
            | TAG_WIFE   { }
            | TAG_WILL   { }

%%

/* Functions that handle the counting of subtags */

int* count_arrays[MAXGEDCLEVEL+1];
char tag_stack[MAXGEDCLEVEL+1][MAXSTDTAGLEN+1];
Gedcom_ctxt ctxt_stack[MAXGEDCLEVEL+1];

void push_countarray()
{
  int *count = NULL;
  if (count_level > MAXGEDCLEVEL) {
    gedcom_error(_("Internal error: count array overflow"));
    exit(1);
  }
  else {
    count = (int *)calloc(YYNTOKENS, sizeof(int));
    if (count == NULL) {
      gedcom_error(_("Internal error: count array calloc error"));
      exit(1);
    }
    else {
      count_arrays[count_level] = count;
    }
  }
}

void set_parenttag(char* tag)
{
  strncpy(tag_stack[count_level], tag, MAXSTDTAGLEN+1);
}

void set_parentctxt(Gedcom_ctxt ctxt)
{
  ctxt_stack[count_level] = ctxt;
}

char* get_parenttag()
{
  return tag_stack[count_level];
}

Gedcom_ctxt get_parentctxt()
{
  return ctxt_stack[count_level];
}

int count_tag(int tag)
{
  int *count = count_arrays[count_level];
  return ++count[tag - GEDCOMTAGOFFSET];
}

int check_occurrence(int tag)
{
  int *count = count_arrays[count_level];
  return (count[tag - GEDCOMTAGOFFSET] > 0);
}

void pop_countarray()
{
  int *count;
  if (count_level < 0) {
    gedcom_error(_("Internal error: count array underflow"));
    exit(1);
  }
  else {
    count = count_arrays[count_level];
    free(count);
    count_arrays[count_level] = NULL;
  }
}

/* Enabling debug mode */
/* level 0: no debugging */
/* level 1: only internal */
/* level 2: also bison */
FILE* trace_output;

void gedcom_set_debug_level(int level, FILE* f)
{
  if (f != NULL)
    trace_output = f;
  else
    trace_output = stderr;
  if (level > 0) {
    gedcom_high_level_debug = 1;
  }
  if (level > 1) {
#if YYDEBUG != 0
    gedcom_debug = 1;
#endif
  }
}

int gedcom_debug_print(char* s, ...)
{
  int res = 0;
  if (gedcom_high_level_debug) {
    va_list ap;
    va_start(ap, s);
    res = vfprintf(trace_output, s, ap);
    va_end(ap);
    fprintf(trace_output, "\n");
  }
  return(res);
}

/* Setting the error mechanism */
void gedcom_set_error_handling(Gedcom_err_mech mechanism)
{
  error_mechanism = mechanism;
}

/* Compatibility handling */

void gedcom_set_compat_handling(int enable_compat)
{
  compat_enabled = enable_compat;
}

void set_compatibility(char* program)
{
  if (compat_enabled) {
    if (! strncmp(program, "ftree", 6)) {
      gedcom_warning(_("Enabling compatibility with 'ftree'"));
      compatibility = C_FTREE;
    }
    else {
      compatibility = 0;
    }
  }
}

int compat_mode(int compat_flags)
{
  return (compat_flags & compatibility);
}

