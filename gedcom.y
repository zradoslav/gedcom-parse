/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

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

   - The syntax analysis doesn't handle the contents of the line values
     or their encoding; this is done in the semantic analysis.

 */

%{
#include "gedcom.h"
#include "multilex.h"
#include "encoding.h"

int  count_level    = 0;
int  fail           = 0;
int  compat_enabled = 1;
int  gedcom_high_level_debug = 0; 
int  compatibility  = 0; 
MECHANISM error_mechanism=IMMED_FAIL;
char string_buf[MAXGEDCLINELEN*4+1];
char *string_buf_ptr;

enum _COMPAT {
  C_FTREE = 0x01
};

/* These are defined at the bottom of the file */ 
void push_countarray();
void set_parenttag(char* tag);
char* get_parenttag(); 
void pop_countarray();
int  count_tag(int tag);
int  check_occurrence(int tag);
void set_compatibility(char* program);
int  compat_mode(int flags); 

#define CLEAR_BUFFER(BUF) { memset(BUF, 0, sizeof(BUF)); } 
 
#define HANDLE_ERROR \
     { \
       if (error_mechanism == IMMED_FAIL) { \
	 YYABORT; \
       } \
       else if (error_mechanism == DEFER_FAIL) { \
         yyerrok; fail = 1; \
       } \
       else if (error_mechanism == IGNORE_ERRORS) { \
	 yyerrok; \
       } \
     }
#define START(PARENTTAG) \
     { ++count_level; \
       set_parenttag(#PARENTTAG); \
       push_countarray(); \
     }
#define CHK(TAG) \
     { if (!check_occurrence(TAG_##TAG)) { \
         char* parenttag = get_parenttag(); \
         gedcom_error("The tag '%s' is mandatory within '%s', but missing", \
		      #TAG, parenttag); \
         HANDLE_ERROR; \
       } \
     }
#define POP \
     { pop_countarray(); \
       --count_level; \
     }
#define CHECK0 POP; 
#define CHECK1(TAG1) { CHK(TAG1); POP; }
#define CHECK2(TAG1,TAG2) \
     { CHK(TAG1); CHK(TAG2); POP; }
#define CHECK3(TAG1,TAG2,TAG3) \
     { CHK(TAG1); CHK(TAG2); CHK(TAG3); POP; }
#define CHECK4(TAG1,TAG2,TAG3,TAG4) \
     { CHK(TAG1); CHK(TAG2); CHK(TAG3); CHK(TAG4); POP; } 
#define OCCUR1(CHILDTAG, MIN) { count_tag(TAG_##CHILDTAG); } 
#define OCCUR2(CHILDTAG, MIN, MAX) \
     { int num = count_tag(TAG_##CHILDTAG); \
       if (num > MAX) { \
         char* parenttag = get_parenttag(); \
         gedcom_error("The tag '%s' can maximally occur %d " \
		      "time(s) within '%s'", \
		      #CHILDTAG, MAX, parenttag); \
         HANDLE_ERROR; \
       } \
     }
#define INVALID_TAG(CHILDTAG) \
     { char* parenttag = get_parenttag(); \
       gedcom_error("The tag '%s' is not a valid tag within '%s'", \
		    CHILDTAG, parenttag); \
       HANDLE_ERROR; \
     }
#define INVALID_TOP_TAG(CHILDTAG) \
     { gedcom_error("The tag '%s' is not a valid top-level tag", \
		    CHILDTAG); \
       HANDLE_ERROR; \
     }

%}

%union {
  int  level;
  char *pointer;
  char *tag;
  char *string;
}

%token_table
%expect 300

%token <string> BADTOKEN
%token <level> OPEN
%token <string> CLOSE
%token <string> ESCAPE
%token <string> DELIM
%token <string> ANYCHAR
%token <pointer> POINTER
%token <tag> USERTAG
%token <tag> TAG_ABBR
%token <tag> TAG_ADDR
%token <tag> TAG_ADR1
%token <tag> TAG_ADR2
%token <tag> TAG_ADOP
%token <tag> TAG_AFN
%token <tag> TAG_AGE
%token <tag> TAG_AGNC
%token <tag> TAG_ALIA
%token <tag> TAG_ANCE
%token <tag> TAG_ANCI
%token <tag> TAG_ANUL
%token <tag> TAG_ASSO
%token <tag> TAG_AUTH
%token <tag> TAG_BAPL
%token <tag> TAG_BAPM
%token <tag> TAG_BARM
%token <tag> TAG_BASM
%token <tag> TAG_BIRT
%token <tag> TAG_BLES
%token <tag> TAG_BLOB
%token <tag> TAG_BURI
%token <tag> TAG_CALN
%token <tag> TAG_CAST
%token <tag> TAG_CAUS
%token <tag> TAG_CENS
%token <tag> TAG_CHAN
%token <tag> TAG_CHAR
%token <tag> TAG_CHIL
%token <tag> TAG_CHR
%token <tag> TAG_CHRA
%token <tag> TAG_CITY
%token <tag> TAG_CONC
%token <tag> TAG_CONF
%token <tag> TAG_CONL
%token <tag> TAG_CONT
%token <tag> TAG_COPR
%token <tag> TAG_CORP
%token <tag> TAG_CREM
%token <tag> TAG_CTRY
%token <tag> TAG_DATA
%token <tag> TAG_DATE
%token <tag> TAG_DEAT
%token <tag> TAG_DESC
%token <tag> TAG_DESI
%token <tag> TAG_DEST
%token <tag> TAG_DIV
%token <tag> TAG_DIVF
%token <tag> TAG_DSCR
%token <tag> TAG_EDUC
%token <tag> TAG_EMIG
%token <tag> TAG_ENDL
%token <tag> TAG_ENGA
%token <tag> TAG_EVEN
%token <tag> TAG_FAM
%token <tag> TAG_FAMC
%token <tag> TAG_FAMF
%token <tag> TAG_FAMS
%token <tag> TAG_FCOM
%token <tag> TAG_FILE
%token <tag> TAG_FORM
%token <tag> TAG_GEDC
%token <tag> TAG_GIVN
%token <tag> TAG_GRAD
%token <tag> TAG_HEAD
%token <tag> TAG_HUSB
%token <tag> TAG_IDNO
%token <tag> TAG_IMMI
%token <tag> TAG_INDI
%token <tag> TAG_LANG
%token <tag> TAG_LEGA
%token <tag> TAG_MARB
%token <tag> TAG_MARC
%token <tag> TAG_MARL
%token <tag> TAG_MARR
%token <tag> TAG_MARS
%token <tag> TAG_MEDI
%token <tag> TAG_NAME
%token <tag> TAG_NATI
%token <tag> TAG_NATU
%token <tag> TAG_NCHI
%token <tag> TAG_NICK
%token <tag> TAG_NMR
%token <tag> TAG_NOTE
%token <tag> TAG_NPFX
%token <tag> TAG_NSFX
%token <tag> TAG_OBJE
%token <tag> TAG_OCCU
%token <tag> TAG_ORDI
%token <tag> TAG_ORDN
%token <tag> TAG_PAGE
%token <tag> TAG_PEDI
%token <tag> TAG_PHON
%token <tag> TAG_PLAC
%token <tag> TAG_POST
%token <tag> TAG_PROB
%token <tag> TAG_PROP
%token <tag> TAG_PUBL
%token <tag> TAG_QUAY
%token <tag> TAG_REFN
%token <tag> TAG_RELA
%token <tag> TAG_RELI
%token <tag> TAG_REPO
%token <tag> TAG_RESI
%token <tag> TAG_RESN
%token <tag> TAG_RETI
%token <tag> TAG_RFN
%token <tag> TAG_RIN
%token <tag> TAG_ROLE
%token <tag> TAG_SEX
%token <tag> TAG_SLGC
%token <tag> TAG_SLGS
%token <tag> TAG_SOUR
%token <tag> TAG_SPFX
%token <tag> TAG_SSN
%token <tag> TAG_STAE
%token <tag> TAG_STAT
%token <tag> TAG_SUBM
%token <tag> TAG_SUBN
%token <tag> TAG_SURN
%token <tag> TAG_TEMP
%token <tag> TAG_TEXT
%token <tag> TAG_TIME
%token <tag> TAG_TITL
%token <tag> TAG_TRLR
%token <tag> TAG_TYPE
%token <tag> TAG_VERS
%token <tag> TAG_WIFE
%token <tag> TAG_WILL

%type <tag> anystdtag
%type <tag> anytoptag
%type <string> line_item
%type <string> mand_line_item
%type <string> note_line_item
%type <string> anychar

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
               { START(HEAD) }
               head_subs
               { if (compat_mode(C_FTREE))
		   CHECK3(SOUR, GEDC, CHAR)
	         else
		   CHECK4(SOUR, SUBM, GEDC, CHAR)
	       }
               CLOSE { }
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
		   gedcom_debug_print("===Source: '%s'\n", $4);
		   START(SOUR)
		 }
                 head_sour_subs
                 { CHECK0 }
		 CLOSE
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
                      { START(VERS)} no_std_subs { CHECK0 } CLOSE
                      { gedcom_debug_print("===Source version: '%s'\n", $4);
		      }
                    ;
head_sour_name_sect : OPEN DELIM TAG_NAME mand_line_item
                      { START(NAME) } no_std_subs { CHECK0 } CLOSE
                      { gedcom_debug_print("===Source name: '%s'\n", $4);
		      }
                    ;
head_sour_corp_sect : OPEN DELIM TAG_CORP mand_line_item 
                      { gedcom_debug_print("===Source corp name: '%s'\n", $4);
			START(CORP) }
                      head_sour_corp_subs
		      { CHECK0 }
                      CLOSE
                            { }
                    ;

head_sour_corp_subs : /* empty */
                    | head_sour_corp_subs head_sour_corp_sub
                    ;

head_sour_corp_sub : addr_struc_sub  /* 0:1 */
                   | no_std_sub
                   ;

head_sour_data_sect : OPEN DELIM TAG_DATA mand_line_item 
                      { START(DATA) }
                      head_sour_data_subs
                      { CHECK0 }
		      CLOSE
                            { }
                    ;

head_sour_data_subs : /* empty */
                    | head_sour_data_subs head_sour_data_sub
                    ;

head_sour_data_sub : head_sour_data_date_sect  { OCCUR2(DATE, 0, 1) }
                   | head_sour_data_copr_sect  { OCCUR2(COPR, 0, 1) }
                   | no_std_sub
                   ;

head_sour_data_date_sect : OPEN DELIM TAG_DATE mand_line_item
                           { START(DATE) } no_std_subs { CHECK0 } CLOSE
                                { }
                         ;
head_sour_data_copr_sect : OPEN DELIM TAG_COPR mand_line_item
                           { START(COPR) } no_std_subs { CHECK0 } CLOSE
                                { }
                         ;

/* HEAD.DEST */
head_dest_sect : OPEN DELIM TAG_DEST mand_line_item
                 { START(DEST) } no_std_subs { CHECK0 } CLOSE
                       { }
               ;

/* HEAD.DATE */
head_date_sect : OPEN DELIM TAG_DATE mand_line_item 
                 { START(DATE) }
                 head_date_subs
		 { CHECK0 }
                 CLOSE
                       { }
               ;

head_date_subs : /* empty */
               | head_date_subs head_date_sub
               ;

head_date_sub  : head_date_time_sect  { OCCUR2(TIME, 0, 1) }
               | no_std_sub
               ;

head_date_time_sect : OPEN DELIM TAG_TIME mand_line_item
                      { START(TIME) } no_std_subs { CHECK0 } CLOSE
                          { }
                    ;

/* HEAD.SUBM */
head_subm_sect : OPEN DELIM TAG_SUBM mand_pointer
                 { START(SUBM) } no_std_subs { CHECK0 } CLOSE
                       { }
               ;
/* HEAD.SUBN */
head_subn_sect : OPEN DELIM TAG_SUBN mand_pointer 
                 { START(SUBN) } no_std_subs { CHECK0 } CLOSE
                       { }
               ;
/* HEAD.FILE */
head_file_sect : OPEN DELIM TAG_FILE mand_line_item 
                 { START(FILE) } no_std_subs { CHECK0 } CLOSE
                       { }
               ;
/* HEAD.COPR */
head_copr_sect : OPEN DELIM TAG_COPR mand_line_item 
                 { START(COPR) } no_std_subs { CHECK0 } CLOSE
                       { }
               ;
/* HEAD.GEDC */
head_gedc_sect : OPEN DELIM TAG_GEDC
                 { START(GEDC) }
                 head_gedc_subs
		 { CHECK2(VERS, FORM) }
                 CLOSE
                       { }
               ;

head_gedc_subs : /* empty */
               | head_gedc_subs head_gedc_sub
               ;

head_gedc_sub  : head_gedc_vers_sect  { OCCUR2(VERS, 1, 1) }
               | head_gedc_form_sect  { OCCUR2(FORM, 1, 1) }
               | no_std_sub
               ;
head_gedc_vers_sect : OPEN DELIM TAG_VERS mand_line_item  
                      { START(VERS) } no_std_subs { CHECK0 } CLOSE
                          { }
                    ;
head_gedc_form_sect : OPEN DELIM TAG_FORM mand_line_item   
                      { START(FORM) } no_std_subs { CHECK0 } CLOSE
                          { }
                    ;

/* HEAD.CHAR */
head_char_sect : OPEN DELIM TAG_CHAR mand_line_item 
                 { if (open_conv_to_internal($4) == 0) YYERROR;
		   START(CHAR) }
                 head_char_subs
		 { CHECK0 }
                 CLOSE
                       { }
               ;

head_char_subs : /* empty */
               | head_char_subs head_char_sub
               ;

head_char_sub  : head_char_vers_sect  { OCCUR2(VERS, 0, 1) }
               | no_std_sub
               ;
head_char_vers_sect : OPEN DELIM TAG_VERS mand_line_item   
                      { START(VERS) } no_std_subs { CHECK0 } CLOSE
                          { }
                    ;

/* HEAD.LANG */
head_lang_sect : OPEN DELIM TAG_LANG mand_line_item   
                 { START(LANG) } no_std_subs { CHECK0 } CLOSE
                       { }
               ;
/* HEAD.PLAC */
head_plac_sect : OPEN DELIM TAG_PLAC
                 { START(PLAC) }
                 head_plac_subs
		 { CHECK1(FORM) }
                 CLOSE
                       { }
               ;

head_plac_subs : /* empty */
               | head_plac_subs head_plac_sub
               ;

head_plac_sub  : head_plac_form_sect  { OCCUR2(FORM, 1, 1) }
               | no_std_sub
               ;
head_plac_form_sect : OPEN DELIM TAG_FORM mand_line_item   
                      { START(FORM) } no_std_subs { CHECK0 } CLOSE
                          { }
                    ;

/* HEAD.NOTE */
head_note_sect : OPEN DELIM TAG_NOTE mand_line_item 
                 { START(NOTE) }
                 head_note_subs
		 { CHECK0 }
                 CLOSE
                       { }
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
trlr_sect   : OPEN DELIM TAG_TRLR CLOSE { }
            ;

/*********************************************************************/
/**** Family record                                               ****/
/*********************************************************************/
fam_rec      : OPEN DELIM POINTER DELIM TAG_FAM
               { START(FAM) }
               fam_subs
	       { CHECK0 }
               CLOSE { }
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
                { START(HUSB) } no_std_subs { CHECK0 } CLOSE
                       { }
              ;

/* FAM.WIFE */
fam_wife_sect : OPEN DELIM TAG_WIFE mand_pointer 
                { START(WIFE) } no_std_subs { CHECK0 } CLOSE
                       { }
              ;

/* FAM.CHIL */
fam_chil_sect : OPEN DELIM TAG_CHIL mand_pointer
                { START(CHIL) } no_std_subs { CHECK0 } CLOSE
                       { }
              ;

/* FAM.NCHI */
fam_nchi_sect : OPEN DELIM TAG_NCHI mand_line_item    
                { START(NCHI) } no_std_subs { CHECK0 } CLOSE
                       { }
              ;

/* FAM.SUBM */
fam_subm_sect : OPEN DELIM TAG_SUBM mand_pointer
                { START(SUBM) } no_std_subs { CHECK0 } CLOSE
                       { }
              ;

/*********************************************************************/
/**** Individual record                                           ****/
/*********************************************************************/
indiv_rec   : OPEN DELIM POINTER DELIM TAG_INDI
              { START(INDI) }
              indi_subs
	      { CHECK0 }
              CLOSE { }
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
                 { START(RESN) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* INDI.SEX */
indi_sex_sect  : OPEN DELIM TAG_SEX mand_line_item     
                 { START(SEX) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* INDI.SUBM */
indi_subm_sect : OPEN DELIM TAG_SUBM mand_pointer 
                 { START(SUBM) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* INDI.ALIA */
indi_alia_sect : OPEN DELIM TAG_ALIA mand_pointer
                 { START(ALIA) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* INDI.ANCI */
indi_anci_sect : OPEN DELIM TAG_ANCI mand_pointer
                 { START(ANCI) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* INDI.DESI */
indi_desi_sect : OPEN DELIM TAG_DESI mand_pointer
                 { START(DESI) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* INDI.RFN */
indi_rfn_sect  : OPEN DELIM TAG_RFN mand_line_item     
                 { START(RFN) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* INDI.AFN */
indi_afn_sect  : OPEN DELIM TAG_AFN mand_line_item      
                 { START(AFN) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* INDI.ADDR (Only for 'ftree' compatibility) */
ftree_addr_sect : OPEN DELIM TAG_ADDR opt_line_item
                  { START(ADDR) } no_std_subs { CHECK0 } CLOSE { }

/*********************************************************************/
/**** Multimedia record                                           ****/
/*********************************************************************/
multim_rec  : OPEN DELIM POINTER DELIM TAG_OBJE
              { START(OBJE) }
              obje_subs
	      { CHECK2(FORM, BLOB) }
              CLOSE { }
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
                 { START(FORM) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* OBJE.TITL */
obje_titl_sect : OPEN DELIM TAG_TITL mand_line_item       
                 { START(TITL) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* OBJE.BLOB */
obje_blob_sect : OPEN DELIM TAG_BLOB
                 { START(BLOB) }
                 obje_blob_subs
		 { CHECK1(CONT) }
                 CLOSE { }
               ;

obje_blob_subs : /* empty */
               | obje_blob_subs obje_blob_sub
               ;

obje_blob_sub  : obje_blob_cont_sect  { OCCUR1(CONT, 1) }
               | no_std_sub
               ;

obje_blob_cont_sect : OPEN DELIM TAG_CONT mand_line_item        
                      { START(CONT) } no_std_subs { CHECK0 } CLOSE { }
                    ;

/* OBJE.OBJE */
obje_obje_sect : OPEN DELIM TAG_OBJE mand_pointer 
                 { START(OBJE) } no_std_subs { CHECK0 } CLOSE { }
               ;

/*********************************************************************/
/**** Note record                                                 ****/
/*********************************************************************/
note_rec    : OPEN DELIM POINTER DELIM TAG_NOTE note_line_item
              { START(NOTE) }
              note_subs
	      { CHECK0 }
              CLOSE { }
            ;

note_line_item : /* empty */
                   { if (!compat_mode(C_FTREE)) {
		       gedcom_error("Missing value"); YYERROR;
		     }
		   }
               | DELIM line_item
                   { gedcom_debug_print("==Val: %s==\n", $2);
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
              { START(REPO) }
              repo_subs
	      { CHECK0 }
              CLOSE { }
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
                 { START(NAME) } no_std_subs { CHECK0 } CLOSE {}
               ;

/*********************************************************************/
/**** Source record                                               ****/
/*********************************************************************/
source_rec  : OPEN DELIM POINTER DELIM TAG_SOUR
              { START(SOUR) }
              sour_subs
	      { CHECK0 }
              CLOSE { }
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
                 { START(DATA) }
                 sour_data_subs
		 { CHECK0 }
                 CLOSE { }
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
                      { START(EVEN) }
                      sour_data_even_subs
		      { CHECK0 }
                      CLOSE { }
                    ;

sour_data_even_subs : /* empty */
                    | sour_data_even_subs sour_data_even_sub
                    ;

sour_data_even_sub  : sour_data_even_date_sect { OCCUR2(DATE, 0, 1) }
                    | sour_data_even_plac_sect { OCCUR2(PLAC, 0, 1) }
                    | no_std_sub
                    ;

sour_data_even_date_sect : OPEN DELIM TAG_DATE mand_line_item          
                           { START(DATE) } no_std_subs { CHECK0 } CLOSE { }
                         ;

sour_data_even_plac_sect : OPEN DELIM TAG_PLAC mand_line_item          
                           { START(PLAC) } no_std_subs { CHECK0 } CLOSE { }
                         ;

sour_data_agnc_sect : OPEN DELIM TAG_AGNC mand_line_item          
                      { START(AGNC) } no_std_subs { CHECK0 } CLOSE { }
                    ;

/* SOUR.AUTH */
sour_auth_sect : OPEN DELIM TAG_AUTH mand_line_item
                 { START(AUTH) }
                 sour_auth_subs
		 { CHECK0 }
                 CLOSE { }
               ;

sour_auth_subs : /* empty */
               | sour_auth_subs sour_auth_sub
               ;

sour_auth_sub  : continuation_sub  /* 0:M */
               | no_std_sub
               ;

/* SOUR.TITL */
sour_titl_sect : OPEN DELIM TAG_TITL mand_line_item  
                 { START(TITL) }
                 sour_titl_subs 
		 { CHECK0 }
                 CLOSE { }
               ;

sour_titl_subs : /* empty */
               | sour_titl_subs sour_titl_sub
               ;

sour_titl_sub  : continuation_sub  /* 0:M */
               | no_std_sub
               ;

/* SOUR.ABBR */
sour_abbr_sect : OPEN DELIM TAG_ABBR mand_line_item           
                 { START(ABBR) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* SOUR.PUBL */
sour_publ_sect : OPEN DELIM TAG_PUBL mand_line_item  
                 { START(PUBL) }
                 sour_publ_subs  
		 { CHECK0 }
                 CLOSE { }
               ;

sour_publ_subs : /* empty */
               | sour_publ_subs sour_publ_sub
               ;

sour_publ_sub  : continuation_sub  /* 0:M */
               | no_std_sub
               ;

/* SOUR.TEXT */
sour_text_sect : OPEN DELIM TAG_TEXT mand_line_item   
                 { START(TEXT) }
                 sour_text_subs  
		 { CHECK0 }
                 CLOSE { }
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
              { START(SUBN) }
              subn_subs
	      { CHECK0 }
              CLOSE { }
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
                 { START(SUBM) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* SUBN.FAMF */
subn_famf_sect : OPEN DELIM TAG_FAMF mand_line_item            
                 { START(FAMF) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* SUBN.TEMP */
subn_temp_sect : OPEN DELIM TAG_TEMP mand_line_item            
                 { START(TEMP) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* SUBN.ANCE */
subn_ance_sect : OPEN DELIM TAG_ANCE mand_line_item            
                 { START(ANCE) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* SUBN.DESC */
subn_desc_sect : OPEN DELIM TAG_DESC mand_line_item            
                 { START(DESC) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* SUBN.ORDI */
subn_ordi_sect : OPEN DELIM TAG_ORDI mand_line_item            
                 { START(ORDI) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* SUBN.RIN */
subn_rin_sect  : OPEN DELIM TAG_RIN mand_line_item            
                 { START(RIN) } no_std_subs { CHECK0 } CLOSE { }
               ;

/*********************************************************************/
/**** Submitter record                                            ****/
/*********************************************************************/
submit_rec : OPEN DELIM POINTER DELIM TAG_SUBM    
             { START(SUBM) }
             subm_subs
	     { CHECK1(NAME) }
             CLOSE { }
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
                 { START(NAME) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* SUBM.LANG */
subm_lang_sect : OPEN DELIM TAG_LANG mand_line_item             
                 { START(LANG) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* SUBM.RFN */
subm_rfn_sect  : OPEN DELIM TAG_RFN mand_line_item             
                 { START(RFN) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* SUBM.RIN */
subm_rin_sect  : OPEN DELIM TAG_RIN mand_line_item             
                 { START(RIN) } no_std_subs { CHECK0 } CLOSE { }
               ;

/*********************************************************************/
/**** Substructures                                               ****/
/*********************************************************************/

/* ADDRESS STRUCTURE */
addr_struc_sub : addr_sect { OCCUR2(ADDR, 0, 1) }
               | phon_sect { OCCUR2(PHON, 0, 3) }
               ;

addr_sect   : OPEN DELIM TAG_ADDR mand_line_item 
              { START(ADDR) }
              addr_subs
	      { CHECK0 }
              CLOSE { }
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
                 { START(CONT) } no_std_subs { CHECK0 } CLOSE { }
               ;
addr_adr1_sect : OPEN DELIM TAG_ADR1 mand_line_item              
                 { START(ADR1) } no_std_subs { CHECK0 } CLOSE { }
               ;
addr_adr2_sect : OPEN DELIM TAG_ADR2 mand_line_item              
                 { START(ADR2) } no_std_subs { CHECK0 } CLOSE { }
               ;
addr_city_sect : OPEN DELIM TAG_CITY mand_line_item              
                 { START(CITY) } no_std_subs { CHECK0 } CLOSE { }
               ;
addr_stae_sect : OPEN DELIM TAG_STAE mand_line_item              
                 { START(STAE) } no_std_subs { CHECK0 } CLOSE { }
               ;
addr_post_sect : OPEN DELIM TAG_POST mand_line_item              
                 { START(POST) } no_std_subs { CHECK0 } CLOSE { }
               ;
addr_ctry_sect : OPEN DELIM TAG_CTRY mand_line_item              
                 { START(CTRY) } no_std_subs { CHECK0 } CLOSE { }
               ;

phon_sect   : OPEN DELIM TAG_PHON mand_line_item              
              { START(PHON) } no_std_subs { CHECK0 } CLOSE { }
            ;

/* ASSOCIATION STRUCTURE */
assoc_struc_sub : asso_sect /* 0:M */
                ;

asso_sect : OPEN DELIM TAG_ASSO mand_pointer
            { START(ASSO) }
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
                 { START(TYPE) } no_std_subs { CHECK0 } CLOSE { }
               ;

asso_rela_sect : OPEN DELIM TAG_RELA mand_line_item               
                 { START(RELA) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* CHANGE DATE */
change_date_sub : change_date_chan_sect  { OCCUR2(CHAN, 0, 1) }
                ;

change_date_chan_sect : OPEN DELIM TAG_CHAN
                        { START(CHAN) }
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
                        { START(DATE) }
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
                             { START(TIME) } no_std_subs { CHECK0 } CLOSE { }
                           ;

/* CHILD TO FAMILY LINK */
chi_fam_link_sub : famc_sect  /* 0:M */
                 ;

famc_sect : OPEN DELIM TAG_FAMC mand_pointer
            { START(FAMC) }
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
                 { START(PEDI) } no_std_subs { CHECK0 } CLOSE { }
               ;

/* CONTINUATION SUBSECTIONS */
continuation_sub : cont_sect  /* 0:M */
                 | conc_sect  /* 0:M */
                 ;

cont_sect : OPEN DELIM TAG_CONT mand_line_item 
            { START(CONT) } no_std_subs { CHECK0 } CLOSE { }
          ;

conc_sect : OPEN DELIM TAG_CONC mand_line_item 
            { START(CONC) } no_std_subs { CHECK0 } CLOSE { }
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
                         { START(TYPE) } no_std_subs { CHECK0 } CLOSE { }
                       ;
event_detail_date_sect : OPEN DELIM TAG_DATE mand_line_item 
                         { START(DATE) } no_std_subs { CHECK0 } CLOSE { }
                       ;
event_detail_age_sect  : OPEN DELIM TAG_AGE mand_line_item 
                         { START(AGE) } no_std_subs { CHECK0 } CLOSE { }
                       ;
event_detail_agnc_sect : OPEN DELIM TAG_AGNC mand_line_item 
                         { START(AGNC) } no_std_subs { CHECK0 } CLOSE { }
                       ;
event_detail_caus_sect : OPEN DELIM TAG_CAUS mand_line_item 
                         { START(CAUS) } no_std_subs { CHECK0 } CLOSE { }
                       ;

/* FAMILY EVENT STRUCTURE */
fam_event_struc_sub : fam_event_sect
                    | fam_gen_even_sect  /* 0:M */
                    ;

fam_event_sect : OPEN DELIM fam_event_tag opt_value fam_event_subs
                 { CHECK0 }
                 CLOSE { }
               ;

fam_event_tag : TAG_ANUL { START(ANUL) }
              | TAG_CENS { START(CENS) }
              | TAG_DIV { START(DIV) }
              | TAG_DIVF { START(DIVF) }
              | TAG_ENGA { START(ENGA) }
              | TAG_MARR { START(MARR) }
              | TAG_MARB { START(MARB) }
              | TAG_MARC { START(MARC) }
              | TAG_MARL { START(MARL) }
              | TAG_MARS { START(MARS) }
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
                     { START(HUSB) }
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
                         { START(AGE) } no_std_subs { CHECK0 } CLOSE { }
                       ;

fam_even_wife_sect : OPEN DELIM TAG_WIFE
                     { START(HUSB) }
                     fam_even_husb_subs
		     { CHECK1(AGE) }
                     CLOSE { }
                   ;

fam_gen_even_sect : OPEN DELIM TAG_EVEN
                    { START(EVEN) }
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
                  { START(REFN) }
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
                       { START(TYPE) } no_std_subs { CHECK0 } CLOSE { }
                     ;

ident_rin_sect  : OPEN DELIM TAG_RIN mand_line_item   
                  { START(RIN) } no_std_subs { CHECK0 } CLOSE { }
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
                  { START(CAST) }
                  indiv_attr_event_subs
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_dscr_sect : OPEN DELIM TAG_DSCR mand_line_item 
                  { START(DSCR) }
                  indiv_attr_event_subs
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_educ_sect : OPEN DELIM TAG_EDUC mand_line_item  
                  { START(EDUC) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_idno_sect : OPEN DELIM TAG_IDNO mand_line_item 
                  { START(IDNO) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_nati_sect : OPEN DELIM TAG_NATI mand_line_item 
                  { START(NATI) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_nchi_sect : OPEN DELIM TAG_NCHI mand_line_item 
                  { START(NCHI) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_nmr_sect  : OPEN DELIM TAG_NMR mand_line_item 
                  { START(NMR) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_occu_sect : OPEN DELIM TAG_OCCU mand_line_item 
                  { START(OCCU) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_prop_sect : OPEN DELIM TAG_PROP mand_line_item 
                  { START(PROP) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_reli_sect : OPEN DELIM TAG_RELI mand_line_item 
                  { START(RELI) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_resi_sect : OPEN DELIM TAG_RESI 
                  { START(RESI) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_ssn_sect  : OPEN DELIM TAG_SSN mand_line_item 
                  { START(SSN) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_titl_sect : OPEN DELIM TAG_TITL mand_line_item 
                  { START(TITL) }
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

indiv_birt_tag  : TAG_BIRT { START(BIRT) }
                | TAG_CHR { START(CHR) }
                ;

indiv_birt_subs : /* empty */
                | indiv_birt_subs indiv_birt_sub
                ;

indiv_birt_sub  : event_detail_sub
                | indiv_birt_famc_sect  { OCCUR2(FAMC,0, 1) }
                | no_std_sub
                ;

indiv_birt_famc_sect : OPEN DELIM TAG_FAMC mand_pointer
                       { START(FAMC) } no_std_subs { CHECK0 } CLOSE { }
                     ;

indiv_gen_sect  : OPEN DELIM indiv_gen_tag opt_value indiv_gen_subs
                  { CHECK0 }
                  CLOSE { }
                ;

indiv_gen_tag   : TAG_DEAT { START(DEAT) }
                | TAG_BURI { START(BURI) }
                | TAG_CREM { START(CREM) }
                | TAG_BAPM { START(BAPM) }
                | TAG_BARM { START(BARM) }
                | TAG_BASM { START(BASM) }
                | TAG_BLES { START(BLES) }
                | TAG_CHRA { START(CHRA) }
                | TAG_CONF { START(CONF) }
                | TAG_FCOM { START(FCOM) }
                | TAG_ORDN { START(ORDN) }
                | TAG_NATU { START(NATU) }
                | TAG_EMIG { START(EMIG) }
                | TAG_IMMI { START(IMMI) }
                | TAG_CENS { START(CENS) }
                | TAG_PROB { START(PROB) }
                | TAG_WILL { START(WILL) }
                | TAG_GRAD { START(GRAD) }
                | TAG_RETI { START(RETI) }
                ;

indiv_gen_subs  : /* empty */
                | indiv_gen_subs indiv_gen_sub
                ;

indiv_gen_sub   : event_detail_sub
                | no_std_sub
                ;

indiv_adop_sect : OPEN DELIM TAG_ADOP opt_value 
                  { START(ADOP) }
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
                       { START(FAMC) }
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
                            { START(ADOP) } no_std_subs { CHECK0 } CLOSE { }
                          ;

indiv_even_sect : OPEN DELIM TAG_EVEN
                  { START(EVEN) }
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

lio_bapl_tag  : TAG_BAPL { START(BAPL) }
              | TAG_CONL { START(CONL) }
              | TAG_ENDL { START(ENDL) }
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
                     { START(STAT) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lio_bapl_date_sect : OPEN DELIM TAG_DATE mand_line_item   
                     { START(DATE) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lio_bapl_temp_sect : OPEN DELIM TAG_TEMP mand_line_item   
                     { START(TEMP) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lio_bapl_plac_sect : OPEN DELIM TAG_PLAC mand_line_item   
                     { START(PLAC) } no_std_subs { CHECK0 } CLOSE { }
                   ;

lio_slgc_sect : OPEN DELIM TAG_SLGC
                { START(SLGC) }
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
                     { START(FAMC) } no_std_subs { CHECK0 } CLOSE { }
                   ;

/* LDS SPOUSE SEALING */
lds_spouse_seal_sub : lss_slgs_sect
                    ;

lss_slgs_sect : OPEN DELIM TAG_SLGS
                { START(SLGS) }
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
                     { START(STAT) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lss_slgs_date_sect : OPEN DELIM TAG_DATE mand_line_item   
                     { START(DATE) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lss_slgs_temp_sect : OPEN DELIM TAG_TEMP mand_line_item   
                     { START(TEMP) } no_std_subs { CHECK0 } CLOSE { }
                   ;
lss_slgs_plac_sect : OPEN DELIM TAG_PLAC mand_line_item   
                     { START(PLAC) } no_std_subs { CHECK0 } CLOSE { }
                   ;

/* MULTIMEDIA LINK */
multim_link_sub : multim_obje_link_sect
                | multim_obje_emb_sect
                ;

multim_obje_link_sect : OPEN DELIM TAG_OBJE DELIM POINTER    
                        { START(OBJE) } no_std_subs { CHECK0 } CLOSE { }
                      ;

multim_obje_emb_sect : OPEN DELIM TAG_OBJE
                       { START(OBJE) }
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
                        { START(FORM) } no_std_subs { CHECK0 } CLOSE { }
                      ;
multim_obje_titl_sect : OPEN DELIM TAG_TITL mand_line_item    
                        { START(TITL) } no_std_subs { CHECK0 } CLOSE { }
                      ;
multim_obje_file_sect : OPEN DELIM TAG_FILE mand_line_item    
                        { START(FILE) } no_std_subs { CHECK0 } CLOSE { }
                      ;

/* NOTE STRUCTURE */
note_struc_sub : note_struc_link_sect  /* 0:M */
               | note_struc_emb_sect  /* 0:M */
               ;

note_struc_link_sect : OPEN DELIM TAG_NOTE DELIM POINTER
                       { START(NOTE) }
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
                      { START(NOTE) }
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
                 { START(NAME) }
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
                      { START(NPFX) } no_std_subs { CHECK0 } CLOSE { }
                    ;
pers_name_givn_sect : OPEN DELIM TAG_GIVN mand_line_item    
                      { START(GIVN) } no_std_subs { CHECK0 } CLOSE { }
                    ;
pers_name_nick_sect : OPEN DELIM TAG_NICK mand_line_item    
                      { START(NICK) } no_std_subs { CHECK0 } CLOSE { }
                    ;
pers_name_spfx_sect : OPEN DELIM TAG_SPFX mand_line_item    
                      { START(SPFX) } no_std_subs { CHECK0 } CLOSE { }
                    ;
pers_name_surn_sect : OPEN DELIM TAG_SURN mand_line_item    
                      { START(SURN) } no_std_subs { CHECK0 } CLOSE { }
                    ;
pers_name_nsfx_sect : OPEN DELIM TAG_NSFX mand_line_item    
                      { START(NSFX) } no_std_subs { CHECK0 } CLOSE { }
                    ;

/* PLACE STRUCTURE */
place_struc_sub : place_struc_plac_sect /* 0:M */
                ;

place_struc_plac_sect : OPEN DELIM TAG_PLAC mand_line_item 
                        { START(PLAC) }
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
                       { START(FORM) } no_std_subs { CHECK0 } CLOSE { }
                     ;

/* SOURCE_CITATION */
source_cit_sub : source_cit_link_sect /* 0:M */
               | source_cit_emb_sect /* 0:M */
               ;

source_cit_link_sect : OPEN DELIM TAG_SOUR DELIM POINTER
                       { START(SOUR) }
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
                       { START(PAGE) } no_std_subs { CHECK0 } CLOSE { }
                     ;

source_cit_even_sect : OPEN DELIM TAG_EVEN mand_line_item 
                       { START(EVEN) }
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
                          { START(ROLE) } no_std_subs { CHECK0 } CLOSE { }
                          ;

source_cit_data_sect : OPEN DELIM TAG_DATA
                       { START(DATA) }
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
                            { START(DATE) } no_std_subs { CHECK0 } CLOSE { }
                          ;

source_cit_text_sect : OPEN DELIM TAG_TEXT mand_line_item 
                       { START(TEXT) }
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
                       { START(QUAY) } no_std_subs { CHECK0 } CLOSE { }
                     ;

source_cit_emb_sect : OPEN DELIM TAG_SOUR mand_line_item
                      { START(SOUR) }
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
                         { START(REPO) }
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
            { START(CALN) }
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
                 { START(MEDI) } no_std_subs { CHECK0 } CLOSE { }
               ;
 
/* SPOUSE TO FAMILY LINK */
spou_fam_link_sub : spou_fam_fams_sect  /* 0:M */
                  ;

spou_fam_fams_sect : OPEN DELIM TAG_FAMS mand_pointer
                     { START(FAMS) }
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
		  gedcom_error("Undefined tag (and not a valid user tag): %s",
			       $4);
		  YYERROR;
	        }
	      }
              opt_value user_sects CLOSE { }
            ;

user_sect   : OPEN DELIM opt_xref USERTAG 
              { if ($4[0] != '_') {
		  gedcom_error("Undefined tag (and not a valid user tag): %s",
			       $4);
		  YYERROR;
	        }
	      }
              opt_value user_sects CLOSE { }
            ;

user_sects   : /* empty */     { }
            | user_sects user_sect { }
            ;

opt_xref    : /* empty */        { }
            | POINTER DELIM        { }
            ;

opt_value   : /* empty */        { }
            | DELIM line_value        { }
            ;

line_value  : POINTER        { }
            | line_item        { }
            ;

mand_pointer : /* empty */ { gedcom_error("Missing pointer"); YYERROR; }
             | DELIM POINTER { }
             ;

mand_line_item : /* empty */ { gedcom_error("Missing value"); YYERROR; }
               | DELIM line_item { gedcom_debug_print("==Val: %s==\n", $2);
	                           $$ = $2; }
               ;

opt_line_item : /* empty */ { }
              | DELIM line_item { }
              ;

line_item   : anychar  { size_t i;
		         CLEAR_BUFFER(string_buf);
                         string_buf_ptr = string_buf;
			 /* The following also takes care of '@@' */
			 if (!strncmp($1, "@@", 3))
			   *string_buf_ptr++ = '@';
			 else
			   for (i=0; i < strlen($1); i++)
			     *string_buf_ptr++ = $1[i];
			 $$ = string_buf;
                       }
            | ESCAPE   { CLEAR_BUFFER(string_buf);
	                 string_buf_ptr = string_buf;
			 /* For now, ignore escapes */
			 $$ = string_buf;
	               }
            | line_item anychar
                  { if (strlen(string_buf) >= MAXGEDCLINELEN) {
		      gedcom_error("Line too long");
		      YYERROR;
		    }
		    else {
		      size_t i;
		      /* The following also takes care of '@@' */
		      if (!strncmp($2, "@@", 3))
			*string_buf_ptr++ = '@';
		      else
			for (i=0; i < strlen($2); i++)
			  *string_buf_ptr++ = $2[i];
		      $$ = string_buf;
		    }
		  }
            | line_item ESCAPE
                  { /* For now, ignore escapes */
		    $$ = string_buf;
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
              { gedcom_error("Missing cross-reference"); YYERROR; }
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
char tag_stack[MAXGEDCLEVEL+1][MAXSTDTAGLENGTH+1];

void push_countarray()
{
  int *count = NULL;
  if (count_level > MAXGEDCLEVEL) {
    gedcom_error("Internal error: count array overflow");
    exit(1);
  }
  else {
    count = (int *)calloc(YYNTOKENS, sizeof(int));
    if (count == NULL) {
      gedcom_error("Internal error: count array calloc error");
      exit(1);
    }
    else {
      count_arrays[count_level] = count;
    }
  }
}

void set_parenttag(char* tag)
{
  strncpy(tag_stack[count_level], tag, MAXSTDTAGLENGTH+1);
}

char* get_parenttag()
{
  return tag_stack[count_level];
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
    gedcom_error("Internal error: count array underflow");
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
void gedcom_set_debug_level(int level)
{
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
  int res;
  if (gedcom_high_level_debug) {
    va_list ap;
    va_start(ap, s);
    res = vfprintf(stderr, s, ap);
    va_end(ap);
  }
  return(res);
}

/* Setting the error mechanism */
void gedcom_set_error_handling(MECHANISM mechanism)
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
    gedcom_debug_print("==== Program: %s\n", program);
    if (! strncmp(program, "ftree", 6)) {
      gedcom_warning("Enabling compatibility with 'ftree'");
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

