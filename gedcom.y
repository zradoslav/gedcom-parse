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

int  count_level=0;
MECHANISM curr_mechanism=FAIL_PARSE; 

/* These are defined at the bottom of the file */ 
void push_countarray();
void set_parenttag(char* tag);
char* get_parenttag(); 
void pop_countarray();
int  count_tag(int tag);
int  check_occurrence(int tag);
 
#define OPEN(PARENTTAG) \
     { ++count_level; \
       set_parenttag(#PARENTTAG); \
       push_countarray(); \
     }
#define CHK(TAG) \
     { if (!check_occurrence(TAG_##TAG)) { \
         char* parenttag = get_parenttag(); \
         gedcom_error("The tag '%s' is mandatory within '%s'", \
		      #TAG, parenttag); \
	 YYERROR; \
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
         YYERROR; \
       } \
     }
#define INVALID_TAG(CHILDTAG) \
     { char* parenttag = get_parenttag(); \
       gedcom_error("The tag '%s' is not a valid tag within '%s'", \
		    CHILDTAG, parenttag); \
       YYERROR; \
     }
#define INVALID_TOP_TAG(CHILDTAG) \
     { gedcom_error("The tag '%s' is not a valid top-level tag", \
		    CHILDTAG); \
       YYERROR; \
     }

%}

%union {
  char *string;
}

%token_table

%token <string> BADTOKEN
%token <string> OPEN
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

/*
%type <string> anytag
*/

%%

file        : head_sect records trlr_sect  { }
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
            | user_sect /* 0:M */
            ;

/*********************************************************************/
/**** Header                                                      ****/
/*********************************************************************/
head_sect    : OPEN DELIM TAG_HEAD
               { OPEN(HEAD) }
               head_subs
               { CHECK4(SOUR, SUBM, GEDC, CHAR) }
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
             | user_sect /* 0:M */
	     ;

/* HEAD.SOUR */
head_sour_sect : OPEN DELIM TAG_SOUR DELIM line_item 
                 { OPEN(SOUR) }
                 head_sour_subs
                 { CHECK0 }
		 CLOSE
                       { }
               ;

head_sour_subs : /* empty */
               | head_sour_subs head_sour_sub
               ;

head_sour_sub : head_sour_vers_sect  { OCCUR2(VERS, 0, 1) }
              | head_sour_name_sect  { OCCUR2(NAME, 0, 1) }
              | head_sour_corp_sect  { OCCUR2(CORP, 0, 1) } 
              | head_sour_data_sect  { OCCUR2(DATA, 0, 1) }
              ;
head_sour_vers_sect : OPEN DELIM TAG_VERS DELIM line_item CLOSE
                            { }
                    ;
head_sour_name_sect : OPEN DELIM TAG_NAME DELIM line_item CLOSE
                            { }
                    ;
head_sour_corp_sect : OPEN DELIM TAG_CORP DELIM line_item 
                      { OPEN(CORP) }
                      head_sour_corp_subs
		      { CHECK0 }
                      CLOSE
                            { }
                    ;

head_sour_corp_subs : /* empty */
                    | head_sour_corp_subs head_sour_corp_sub
                    ;

head_sour_corp_sub : addr_struc_sub  /* 0:1 */
                   ;

head_sour_data_sect : OPEN DELIM TAG_DATA DELIM line_item 
                      { OPEN(DATA) }
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
                   ;

head_sour_data_date_sect : OPEN DELIM TAG_DATE DELIM line_item CLOSE
                                { }
                         ;
head_sour_data_copr_sect : OPEN DELIM TAG_COPR DELIM line_item CLOSE
                                { }
                         ;

/* HEAD.DEST */
head_dest_sect : OPEN DELIM TAG_DEST DELIM line_item CLOSE
                       { }
               ;

/* HEAD.DATE */
head_date_sect : OPEN DELIM TAG_DATE DELIM line_item 
                 { OPEN(DATE) }
                 head_date_subs
		 { CHECK0 }
                 CLOSE
                       { }
               ;

head_date_subs : /* empty */
               | head_date_subs head_date_sub
               ;

head_date_sub  : head_date_time_sect  { OCCUR2(TIME, 0, 1) }
               ;

head_date_time_sect : OPEN DELIM TAG_TIME DELIM line_item CLOSE
                          { }
                    ;

/* HEAD.SUBM */
head_subm_sect : OPEN DELIM TAG_SUBM DELIM POINTER CLOSE
                       { }
               ;
/* HEAD.SUBN */
head_subn_sect : OPEN DELIM TAG_SUBN DELIM POINTER CLOSE
                       { }
               ;
/* HEAD.FILE */
head_file_sect : OPEN DELIM TAG_FILE DELIM line_item CLOSE
                       { }
               ;
/* HEAD.COPR */
head_copr_sect : OPEN DELIM TAG_COPR DELIM line_item CLOSE
                       { }
               ;
/* HEAD.GEDC */
head_gedc_sect : OPEN DELIM TAG_GEDC
                 { OPEN(GEDC) }
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
               ;
head_gedc_vers_sect : OPEN DELIM TAG_VERS DELIM line_item CLOSE
                          { }
                    ;
head_gedc_form_sect : OPEN DELIM TAG_FORM DELIM line_item CLOSE
                          { }
                    ;

/* HEAD.CHAR */
head_char_sect : OPEN DELIM TAG_CHAR DELIM line_item 
                 { OPEN(CHAR) }
                 head_char_subs
		 { CHECK0 }
                 CLOSE
                       { }
               ;

head_char_subs : /* empty */
               | head_char_subs head_char_sub
               ;

head_char_sub  : head_char_vers_sect  { OCCUR2(VERS, 0, 1) }
               ;
head_char_vers_sect : OPEN DELIM TAG_VERS DELIM line_item CLOSE
                          { }
                    ;

/* HEAD.LANG */
head_lang_sect : OPEN DELIM TAG_LANG DELIM line_item CLOSE
                       { }
               ;
/* HEAD.PLAC */
head_plac_sect : OPEN DELIM TAG_PLAC
                 { OPEN(PLAC) }
                 head_plac_subs
		 { CHECK1(FORM) }
                 CLOSE
                       { }
               ;

head_plac_subs : /* empty */
               | head_plac_subs head_plac_sub
               ;

head_plac_sub  : head_plac_form_sect  { OCCUR2(FORM, 1, 1) }
               ;
head_plac_form_sect : OPEN DELIM TAG_FORM DELIM line_item CLOSE
                          { }
                    ;

/* HEAD.NOTE */
head_note_sect : OPEN DELIM TAG_NOTE DELIM line_item 
                 { OPEN(NOTE) }
                 head_note_subs
		 { CHECK0 }
                 CLOSE
                       { }
               ;

head_note_subs : /* empty */
               | head_note_subs head_note_sub
               ;

head_note_sub  : continuation_sub  /* 0:M */
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
               { OPEN(FAM) }
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
             | user_sect /* 0:M */
             ;

/* FAM.HUSB */
fam_husb_sect : OPEN DELIM TAG_HUSB DELIM POINTER CLOSE
                       { }
              ;

/* FAM.WIFE */
fam_wife_sect : OPEN DELIM TAG_WIFE DELIM POINTER CLOSE
                       { }
              ;

/* FAM.CHIL */
fam_chil_sect : OPEN DELIM TAG_CHIL DELIM POINTER CLOSE
                       { }
              ;

/* FAM.NCHI */
fam_nchi_sect : OPEN DELIM TAG_NCHI DELIM line_item CLOSE
                       { }
              ;

/* FAM.SUBM */
fam_subm_sect : OPEN DELIM TAG_SUBM DELIM POINTER CLOSE
                       { }
              ;

/*********************************************************************/
/**** Individual record                                           ****/
/*********************************************************************/
indiv_rec   : OPEN DELIM POINTER DELIM TAG_INDI
              { OPEN(INDI) }
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
            | user_sect /* 0:M */
            ;

/* INDI.RESN */
indi_resn_sect : OPEN DELIM TAG_RESN DELIM line_item CLOSE { }
               ;

/* INDI.SEX */
indi_sex_sect  : OPEN DELIM TAG_SEX DELIM line_item CLOSE { }
               ;

/* INDI.SUBM */
indi_subm_sect : OPEN DELIM TAG_SUBM DELIM POINTER CLOSE { }
               ;

/* INDI.ALIA */
indi_alia_sect : OPEN DELIM TAG_ALIA DELIM POINTER CLOSE { }
               ;

/* INDI.ANCI */
indi_anci_sect : OPEN DELIM TAG_ANCI DELIM POINTER CLOSE { }
               ;

/* INDI.DESI */
indi_desi_sect : OPEN DELIM TAG_DESI DELIM POINTER CLOSE { }
               ;

/* INDI.RFN */
indi_rfn_sect  : OPEN DELIM TAG_RFN DELIM line_item CLOSE { }
               ;

/* INDI.AFN */
indi_afn_sect  : OPEN DELIM TAG_AFN DELIM line_item CLOSE { }
               ;

/*********************************************************************/
/**** Multimedia record                                           ****/
/*********************************************************************/
multim_rec  : OPEN DELIM POINTER DELIM TAG_OBJE
              { OPEN(OBJE) }
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
            | user_sect /* 0:M */
            ;

/* OBJE.FORM */
obje_form_sect : OPEN DELIM TAG_FORM DELIM line_item CLOSE { }
               ;

/* OBJE.TITL */
obje_titl_sect : OPEN DELIM TAG_TITL DELIM line_item CLOSE { }
               ;

/* OBJE.BLOB */
obje_blob_sect : OPEN DELIM TAG_BLOB
                 { OPEN(BLOB) }
                 obje_blob_subs
		 { CHECK1(CONT) }
                 CLOSE { }
               ;

obje_blob_subs : /* empty */
               | obje_blob_subs obje_blob_sub
               ;

obje_blob_sub  : obje_blob_cont_sect  { OCCUR1(CONT, 1) }
               ;

obje_blob_cont_sect : OPEN DELIM TAG_CONT DELIM line_item CLOSE { }
                    ;

/* OBJE.OBJE */
obje_obje_sect : OPEN DELIM TAG_OBJE DELIM POINTER CLOSE { }
               ;

/*********************************************************************/
/**** Note record                                                 ****/
/*********************************************************************/
note_rec    : OPEN DELIM POINTER DELIM TAG_NOTE DELIM line_item
              { OPEN(NOTE) }
              note_subs
	      { CHECK0 }
              CLOSE { }
            ;

note_subs   : /* empty */
            | note_subs note_sub
            ;

note_sub    : continuation_sub  /* 0:M */
            | source_cit_sub  /* 0:M */
            | ident_struc_sub  /* 0:1 */
            | change_date_sub  /* 0:1 */
            | user_sect /* 0:M */
            ;

/*********************************************************************/
/**** Repository record                                           ****/
/*********************************************************************/
repos_rec   : OPEN DELIM POINTER DELIM TAG_REPO
              { OPEN(REPO) }
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
            | user_sect /* 0:M */
            ;

/* REPO.NAME */
repo_name_sect : OPEN DELIM TAG_NAME DELIM line_item CLOSE {}
               ;

/*********************************************************************/
/**** Source record                                               ****/
/*********************************************************************/
source_rec  : OPEN DELIM POINTER DELIM TAG_SOUR
              { OPEN(SOUR) }
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
            | user_sect /* 0:M */
            ;

/* SOUR.DATA */
sour_data_sect : OPEN DELIM TAG_DATA
                 { OPEN(DATA) }
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
               ;

sour_data_even_sect : OPEN DELIM TAG_EVEN DELIM line_item 
                      { OPEN(EVEN) }
                      sour_data_even_subs
		      { CHECK0 }
                      CLOSE { }
                    ;

sour_data_even_subs : /* empty */
                    | sour_data_even_subs sour_data_even_sub
                    ;

sour_data_even_sub  : sour_data_even_date_sect { OCCUR2(DATE, 0, 1) }
                    | sour_data_even_plac_sect { OCCUR2(PLAC, 0, 1) }
                    ;

sour_data_even_date_sect : OPEN DELIM TAG_DATE DELIM line_item CLOSE { }
                         ;

sour_data_even_plac_sect : OPEN DELIM TAG_PLAC DELIM line_item CLOSE { }
                         ;

sour_data_agnc_sect : OPEN DELIM TAG_AGNC DELIM line_item CLOSE { }
                    ;

/* SOUR.AUTH */
sour_auth_sect : OPEN DELIM TAG_AUTH DELIM line_item
                 { OPEN(AUTH) }
                 sour_auth_subs
		 { CHECK0 }
                 CLOSE { }
               ;

sour_auth_subs : /* empty */
               | sour_auth_subs sour_auth_sub
               ;

sour_auth_sub  : continuation_sub  /* 0:M */
               ;

/* SOUR.TITL */
sour_titl_sect : OPEN DELIM TAG_TITL DELIM line_item  
                 { OPEN(TITL) }
                 sour_titl_subs 
		 { CHECK0 }
                 CLOSE { }
               ;

sour_titl_subs : /* empty */
               | sour_titl_subs sour_titl_sub
               ;

sour_titl_sub  : continuation_sub  /* 0:M */
               ;

/* SOUR.ABBR */
sour_abbr_sect : OPEN DELIM TAG_ABBR DELIM line_item CLOSE { }
               ;

/* SOUR.PUBL */
sour_publ_sect : OPEN DELIM TAG_PUBL DELIM line_item  
                 { OPEN(PUBL) }
                 sour_publ_subs  
		 { CHECK0 }
                 CLOSE { }
               ;

sour_publ_subs : /* empty */
               | sour_publ_subs sour_publ_sub
               ;

sour_publ_sub  : continuation_sub  /* 0:M */
               ;

/* SOUR.TEXT */
sour_text_sect : OPEN DELIM TAG_TEXT DELIM line_item   
                 { OPEN(TEXT) }
                 sour_text_subs  
		 { CHECK0 }
                 CLOSE { }
               ;

sour_text_subs : /* empty */
               | sour_text_subs sour_text_sub
               ;

sour_text_sub  : continuation_sub  /* 0:M */
               ;

/*********************************************************************/
/**** Submission record                                           ****/
/*********************************************************************/
submis_rec  : OPEN DELIM POINTER DELIM TAG_SUBN    
              { OPEN(SUBN) }
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
            | user_sect /* 0:M */
            ;

/* SUBN.SUBM */
subn_subm_sect : OPEN DELIM TAG_SUBM DELIM POINTER CLOSE { }
               ;

/* SUBN.FAMF */
subn_famf_sect : OPEN DELIM TAG_FAMF DELIM line_item CLOSE { }
               ;

/* SUBN.TEMP */
subn_temp_sect : OPEN DELIM TAG_TEMP DELIM line_item CLOSE { }
               ;

/* SUBN.ANCE */
subn_ance_sect : OPEN DELIM TAG_ANCE DELIM line_item CLOSE { }
               ;

/* SUBN.DESC */
subn_desc_sect : OPEN DELIM TAG_DESC DELIM line_item CLOSE { }
               ;

/* SUBN.ORDI */
subn_ordi_sect : OPEN DELIM TAG_ORDI DELIM line_item CLOSE { }
               ;

/* SUBN.RIN */
subn_rin_sect  : OPEN DELIM TAG_RIN DELIM line_item CLOSE { }
               ;

/*********************************************************************/
/**** Submitter record                                            ****/
/*********************************************************************/
submit_rec : OPEN DELIM POINTER DELIM TAG_SUBM    
             { OPEN(SUBM) }
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
           | user_sect /* 0:M */
           ;

/* SUBM.NAME */
subm_name_sect : OPEN DELIM TAG_NAME DELIM line_item CLOSE { }
               ;

/* SUBM.LANG */
subm_lang_sect : OPEN DELIM TAG_LANG DELIM line_item CLOSE { }
               ;

/* SUBM.RFN */
subm_rfn_sect  : OPEN DELIM TAG_RFN DELIM line_item CLOSE { }
               ;

/* SUBM.RIN */
subm_rin_sect  : OPEN DELIM TAG_RIN DELIM line_item CLOSE { }
               ;

/*********************************************************************/
/**** Substructures                                               ****/
/*********************************************************************/

/* ADDRESS STRUCTURE */
addr_struc_sub : addr_sect { OCCUR2(ADDR, 0, 1) }
               | phon_sect { OCCUR2(PHON, 0, 3) }
               ;

addr_sect   : OPEN DELIM TAG_ADDR DELIM line_item 
              { OPEN(ADDR) }
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
            ;

addr_cont_sect : OPEN DELIM TAG_CONT DELIM line_item CLOSE { }
               ;
addr_adr1_sect : OPEN DELIM TAG_ADR1 DELIM line_item CLOSE { }
               ;
addr_adr2_sect : OPEN DELIM TAG_ADR2 DELIM line_item CLOSE { }
               ;
addr_city_sect : OPEN DELIM TAG_CITY DELIM line_item CLOSE { }
               ;
addr_stae_sect : OPEN DELIM TAG_STAE DELIM line_item CLOSE { }
               ;
addr_post_sect : OPEN DELIM TAG_POST DELIM line_item CLOSE { }
               ;
addr_ctry_sect : OPEN DELIM TAG_CTRY DELIM line_item CLOSE { }
               ;

phon_sect   : OPEN DELIM TAG_PHON DELIM line_item CLOSE { }
            ;

/* ASSOCIATION STRUCTURE */
assoc_struc_sub : asso_sect /* 0:M */
                ;

asso_sect : OPEN DELIM TAG_ASSO DELIM POINTER 
            { OPEN(ASSO) }
            asso_subs
	    { CHECK2(TYPE,RELA) }
            CLOSE { }
          ;

asso_subs : /* empty */
          | asso_type_sect  { OCCUR2(TYPE, 1, 1) }
          | asso_rela_sect  { OCCUR2(RELA, 1, 1) }
          | note_struc_sub
          | source_cit_sub
          ;

asso_type_sect : OPEN DELIM TAG_TYPE DELIM line_item CLOSE { }
               ;

asso_rela_sect : OPEN DELIM TAG_RELA DELIM line_item CLOSE { }
               ;

/* CHANGE DATE */
change_date_sub : change_date_chan_sect  { OCCUR2(CHAN, 0, 1) }
                ;

change_date_chan_sect : OPEN DELIM TAG_CHAN
                        { OPEN(CHAN) }
                        change_date_chan_subs
			{ CHECK1(DATE) }
                        CLOSE { }
                      ;

change_date_chan_subs : /* empty */
                      | change_date_chan_subs change_date_chan_sub
                      ;

change_date_chan_sub  : change_date_date_sect  { OCCUR2(DATE, 1, 1) }
                      | note_struc_sub
                      ;

change_date_date_sect : OPEN DELIM TAG_DATE DELIM line_item 
                        { OPEN(DATE) }
                        change_date_date_subs
			{ CHECK0 }
                        CLOSE { }
                      ;

change_date_date_subs : /* empty */
                      | change_date_date_subs change_date_date_sub
                      ;

change_date_date_sub : change_date_date_time_sect  { OCCUR2(TIME, 0, 1) }
                     ;

change_date_date_time_sect : OPEN DELIM TAG_TIME DELIM line_item CLOSE { }
                           ;

/* CHILD TO FAMILY LINK */
chi_fam_link_sub : famc_sect  /* 0:M */
                 ;

famc_sect : OPEN DELIM TAG_FAMC DELIM POINTER 
            { OPEN(FAMC) }
            famc_subs
	    { CHECK0 }
            CLOSE { }
          ;

famc_subs : /* empty */
          | famc_subs famc_sub
          ;

famc_sub  : famc_pedi_sect  /* 0:M */
          | note_struc_sub
          ;

famc_pedi_sect : OPEN DELIM TAG_PEDI DELIM line_item CLOSE { }
               ;

/* CONTINUATION SUBSECTIONS */
continuation_sub : cont_sect  /* 0:M */
                 | conc_sect  /* 0:M */
                 ;

cont_sect : OPEN DELIM TAG_CONT DELIM line_item CLOSE { }
          ;

conc_sect : OPEN DELIM TAG_CONC DELIM line_item CLOSE { }
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

event_detail_type_sect : OPEN DELIM TAG_TYPE DELIM line_item CLOSE { }
                       ;
event_detail_date_sect : OPEN DELIM TAG_DATE DELIM line_item CLOSE { }
                       ;
event_detail_age_sect  : OPEN DELIM TAG_AGE DELIM line_item CLOSE { }
                       ;
event_detail_agnc_sect : OPEN DELIM TAG_AGNC DELIM line_item CLOSE { }
                       ;
event_detail_caus_sect : OPEN DELIM TAG_CAUS DELIM line_item CLOSE { }
                       ;

/* FAMILY EVENT STRUCTURE */
fam_event_struc_sub : fam_event_sect
                    | fam_gen_even_sect  /* 0:M */
                    ;

fam_event_sect : OPEN DELIM fam_event_tag opt_value fam_event_subs
                 { CHECK0 }
                 CLOSE { }
               ;

fam_event_tag : TAG_ANUL { OPEN(ANUL) }
              | TAG_CENS { OPEN(CENS) }
              | TAG_DIV { OPEN(DIV) }
              | TAG_DIVF { OPEN(DIVF) }
              | TAG_ENGA { OPEN(ENGA) }
              | TAG_MARR { OPEN(MARR) }
              | TAG_MARB { OPEN(MARB) }
              | TAG_MARC { OPEN(MARC) }
              | TAG_MARL { OPEN(MARL) }
              | TAG_MARS { OPEN(MARS) }
              ;

fam_event_subs : /* empty */
               | fam_event_subs fam_event_sub
               ;

fam_event_sub : event_detail_sub
              | fam_even_husb_sect  { OCCUR2(HUSB, 0, 1) }
              | fam_even_wife_sect  { OCCUR2(WIFE, 0, 1) }
              ;

fam_even_husb_sect : OPEN DELIM TAG_HUSB
                     { OPEN(HUSB) }
                     fam_even_husb_subs
		     { CHECK1(AGE) }
                     CLOSE { }
                   ;

fam_even_husb_subs : /* empty */
                   | fam_even_husb_subs fam_even_husb_sub
                   ;

fam_even_husb_sub : fam_even_husb_age_sect  { OCCUR2(AGE, 1, 1) }
                  ;

fam_even_husb_age_sect : OPEN DELIM TAG_AGE DELIM line_item CLOSE { }
                       ;

fam_even_wife_sect : OPEN DELIM TAG_WIFE
                     { OPEN(HUSB) }
                     fam_even_husb_subs
		     { CHECK1(AGE) }
                     CLOSE { }
                   ;

fam_gen_even_sect : OPEN DELIM TAG_EVEN
                    { OPEN(EVEN) }
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
                 ;

/* IDENTIFICATION STRUCTURE */
ident_struc_sub : ident_refn_sect  /* 0:M */
                | ident_rin_sect  { OCCUR2(RIN, 0, 1) }
                ;

ident_refn_sect : OPEN DELIM TAG_REFN DELIM line_item 
                  { OPEN(REFN) }
                  ident_refn_subs
		  { CHECK0 }
                  CLOSE { }
                ;

ident_refn_subs : /* empty */
                | ident_refn_subs ident_refn_sub
                ;

ident_refn_sub  : ident_refn_type_sect  { OCCUR2(TYPE, 0, 1) }
                ;

ident_refn_type_sect : OPEN DELIM TAG_TYPE DELIM line_item CLOSE { }
                     ;

ident_rin_sect  : OPEN DELIM TAG_RIN DELIM line_item CLOSE { }
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

indiv_cast_sect : OPEN DELIM TAG_CAST DELIM line_item 
                  { OPEN(CAST) }
                  indiv_attr_event_subs
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_dscr_sect : OPEN DELIM TAG_DSCR DELIM line_item 
                  { OPEN(DSCR) }
                  indiv_attr_event_subs
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_educ_sect : OPEN DELIM TAG_EDUC DELIM line_item  
                  { OPEN(EDUC) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_idno_sect : OPEN DELIM TAG_IDNO DELIM line_item 
                  { OPEN(IDNO) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_nati_sect : OPEN DELIM TAG_NATI DELIM line_item 
                  { OPEN(NATI) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_nchi_sect : OPEN DELIM TAG_NCHI DELIM line_item 
                  { OPEN(NCHI) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_nmr_sect  : OPEN DELIM TAG_NMR DELIM line_item 
                  { OPEN(NMR) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_occu_sect : OPEN DELIM TAG_OCCU DELIM line_item 
                  { OPEN(OCCU) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_prop_sect : OPEN DELIM TAG_PROP DELIM line_item 
                  { OPEN(PROP) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_reli_sect : OPEN DELIM TAG_RELI DELIM line_item 
                  { OPEN(RELI) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_resi_sect : OPEN DELIM TAG_RESI 
                  { OPEN(RESI) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_ssn_sect  : OPEN DELIM TAG_SSN DELIM line_item 
                  { OPEN(SSN) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;
indiv_titl_sect : OPEN DELIM TAG_TITL DELIM line_item 
                  { OPEN(TITL) }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE { }
                ;

indiv_attr_event_subs : /* empty */
                      | indiv_attr_event_subs indiv_attr_event_sub
                      ;

indiv_attr_event_sub  : event_detail_sub
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

indiv_birt_tag  : TAG_BIRT { OPEN(BIRT) }
                | TAG_CHR { OPEN(CHR) }
                ;

indiv_birt_subs : /* empty */
                | indiv_birt_subs indiv_birt_sub
                ;

indiv_birt_sub  : event_detail_sub
                | indiv_birt_famc_sect  { OCCUR2(FAMC,0, 1) }
                ;

indiv_birt_famc_sect : OPEN DELIM TAG_FAMC DELIM POINTER CLOSE { }
                     ;

indiv_gen_sect  : OPEN DELIM indiv_gen_tag opt_value indiv_gen_subs
                  { CHECK0 }
                  CLOSE { }
                ;

indiv_gen_tag   : TAG_DEAT { OPEN(DEAT) }
                | TAG_BURI { OPEN(BURI) }
                | TAG_CREM { OPEN(CREM) }
                | TAG_BAPM { OPEN(BAPM) }
                | TAG_BARM { OPEN(BARM) }
                | TAG_BASM { OPEN(BASM) }
                | TAG_BLES { OPEN(BLES) }
                | TAG_CHRA { OPEN(CHRA) }
                | TAG_CONF { OPEN(CONF) }
                | TAG_FCOM { OPEN(FCOM) }
                | TAG_ORDN { OPEN(ORDN) }
                | TAG_NATU { OPEN(NATU) }
                | TAG_EMIG { OPEN(EMIG) }
                | TAG_IMMI { OPEN(IMMI) }
                | TAG_CENS { OPEN(CENS) }
                | TAG_PROB { OPEN(PROB) }
                | TAG_WILL { OPEN(WILL) }
                | TAG_GRAD { OPEN(GRAD) }
                | TAG_RETI { OPEN(RETI) }
                ;

indiv_gen_subs  : /* empty */
                | indiv_gen_subs indiv_gen_sub
                ;

indiv_gen_sub   : event_detail_sub
                ;

indiv_adop_sect : OPEN DELIM TAG_ADOP opt_value 
                  { OPEN(ADOP) }
                  indiv_adop_subs
		  { CHECK0 }
                  CLOSE { }
                ;

indiv_adop_subs : /* empty */
                | indiv_adop_subs indiv_adop_sub
                ;

indiv_adop_sub  : event_detail_sub
                | indiv_adop_famc_sect  { OCCUR2(FAMC,0, 1) }
                ;

indiv_adop_famc_sect : OPEN DELIM TAG_FAMC DELIM POINTER 
                       { OPEN(FAMC) }
                       indiv_adop_famc_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

indiv_adop_famc_subs : /* empty */
                     | indiv_adop_famc_subs indiv_adop_famc_sub
                     ;

indiv_adop_famc_sub  : indiv_adop_famc_adop_sect  { OCCUR2(ADOP,0, 1) }
                     ;

indiv_adop_famc_adop_sect : OPEN DELIM TAG_ADOP DELIM line_item CLOSE { }
                          ;

indiv_even_sect : OPEN DELIM TAG_EVEN
                  { OPEN(EVEN) }
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

lio_bapl_tag  : TAG_BAPL { OPEN(BAPL) }
              | TAG_CONL { OPEN(CONL) }
              | TAG_ENDL { OPEN(ENDL) }
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
              ;

lio_bapl_stat_sect : OPEN DELIM TAG_STAT DELIM line_item CLOSE { }
                   ;
lio_bapl_date_sect : OPEN DELIM TAG_DATE DELIM line_item CLOSE { }
                   ;
lio_bapl_temp_sect : OPEN DELIM TAG_TEMP DELIM line_item CLOSE { }
                   ;
lio_bapl_plac_sect : OPEN DELIM TAG_PLAC DELIM line_item CLOSE { }
                   ;

lio_slgc_sect : OPEN DELIM TAG_SLGC
                { OPEN(SLGC) }
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

lio_slgc_famc_sect : OPEN DELIM TAG_FAMC DELIM POINTER CLOSE { }
                   ;

/* LDS SPOUSE SEALING */
lds_spouse_seal_sub : lss_slgs_sect
                    ;

lss_slgs_sect : OPEN DELIM TAG_SLGS
                { OPEN(SLGS) }
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
              ;

lss_slgs_stat_sect : OPEN DELIM TAG_STAT DELIM line_item CLOSE { }
                   ;
lss_slgs_date_sect : OPEN DELIM TAG_DATE DELIM line_item CLOSE { }
                   ;
lss_slgs_temp_sect : OPEN DELIM TAG_TEMP DELIM line_item CLOSE { }
                   ;
lss_slgs_plac_sect : OPEN DELIM TAG_PLAC DELIM line_item CLOSE { }
                   ;

/* MULTIMEDIA LINK */
multim_link_sub : multim_obje_link_sect
                | multim_obje_emb_sect
                ;

multim_obje_link_sect : OPEN DELIM TAG_OBJE DELIM POINTER CLOSE { }
                      ;

multim_obje_emb_sect : OPEN DELIM TAG_OBJE
                       { OPEN(OBJE) }
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
                    ;

multim_obje_form_sect : OPEN DELIM TAG_FORM DELIM line_item CLOSE { }
                      ;
multim_obje_titl_sect : OPEN DELIM TAG_TITL DELIM line_item CLOSE { }
                      ;
multim_obje_file_sect : OPEN DELIM TAG_FILE DELIM line_item CLOSE { }
                      ;

/* NOTE STRUCTURE */
note_struc_sub : note_struc_link_sect  /* 0:M */
               | note_struc_emb_sect  /* 0:M */
               ;

note_struc_link_sect : OPEN DELIM TAG_NOTE DELIM POINTER
                       { OPEN(NOTE) }
                       note_struc_link_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

note_struc_link_subs : /* empty */
                     | note_struc_link_subs note_struc_link_sub
                     ;

note_struc_link_sub : source_cit_sub
                    ;

note_struc_emb_sect : OPEN DELIM TAG_NOTE opt_line_item
                      { OPEN(NOTE) }
                      note_struc_emb_subs
		      { CHECK0 }
                      CLOSE { }
                    ;

note_struc_emb_subs : /* empty */
                    | note_struc_emb_subs note_struc_emb_sub
                    ;

note_struc_emb_sub  : continuation_sub
                    | source_cit_sub
                    ;

/* PERSONAL NAME STRUCTURE */
pers_name_struc_sub : pers_name_sect /* 0:M */
                    ;

pers_name_sect : OPEN DELIM TAG_NAME DELIM line_item 
                 { OPEN(NAME) }
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
               ;

pers_name_npfx_sect : OPEN DELIM TAG_NPFX DELIM line_item CLOSE { }
                    ;
pers_name_givn_sect : OPEN DELIM TAG_GIVN DELIM line_item CLOSE { }
                    ;
pers_name_nick_sect : OPEN DELIM TAG_NICK DELIM line_item CLOSE { }
                    ;
pers_name_spfx_sect : OPEN DELIM TAG_SPFX DELIM line_item CLOSE { }
                    ;
pers_name_surn_sect : OPEN DELIM TAG_SURN DELIM line_item CLOSE { }
                    ;
pers_name_nsfx_sect : OPEN DELIM TAG_NSFX DELIM line_item CLOSE { }
                    ;

/* PLACE STRUCTURE */
place_struc_sub : place_struc_plac_sect /* 0:M */
                ;

place_struc_plac_sect : OPEN DELIM TAG_PLAC DELIM line_item 
                        { OPEN(PLAC) }
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
                     ;

place_plac_form_sect : OPEN DELIM TAG_FORM DELIM line_item CLOSE { }
                     ;

/* SOURCE_CITATION */
source_cit_sub : source_cit_link_sect /* 0:M */
               | source_cit_emb_sect /* 0:M */
               ;

source_cit_link_sect : OPEN DELIM TAG_SOUR DELIM POINTER
                       { OPEN(SOUR) }
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
                    ;

source_cit_page_sect : OPEN DELIM TAG_PAGE DELIM line_item CLOSE { }
                     ;

source_cit_even_sect : OPEN DELIM TAG_EVEN DELIM line_item 
                       { OPEN(EVEN) }
                       source_cit_even_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

source_cit_even_subs : /* empty */
                     | source_cit_even_subs source_cit_even_sub
                     ;

source_cit_even_sub  : source_cit_even_role_sect  { OCCUR2(ROLE, 0, 1) }
                     ;

source_cit_even_role_sect : OPEN DELIM TAG_ROLE DELIM line_item CLOSE { }
                          ;

source_cit_data_sect : OPEN DELIM TAG_DATA
                       { OPEN(DATA) }
                       source_cit_data_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

source_cit_data_subs : /* empty */
                     | source_cit_data_subs source_cit_data_sub
                     ;

source_cit_data_sub : source_cit_data_date_sect  { OCCUR2(DATE, 0, 1) }
                    | source_cit_text_sect  /* 0:M */
                    ;

source_cit_data_date_sect : OPEN DELIM TAG_DATE DELIM line_item CLOSE { }
                          ;

source_cit_text_sect : OPEN DELIM TAG_TEXT DELIM line_item 
                       { OPEN(TEXT) }
                       source_cit_text_subs
		       { CHECK0 }
                       CLOSE { }
                     ;

source_cit_text_subs : /* empty */
                     | source_cit_text_subs source_cit_text_sub
                     ;

source_cit_text_sub : continuation_sub
                    ;

source_cit_quay_sect : OPEN DELIM TAG_QUAY DELIM line_item CLOSE { }
                     ;

source_cit_emb_sect : OPEN DELIM TAG_SOUR DELIM line_item
                      { OPEN(SOUR) }
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
                   ;

/* SOURCE REPOSITORY CITATION */
source_repos_cit_sub : source_repos_repo_sect  { OCCUR2(REPO, 0, 1) }
                     ;

source_repos_repo_sect : OPEN DELIM TAG_REPO DELIM POINTER 
                         { OPEN(REPO) }
                         source_repos_repo_subs
			 { CHECK0 }
                         CLOSE { }
                       ;

source_repos_repo_subs : /* empty */
                       | source_repos_repo_subs source_repos_repo_sub
                       ;

source_repos_repo_sub  : note_struc_sub
                       | caln_sect  /* 0:M */
                       ;

caln_sect : OPEN DELIM TAG_CALN DELIM line_item 
            { OPEN(CALN) }
            caln_subs
	    { CHECK0 }
            CLOSE { }
          ;

caln_subs : /* empty */
          | caln_subs caln_sub
          ;

caln_sub  : caln_medi_sect  { OCCUR2(MEDI, 0, 1) }
          ;

caln_medi_sect : OPEN DELIM TAG_MEDI DELIM line_item CLOSE { }
               ;
 
/* SPOUSE TO FAMILY LINK */
spou_fam_link_sub : spou_fam_fams_sect  /* 0:M */
                  ;

spou_fam_fams_sect : OPEN DELIM TAG_FAMS DELIM POINTER 
                     { OPEN(FAMS) }
                     spou_fam_fams_subs
		     { CHECK0 }
                     CLOSE { }
                   ;

spou_fam_fams_subs : /* empty */
                   | spou_fam_fams_subs spou_fam_fams_sub
                   ;

spou_fam_fams_sub  : note_struc_sub
                   ;

/*********************************************************************/
/**** General                                                     ****/
/*********************************************************************/

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

opt_line_item : /* empty */ { }
              | DELIM line_item { }
              ;

line_item   : anychar        { }
            | ESCAPE        { }
            | line_item anychar        { }
            | line_item ESCAPE        { }
            ;

anychar     : ANYCHAR        { }
            | DELIM        { }
            ;

/*
gen_sect    : OPEN DELIM opt_xref anytag
              { INVALID_TAG($4); }
              opt_value opt_sects CLOSE
              { }
            ;

gen_rec     : OPEN DELIM opt_xref anytag
              { INVALID_TOP_TAG($4) }
              opt_value opt_sects CLOSE
              { }
            ;

opt_sects   : <empty>     { }
            | opt_sects gen_sect { }
            ;

anytag      : TAG_ABBR
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
*/
%%

/* Functions that handle the counting of subtags */

int* count_arrays[MAXGEDCOMLEVEL+1];
char tag_stack[MAXGEDCOMLEVEL+1][MAXSTDTAGLENGTH+1];

void push_countarray()
{
  int *count = NULL;
  if (count_level > MAXGEDCOMLEVEL) {
    gedcom_error("Internal error: count array overflow");
    exit(1);
  }
  else {
    count = (int *)calloc(YYNTOKENS, sizeof(int));
    if (count == NULL) {  int *count = count_arrays[count_level];

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

void gedcom_enable_debug()
{
#if YYDEBUG != 0
  gedcom_debug = 1;
#endif
}

/* Setting the error mechanism */

void gedcom_set_error_handling(MECHANISM mechanism)
{
  curr_mechanism = mechanism;
}

