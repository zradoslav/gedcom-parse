/* Parser for Gedcom.
   Copyright (C) 2001, 2002 The Genes Development Team
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
#include "age.h"
#include "xref.h"
#include "compat.h"
#include "buffer.h"

int  count_level    = 0;
int  fail           = 0;
int  gedcom_high_level_debug = 0; 
Gedcom_err_mech error_mechanism = IMMED_FAIL;
Gedcom_val_struct val1;
Gedcom_val_struct val2; 

void cleanup_line_item_buffer();
struct safe_buffer line_item_buffer = { NULL, 0, NULL, 0,
					cleanup_line_item_buffer };

void cleanup_concat_buffer(); 
struct safe_buffer concat_buffer = { NULL, 0, NULL, 0, cleanup_concat_buffer };

void cleanup_usertag_buffer();
struct safe_buffer usertag_buffer = { NULL, 0, NULL, 0,
				      cleanup_usertag_buffer};
 
/* These are defined at the bottom of the file */ 
void push_countarray(int level);
void set_parenttag(const char* tag);
char* get_parenttag(int offset); 
void set_parentctxt(Gedcom_ctxt ctxt);
Gedcom_ctxt get_parentctxt(int offset);
void pop_countarray();
int  count_tag(int tag);
int  check_occurrence(int tag);
void clean_up();

#define HANDLE_ERROR                                                          \
     { if (error_mechanism == IMMED_FAIL) {                                   \
	 clean_up(); YYABORT;                                                 \
       }                                                                      \
       else if (error_mechanism == DEFER_FAIL) {                              \
         gedcom_debug_print("Fail on line %d", line_no);                      \
         yyerrok; fail = 1;                                                   \
       }                                                                      \
       else if (error_mechanism == IGNORE_ERRORS) {                           \
	 yyerrok;                                                             \
       }                                                                      \
     }
#define START1(PARENTTAG)                                                     \
     { set_parenttag(#PARENTTAG);                                             \
     }
#define START2(LEVEL,PARENTCTXT)                                              \
     { set_parentctxt(PARENTCTXT);                                            \
       ++count_level;                                                         \
       push_countarray(LEVEL);                                                \
     }
#define START(PARENTTAG,LEVEL,PARENTCTXT)                                     \
     { START1(PARENTTAG);                                                     \
       START2(LEVEL,PARENTCTXT);                                              \
     }
#define PARENT                                                                \
     get_parentctxt(0)
#define GRANDPARENT(OFF)                                                      \
     get_parentctxt(OFF)
#define CHK(TAG)                                                              \
     { if (!check_occurrence(TAG_##TAG)) {                                    \
         char* parenttag = get_parenttag(0);                                  \
         gedcom_error(_("The tag '%s' is mandatory within '%s', but missing"),\
		      #TAG, parenttag);                                       \
         HANDLE_ERROR;                                                        \
       }                                                                      \
     }
#define CHK_COND(TAG)                                                         \
     check_occurrence(TAG_##TAG)
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
         char* parenttag = get_parenttag(0);                                  \
         gedcom_error(_("The tag '%s' can maximally occur %d time(s) within '%s'"),                                                                          \
		      #CHILDTAG, MAX, parenttag);                             \
         HANDLE_ERROR;                                                        \
       }                                                                      \
     }
#define INVALID_TAG(CHILDTAG)                                                 \
     { char* parenttag = get_parenttag(0);                                    \
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
  struct tag_struct tag;
  Gedcom_ctxt ctxt;
}

%token_table
%expect 317

%token <string> BADTOKEN
%token <number> OPEN
%token <string> CLOSE
%token <string> ESCAPE
%token <string> DELIM
%token <string> ANYCHAR
%token <string> POINTER
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
%type <tag> fam_event_tag
%type <tag> indiv_attr_tag
%type <tag> indiv_birt_tag
%type <tag> indiv_gen_tag
%type <tag> lio_bapl_tag
%type <string> line_item
%type <string> mand_line_item
%type <string> mand_pointer
%type <string> note_line_item
%type <string> anychar
%type <string> opt_xref
%type <string> opt_value
%type <string> opt_line_item
%type <ctxt> head_sect

%%

file        : head_sect records trlr_sect
               { compat_close();
		 if (fail == 1) YYABORT;
	       }
            | error
               { compat_close();
	         clean_up();
	       }
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
               { $<ctxt>$ = start_record(REC_HEAD, $1, GEDCOM_MAKE_NULL(val1),
					 $3,
					 NULL, GEDCOM_MAKE_NULL(val2));
	         START(HEAD, $1, $<ctxt>$) }
               head_subs
               { if (compat_mode(C_NO_SUBMITTER) && ! CHK_COND(SUBM))
		   compat_generate_submitter_link($<ctxt>4);
	         else CHK(SUBM);

	         if (compat_mode(C_NO_GEDC) && ! CHK_COND(GEDC))
		   compat_generate_gedcom($<ctxt>4);
		 else CHK(GEDC);

		 if (compat_mode(C_NO_CHAR) && ! CHK_COND(CHAR)) {
		   if (compat_generate_char($<ctxt>4)) HANDLE_ERROR;
		 }
		 else CHK(CHAR);

		 CHECK1(SOUR);
	       }
               CLOSE
               { end_record(REC_HEAD, $<ctxt>4, GEDCOM_MAKE_NULL(val1));
	         if (compat_mode(C_NO_SUBMITTER))
	           compat_generate_submitter();
	       }
             ;

head_subs    : /* empty */
             | head_subs head_sub
             ;

head_sub     : head_sour_sect  { OCCUR2(SOUR, 1, 1) }
             | head_dest_sect  { OCCUR2(DEST, 0, 1) }
             | head_date_sect  { OCCUR2(DATE, 0, 1) }
             | head_time_sect  { if (!compat_mode(C_HEAD_TIME))
	                          INVALID_TAG("TIME");
	                         OCCUR2(TIME, 0, 1) }
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
                 { set_compatibility_program($4);
		   $<ctxt>$ = start_element(ELT_HEAD_SOUR, PARENT,
					    $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(SOUR, $1, $<ctxt>$)
		 }
                 head_sour_subs
                 { CHECK0 }
		 CLOSE
                 { compute_compatibility();
		   end_element(ELT_HEAD_SOUR, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1)); }
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
                      { set_compatibility_version($4);
			$<ctxt>$ = start_element(ELT_HEAD_SOUR_VERS, PARENT,
						 $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(VERS, $1, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_SOUR_VERS,
				    PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		      }
                    ;
head_sour_name_sect : OPEN DELIM TAG_NAME mand_line_item
                      { $<ctxt>$ = start_element(ELT_HEAD_SOUR_NAME, PARENT,
						 $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
			START(NAME, $1, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_SOUR_NAME,
				    PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		      }
                    ;
head_sour_corp_sect : OPEN DELIM TAG_CORP mand_line_item 
                      { $<ctxt>$ = start_element(ELT_HEAD_SOUR_CORP, PARENT,
						 $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
			START(CORP, $1, $<ctxt>$)
		      }
                      head_sour_corp_subs
		      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_SOUR_CORP,
				    PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
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
						 GEDCOM_MAKE_STRING(val1, $4));
			START(DATA, $1, $<ctxt>$)
		      }
                      head_sour_data_subs
                      { CHECK0 }
		      CLOSE
                      { end_element(ELT_HEAD_SOUR_DATA,
				    PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
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
			     $<ctxt>$
			       = start_element(ELT_HEAD_SOUR_DATA_DATE,
					       PARENT, $1, $3, $4,
					       GEDCOM_MAKE_DATE(val1, dv));
			     START(DATE, $1, $<ctxt>$)
			   }
                           no_std_subs
                           { CHECK0 }
                           CLOSE
                           { end_element(ELT_HEAD_SOUR_DATA_DATE,
					 PARENT, $<ctxt>5,
					 GEDCOM_MAKE_NULL(val1));
			   }
                         ;
head_sour_data_copr_sect : OPEN DELIM TAG_COPR mand_line_item
                           { $<ctxt>$
			       = start_element(ELT_HEAD_SOUR_DATA_COPR,
					       PARENT, $1, $3, $4,
					       GEDCOM_MAKE_STRING(val1, $4));
			     START(COPR, $1, $<ctxt>$)
			   }
                           no_std_subs
                           { CHECK0 }
                           CLOSE
                           { end_element(ELT_HEAD_SOUR_DATA_COPR,
					 PARENT, $<ctxt>5,
					 GEDCOM_MAKE_NULL(val1));
			   }
                         ;

/* HEAD.DEST */
head_dest_sect : OPEN DELIM TAG_DEST mand_line_item
                 { $<ctxt>$ = start_element(ELT_HEAD_DEST,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(DEST, $1, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_DEST,
			       PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* HEAD.DATE */
head_date_sect : OPEN DELIM TAG_DATE mand_line_item 
                 { struct date_value dv = gedcom_parse_date($4);
		   $<ctxt>$ = start_element(ELT_HEAD_DATE,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_DATE(val1, dv));
		   if (compat_mode(C_HEAD_TIME))
		     compat_save_head_date_context($<ctxt>$);
		   START(DATE, $1, $<ctxt>$)
		 }
                 head_date_subs
		 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_DATE,
			       PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
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
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(TIME, $1, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_DATE_TIME,
				    PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		      }
                    ;

/* HEAD.TIME (Only for compatibility) */
head_time_sect : OPEN DELIM TAG_TIME opt_line_item
                 { if (compat_mode(C_HEAD_TIME)) {
		     $<ctxt>$ = compat_generate_head_time_start($1, $3, $4);
                   }
                 }
                 CLOSE
                 { if (compat_mode (C_HEAD_TIME)) {
		     compat_generate_head_time_end($<ctxt>5);
		   }
                 }
	       ;

/* HEAD.SUBM */
head_subm_sect : OPEN DELIM TAG_SUBM mand_pointer
                 { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							     XREF_SUBM);
	           if (xr == NULL) HANDLE_ERROR;
		   $<ctxt>$ = start_element(ELT_HEAD_SUBM,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_XREF_PTR(val1, xr));
		   START(SUBM, $1, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_SUBM,
			       PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		 }
               ;
/* HEAD.SUBN */
head_subn_sect : OPEN DELIM TAG_SUBN mand_pointer 
                 { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							     XREF_SUBN);
	           if (xr == NULL) HANDLE_ERROR;
		   $<ctxt>$ = start_element(ELT_HEAD_SUBN,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_XREF_PTR(val1, xr));
		   START(SUBN, $1, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_SUBN,
			       PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		 }
               ;
/* HEAD.FILE */
head_file_sect : OPEN DELIM TAG_FILE mand_line_item 
                 { $<ctxt>$ = start_element(ELT_HEAD_FILE,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(FILE, $1, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_FILE, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;
/* HEAD.COPR */
head_copr_sect : OPEN DELIM TAG_COPR mand_line_item 
                 { $<ctxt>$ = start_element(ELT_HEAD_COPR,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(COPR, $1, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_COPR, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;
/* HEAD.GEDC */
head_gedc_sect : OPEN DELIM TAG_GEDC
                 { $<ctxt>$ = start_element(ELT_HEAD_GEDC,
					    PARENT, $1, $3, NULL,
					    GEDCOM_MAKE_NULL(val1));
		   START(GEDC, $1, $<ctxt>$)
		 }
                 head_gedc_subs
		 { if (compat_mode(C_NO_GEDC_FORM) && ! CHK_COND(FORM))
		     compat_generate_gedcom_form($<ctxt>4);
		   else CHK(FORM);
		 
		   CHECK1(VERS)  
		 }

                 CLOSE
                 { end_element(ELT_HEAD_GEDC, PARENT, $<ctxt>4,
			       GEDCOM_MAKE_NULL(val1));
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
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(VERS, $1, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_GEDC_VERS,
				    PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		      }
                    ;
head_gedc_form_sect : OPEN DELIM TAG_FORM mand_line_item   
                      { $<ctxt>$ = start_element(ELT_HEAD_GEDC_FORM,
						 PARENT, $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(FORM, $1, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_GEDC_FORM,
				    PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		      }
                    ;

/* HEAD.CHAR */
head_char_sect : OPEN DELIM TAG_CHAR mand_line_item 
                 { /* Don't allow to continue if conversion context couldn't
		      be opened */
		   if (open_conv_to_internal($4) == 0) HANDLE_ERROR;
		   $<ctxt>$ = start_element(ELT_HEAD_CHAR,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(CHAR, $1, $<ctxt>$)
		 }
                 head_char_subs
		 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_CHAR, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
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
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(VERS, $1, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_CHAR_VERS,
				    PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		      }
                    ;

/* HEAD.LANG */
head_lang_sect : OPEN DELIM TAG_LANG mand_line_item   
                 { $<ctxt>$ = start_element(ELT_HEAD_LANG,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(LANG, $1, $<ctxt>$)
		 }
                 no_std_subs
                 { CHECK0 }
                 CLOSE
                 { end_element(ELT_HEAD_LANG, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;
/* HEAD.PLAC */
head_plac_sect : OPEN DELIM TAG_PLAC
                 { $<ctxt>$ = start_element(ELT_HEAD_PLAC,
					    PARENT, $1, $3, NULL,
					    GEDCOM_MAKE_NULL(val1));
		   START(PLAC, $1, $<ctxt>$)
		 }
                 head_plac_subs
		 { CHECK1(FORM) }
                 CLOSE
                 { end_element(ELT_HEAD_PLAC, PARENT, $<ctxt>4,
			       GEDCOM_MAKE_NULL(val1));
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
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(FORM, $1, $<ctxt>$)
		      }
                      no_std_subs
                      { CHECK0 }
                      CLOSE
                      { end_element(ELT_HEAD_PLAC_FORM,
				    PARENT, $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		      }
                    ;

/* HEAD.NOTE */
head_note_sect : OPEN DELIM TAG_NOTE mand_line_item 
                 { $<ctxt>$ = start_element(ELT_HEAD_NOTE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   reset_buffer(&concat_buffer);
		   safe_buf_append(&concat_buffer, $4);
		   START(NOTE, $1, $<ctxt>$)
		 }
                 head_note_subs
		 { CHECK0 }
                 CLOSE
                 { char* complete = get_buf_string(&concat_buffer);
		   end_element(ELT_HEAD_NOTE, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_STRING(val1, complete));
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
               { struct xref_value *xr = gedcom_parse_xref($3, XREF_DEFINED,
							   XREF_FAM);
	         if (xr == NULL) HANDLE_ERROR;
		 $<ctxt>$ = start_record(REC_FAM,
					 $1, GEDCOM_MAKE_XREF_PTR(val1, xr),
					 $5,
					 NULL, GEDCOM_MAKE_NULL(val2));
		 START(FAM, $1, $<ctxt>$) }
               fam_subs
	       { CHECK0 }
               CLOSE
               { end_record(REC_FAM, $<ctxt>6, GEDCOM_MAKE_NULL(val1)); }
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
                { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							    XREF_INDI);
		  if (xr == NULL) HANDLE_ERROR;
		  $<ctxt>$ = start_element(ELT_FAM_HUSB,
					   PARENT, $1, $3, $4, 
					   GEDCOM_MAKE_XREF_PTR(val1, xr));
		  START(HUSB, $1, $<ctxt>$)
		}
                no_std_subs
                { CHECK0 }
                CLOSE
                { end_element(ELT_FAM_HUSB, PARENT, $<ctxt>5,
			      GEDCOM_MAKE_NULL(val1));
		}
              ;

/* FAM.WIFE */
fam_wife_sect : OPEN DELIM TAG_WIFE mand_pointer 
                { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							    XREF_INDI);
		  if (xr == NULL) HANDLE_ERROR;
		  $<ctxt>$ = start_element(ELT_FAM_WIFE,
					   PARENT, $1, $3, $4, 
					   GEDCOM_MAKE_XREF_PTR(val1, xr));
		  START(WIFE, $1, $<ctxt>$)
		}
                no_std_subs
                { CHECK0 }
                CLOSE
                { end_element(ELT_FAM_WIFE, PARENT, $<ctxt>5,
			      GEDCOM_MAKE_NULL(val1));
		}
              ;

/* FAM.CHIL */
fam_chil_sect : OPEN DELIM TAG_CHIL mand_pointer
                { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							    XREF_INDI);
		  if (xr == NULL) HANDLE_ERROR;
		  $<ctxt>$ = start_element(ELT_FAM_CHIL,
					   PARENT, $1, $3, $4, 
					   GEDCOM_MAKE_XREF_PTR(val1, xr));
		  START(CHIL, $1, $<ctxt>$) 
		} 
		no_std_subs 
		{ CHECK0 } 
		CLOSE
                { end_element(ELT_FAM_CHIL, PARENT, $<ctxt>5,
			      GEDCOM_MAKE_NULL(val1));
		}
              ;

/* FAM.NCHI */
fam_nchi_sect : OPEN DELIM TAG_NCHI mand_line_item    
                { $<ctxt>$ = start_element(ELT_FAM_NCHI,
					   PARENT, $1, $3, $4, 
					   GEDCOM_MAKE_STRING(val1, $4));
		  START(NCHI, $1, $<ctxt>$)  
		}  
		no_std_subs  
		{ CHECK0 }  
		CLOSE
                { end_element(ELT_FAM_NCHI, PARENT, $<ctxt>5,
			      GEDCOM_MAKE_NULL(val1));
		}
              ;

/* FAM.SUBM */
fam_subm_sect : OPEN DELIM TAG_SUBM mand_pointer
                { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							    XREF_SUBM);
		  if (xr == NULL) HANDLE_ERROR;
		  $<ctxt>$ = start_element(ELT_FAM_SUBM,
					   PARENT, $1, $3, $4, 
					   GEDCOM_MAKE_XREF_PTR(val1, xr));
		  START(SUBM, $1, $<ctxt>$)   
		}   
		no_std_subs   
		{ CHECK0 }   
		CLOSE
                { end_element(ELT_FAM_SUBM, PARENT, $<ctxt>5,
			      GEDCOM_MAKE_NULL(val1));
		}
              ;

/*********************************************************************/
/**** Individual record                                           ****/
/*********************************************************************/
indiv_rec   : OPEN DELIM POINTER DELIM TAG_INDI
              { struct xref_value *xr = gedcom_parse_xref($3, XREF_DEFINED,
							  XREF_INDI);
	        if (xr == NULL) HANDLE_ERROR;
		$<ctxt>$ = start_record(REC_INDI,
					$1, GEDCOM_MAKE_XREF_PTR(val1, xr), $5,
					NULL, GEDCOM_MAKE_NULL(val2));
		START(INDI, $1, $<ctxt>$) }
              indi_subs
	      { CHECK0 }
              CLOSE
              { end_record(REC_INDI, $<ctxt>6, GEDCOM_MAKE_NULL(val1));
	        if (compat_mode(C_NO_SLGC_FAMC))
		  compat_generate_slgc_famc_fam();
	      }
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
	    | indi_addr_sect { if (!compat_mode(C_INDI_ADDR))
	                          INVALID_TAG("ADDR");
	                      }
	    | no_std_sub
            ;

/* INDI.RESN */
indi_resn_sect : OPEN DELIM TAG_RESN mand_line_item     
                 { $<ctxt>$ = start_element(ELT_INDI_RESN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(RESN, $1, $<ctxt>$)    
		 }    
		 no_std_subs     
		 { CHECK0 }     
		 CLOSE     
		 { end_element(ELT_INDI_RESN, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* INDI.SEX */
indi_sex_sect  : OPEN DELIM TAG_SEX mand_line_item     
                 { $<ctxt>$ = start_element(ELT_INDI_SEX,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(SEX, $1, $<ctxt>$)     
		 }     
		 no_std_subs     
		 { CHECK0 }     
		 CLOSE     
		 { end_element(ELT_INDI_SEX, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* INDI.SUBM */
indi_subm_sect : OPEN DELIM TAG_SUBM mand_pointer 
                 { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							     XREF_SUBM);
		   if (xr == NULL) HANDLE_ERROR;
		   $<ctxt>$ = start_element(ELT_INDI_SUBM,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_XREF_PTR(val1, xr));
		   START(SUBM, $1, $<ctxt>$)      
		 }      
		 no_std_subs      
		 { CHECK0 }      
		 CLOSE      
		 { end_element(ELT_INDI_SUBM, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* INDI.ALIA */
indi_alia_sect : OPEN DELIM TAG_ALIA mand_pointer
                 { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							     XREF_INDI);
		   if (xr == NULL) HANDLE_ERROR;
		   $<ctxt>$ = start_element(ELT_INDI_ALIA,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_XREF_PTR(val1, xr));
		   START(ALIA, $1, $<ctxt>$)       
		 }       
		 no_std_subs       
		 { CHECK0 }       
		 CLOSE       
		 { end_element(ELT_INDI_ALIA, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* INDI.ANCI */
indi_anci_sect : OPEN DELIM TAG_ANCI mand_pointer
                 { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							     XREF_SUBM);
		   if (xr == NULL) HANDLE_ERROR;
		   $<ctxt>$ = start_element(ELT_INDI_ANCI,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_XREF_PTR(val1, xr));
		   START(ANCI, $1, $<ctxt>$)        
		 }        
		 no_std_subs        
		 { CHECK0 }        
		 CLOSE        
		 { end_element(ELT_INDI_ANCI, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* INDI.DESI */
indi_desi_sect : OPEN DELIM TAG_DESI mand_pointer
                 { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							     XREF_SUBM);
		   if (xr == NULL) HANDLE_ERROR;
		   $<ctxt>$ = start_element(ELT_INDI_DESI,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_XREF_PTR(val1, xr));
		   START(DESI, $1, $<ctxt>$)         
		 }         
		 no_std_subs         
		 { CHECK0 }         
		 CLOSE         
		 { end_element(ELT_INDI_DESI, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* INDI.RFN */
indi_rfn_sect  : OPEN DELIM TAG_RFN mand_line_item     
                 { $<ctxt>$ = start_element(ELT_INDI_RFN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(RFN, $1, $<ctxt>$)          
		 }          
		 no_std_subs          
		 { CHECK0 }          
		 CLOSE          
		 { end_element(ELT_INDI_RFN, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* INDI.AFN */
indi_afn_sect  : OPEN DELIM TAG_AFN mand_line_item      
                 { $<ctxt>$ = start_element(ELT_INDI_AFN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(AFN, $1, $<ctxt>$)           
		 }           
		 no_std_subs           
		 { CHECK0 }           
		 CLOSE           
		 { end_element(ELT_INDI_AFN, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* INDI.ADDR (Only for compatibility) */
indi_addr_sect : OPEN DELIM TAG_ADDR opt_line_item
                  { if (compat_mode(C_INDI_ADDR)) {
		      Gedcom_ctxt par = compat_generate_resi_start(PARENT);
		      START(RESI, $1, par);
		      $<ctxt>$
			= start_element(ELT_SUB_ADDR,
					par, $1 + 1, $3, $4,
					GEDCOM_MAKE_NULL_OR_STRING(val2, $4));
		      reset_buffer(&concat_buffer);
		      safe_buf_append(&concat_buffer, $4);
		      START(ADDR, $1 + 1, $<ctxt>$);
		    }
		  else { START(ADDR, $1, NULL) }
		  }
                  ftree_addr_subs
                  { CHECK0 }
                  CLOSE
                  { if (compat_mode(C_INDI_ADDR)) {
		      Gedcom_ctxt par = PARENT;
		      char* complete = get_buf_string(&concat_buffer);
		      end_element(ELT_SUB_ADDR, par, $<ctxt>5,
				  GEDCOM_MAKE_STRING(val1, complete));
		      CHECK0;
		      compat_generate_resi_end(PARENT, par);
		    } 
		  }
                ;

ftree_addr_subs : /* empty */
                | ftree_addr_subs ftree_addr_sub
                ;

ftree_addr_sub  : continuation_sub
                | ftree_addr_phon_sect
                | no_std_sub
                ;

ftree_addr_phon_sect : OPEN DELIM TAG_PHON mand_line_item              
                       { $<ctxt>$
			   = start_element(ELT_SUB_PHON,
					   GRANDPARENT(1), $1, $3, $4, 
					   GEDCOM_MAKE_STRING(val1, $4));
	                 START(PHON, $1, $<ctxt>$)               
		       }               
                       no_std_subs               
                       { CHECK0 }               
                       CLOSE               
                       { end_element(ELT_SUB_PHON, GRANDPARENT(1),
				     $<ctxt>5, GEDCOM_MAKE_NULL(val1));
	               }
            ;

/*********************************************************************/
/**** Multimedia record                                           ****/
/*********************************************************************/
multim_rec  : OPEN DELIM POINTER DELIM TAG_OBJE
              { struct xref_value *xr = gedcom_parse_xref($3, XREF_DEFINED,
							  XREF_OBJE);
	        if (xr == NULL) HANDLE_ERROR;
		$<ctxt>$ = start_record(REC_OBJE,
					$1, GEDCOM_MAKE_XREF_PTR(val1, xr), $5,
					NULL, GEDCOM_MAKE_NULL(val2));
		START(OBJE, $1, $<ctxt>$) }
              obje_subs
	      { CHECK2(FORM, BLOB) }
              CLOSE
              { end_record(REC_OBJE, $<ctxt>6, GEDCOM_MAKE_NULL(val1)); }
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
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(FORM, $1, $<ctxt>$)            
		 }            
		 no_std_subs            
		 { CHECK0 }            
		 CLOSE            
		 { end_element(ELT_OBJE_FORM, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* OBJE.TITL */
obje_titl_sect : OPEN DELIM TAG_TITL mand_line_item       
                 { $<ctxt>$ = start_element(ELT_OBJE_TITL,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(TITL, $1, $<ctxt>$)             
		 }             
		 no_std_subs             
		 { CHECK0 }             
		 CLOSE             
		 { end_element(ELT_OBJE_TITL, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* OBJE.BLOB */
obje_blob_sect : OPEN DELIM TAG_BLOB
                 { $<ctxt>$ = start_element(ELT_OBJE_BLOB,
					    PARENT, $1, $3, NULL,
					    GEDCOM_MAKE_NULL(val1));
		   reset_buffer(&concat_buffer);
		   START(BLOB, $1, $<ctxt>$)              
		 }
                 obje_blob_subs
		 { CHECK1(CONT) }
                 CLOSE              
                 { char* complete = get_buf_string(&concat_buffer);
		   end_element(ELT_OBJE_BLOB, PARENT, $<ctxt>4,
			       GEDCOM_MAKE_STRING(val1, complete));
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
						 GEDCOM_MAKE_STRING(val1, $4));
		        safe_buf_append(&concat_buffer, $4);
		        START(CONT, $1, $<ctxt>$)               
		      }                
		      no_std_subs                
		      { CHECK0 }                
		      CLOSE                
		      { end_element(ELT_OBJE_BLOB_CONT, PARENT,
				    $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		      }
                    ;

/* OBJE.OBJE */
obje_obje_sect : OPEN DELIM TAG_OBJE mand_pointer 
                 { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							     XREF_OBJE);
		   if (xr == NULL) HANDLE_ERROR;
		   $<ctxt>$ = start_element(ELT_OBJE_OBJE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_XREF_PTR(val1, xr));
		   START(OBJE, $1, $<ctxt>$)  
                 }  
                 no_std_subs  
                 { CHECK0 }  
                 CLOSE  
                 { end_element(ELT_OBJE_OBJE, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/*********************************************************************/
/**** Note record                                                 ****/
/*********************************************************************/
note_rec    : OPEN DELIM POINTER DELIM TAG_NOTE note_line_item
              { struct xref_value *xr = gedcom_parse_xref($3, XREF_DEFINED,
							  XREF_NOTE);
	        if (xr == NULL) HANDLE_ERROR;
		$<ctxt>$ = start_record(REC_NOTE,
					$1, GEDCOM_MAKE_XREF_PTR(val1, xr), $5,
					$6, GEDCOM_MAKE_STRING(val2, $6));
		reset_buffer(&concat_buffer);
		safe_buf_append(&concat_buffer, $6);
		START(NOTE, $1, $<ctxt>$) }
              note_subs
	      { CHECK0 }
              CLOSE
              { char* complete = get_buf_string(&concat_buffer);
		end_record(REC_NOTE, $<ctxt>7,
			   GEDCOM_MAKE_STRING(val1, complete)); }
            ;

note_line_item : /* empty */
                   { if (!compat_mode(C_NOTE_NO_VALUE)) {
		       gedcom_error(_("Missing value")); YYERROR;
		     }
		     else {
		       $$ = VALUE_IF_MISSING;
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
            | note_note_sect  { if (!compat_mode(C_NOTE_NOTE))
	                          INVALID_TAG("NOTE");
                              }
            | no_std_sub
            ;

/* Same actions as cont_sect, for compatibility */
note_note_sect : OPEN DELIM TAG_NOTE opt_line_item
            { $3.string = "CONT";
	      $3.value  = TAG_CONT;
	      $<ctxt>$ = start_element(ELT_SUB_CONT,
				       PARENT, $1, $3, $4, 
				       GEDCOM_MAKE_NULL_OR_STRING(val1, $4));
	      SAFE_BUF_ADDCHAR(&concat_buffer, '\n');
	      if (GEDCOM_IS_STRING(&val1))
	        safe_buf_append(&concat_buffer, $4);
	      START(CONT, $1, $<ctxt>$)  
            }  
            no_std_subs  
            { CHECK0 }  
            CLOSE  
            { end_element(ELT_SUB_CONT, PARENT, $<ctxt>5,
			  GEDCOM_MAKE_NULL(val1));
	    }
            ;

/*********************************************************************/
/**** Repository record                                           ****/
/*********************************************************************/
repos_rec   : OPEN DELIM POINTER DELIM TAG_REPO
              { struct xref_value *xr = gedcom_parse_xref($3, XREF_DEFINED,
							  XREF_REPO);
	        if (xr == NULL) HANDLE_ERROR;
		$<ctxt>$ = start_record(REC_REPO,
					$1, GEDCOM_MAKE_XREF_PTR(val1, xr), $5,
					NULL, GEDCOM_MAKE_NULL(val2));
		START(REPO, $1, $<ctxt>$) }
              repo_subs
	      { CHECK0 }
              CLOSE
              { end_record(REC_REPO, $<ctxt>6, GEDCOM_MAKE_NULL(val1)); }
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
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(NAME, $1, $<ctxt>$)          
                 }          
                 no_std_subs          
                 { CHECK0 }          
                 CLOSE          
                 { end_element(ELT_REPO_NAME, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/*********************************************************************/
/**** Source record                                               ****/
/*********************************************************************/
source_rec  : OPEN DELIM POINTER DELIM TAG_SOUR
              { struct xref_value *xr = gedcom_parse_xref($3, XREF_DEFINED,
							  XREF_SOUR);
	        if (xr == NULL) HANDLE_ERROR;
		$<ctxt>$ = start_record(REC_SOUR,
					$1, GEDCOM_MAKE_XREF_PTR(val1, xr), $5,
					NULL, GEDCOM_MAKE_NULL(val2));
		START(SOUR, $1, $<ctxt>$) }
              sour_subs
	      { CHECK0 }
              CLOSE
              { end_record(REC_SOUR, $<ctxt>6, GEDCOM_MAKE_NULL(val1)); }
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
            | sour_type_sect  { if (!compat_mode(C_NONSTD_SOUR_TAGS))
	                          INVALID_TAG("TYPE");
	                        OCCUR2(TYPE, 0, 1) }
            | sour_file_sect  { if (!compat_mode(C_NONSTD_SOUR_TAGS))
	                          INVALID_TAG("FILE");
	                        OCCUR2(FILE, 0, 1) }
            | sour_plac_sect  { if (!compat_mode(C_NONSTD_SOUR_TAGS))
	                          INVALID_TAG("PLAC");
	                        OCCUR2(PLAC, 0, 1) }
            | sour_date_sect  { if (!compat_mode(C_NONSTD_SOUR_TAGS))
	                          INVALID_TAG("DATE");
	                        OCCUR2(DATE, 0, 1) }
            | sour_medi_sect  { if (!compat_mode(C_NONSTD_SOUR_TAGS))
	                          INVALID_TAG("MEDI");
	                        OCCUR2(MEDI, 0, 1) }
            | sour_page_sect  { if (!compat_mode(C_NONSTD_SOUR_TAGS))
	                          INVALID_TAG("PAGE");
	                        OCCUR2(PAGE, 0, 1) }
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
					    PARENT, $1, $3, NULL,
					    GEDCOM_MAKE_NULL(val1));
		   START(DATA, $1, $<ctxt>$) 
                 }
                 sour_data_subs
		 { CHECK0 }
                 CLOSE 
                 { end_element(ELT_SOUR_DATA, PARENT, $<ctxt>4,
			       GEDCOM_MAKE_NULL(val1));
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
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(EVEN, $1, $<ctxt>$)  
                      }
                      sour_data_even_subs
		      { CHECK0 }
                      CLOSE  
                      { end_element(ELT_SOUR_DATA_EVEN, PARENT,
				    $<ctxt>5, GEDCOM_MAKE_NULL(val1));
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
			     $<ctxt>$
			       = start_element(ELT_SOUR_DATA_EVEN_DATE,
					       PARENT, $1, $3, $4, 
					       GEDCOM_MAKE_DATE(val1, dv));
		             START(DATE, $1, $<ctxt>$)           
                           }           
                           no_std_subs           
                           { CHECK0 }           
                           CLOSE           
                           { end_element(ELT_SOUR_DATA_EVEN_DATE, PARENT,
					 $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		           }
                         ;

sour_data_even_plac_sect : OPEN DELIM TAG_PLAC mand_line_item          
                           { $<ctxt>$
			       = start_element(ELT_SOUR_DATA_EVEN_PLAC,
					       PARENT, $1, $3, $4, 
					       GEDCOM_MAKE_STRING(val1, $4));
		             START(PLAC, $1, $<ctxt>$)           
                           }           
                           no_std_subs           
                           { CHECK0 }           
                           CLOSE           
                           { end_element(ELT_SOUR_DATA_EVEN_PLAC, PARENT,
					 $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		           }
                         ;

sour_data_agnc_sect : OPEN DELIM TAG_AGNC mand_line_item          
                      { $<ctxt>$ = start_element(ELT_SOUR_DATA_AGNC,
						 PARENT, $1, $3, $4, 
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(AGNC, $1, $<ctxt>$)           
                      }           
                      no_std_subs           
                      { CHECK0 }           
                      CLOSE           
                      { end_element(ELT_SOUR_DATA_AGNC, PARENT,
				    $<ctxt>5, GEDCOM_MAKE_NULL(val1));
		      }
                    ;

/* SOUR.AUTH */
sour_auth_sect : OPEN DELIM TAG_AUTH mand_line_item
                 { $<ctxt>$ = start_element(ELT_SOUR_AUTH,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   reset_buffer(&concat_buffer);
		   safe_buf_append(&concat_buffer, $4);
		   START(AUTH, $1, $<ctxt>$) 
                 }
                 sour_auth_subs
		 { CHECK0 }
                 CLOSE 
                 { char* complete = get_buf_string(&concat_buffer);
		   end_element(ELT_SOUR_AUTH, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_STRING(val1, complete));
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
					    GEDCOM_MAKE_STRING(val1, $4));
		   reset_buffer(&concat_buffer);
		   safe_buf_append(&concat_buffer, $4);
		   START(TITL, $1, $<ctxt>$)   
                 }
                 sour_titl_subs 
		 { CHECK0 }
                 CLOSE   
                 { char* complete = get_buf_string(&concat_buffer);
		   end_element(ELT_SOUR_TITL, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_STRING(val1, complete));
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
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(ABBR, $1, $<ctxt>$)            
                 }            
                 no_std_subs            
                 { CHECK0 }            
                 CLOSE            
                 { end_element(ELT_SOUR_ABBR, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SOUR.PUBL */
sour_publ_sect : OPEN DELIM TAG_PUBL mand_line_item  
                 { $<ctxt>$ = start_element(ELT_SOUR_PUBL,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   reset_buffer(&concat_buffer);
		   safe_buf_append(&concat_buffer, $4);
		   START(PUBL, $1, $<ctxt>$)            
                 }
                 sour_publ_subs  
		 { CHECK0 }
                 CLOSE            
                 { char* complete = get_buf_string(&concat_buffer);
		   end_element(ELT_SOUR_PUBL, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_STRING(val1, complete));
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
					    GEDCOM_MAKE_STRING(val1, $4));
		   reset_buffer(&concat_buffer);
		   safe_buf_append(&concat_buffer, $4);
		   START(TEXT, $1, $<ctxt>$)    
                 }
                 sour_text_subs  
		 { CHECK0 }
                 CLOSE    
                 { char* complete = get_buf_string(&concat_buffer);
		   end_element(ELT_SOUR_TEXT, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_STRING(val1, complete));
		 }
               ;

sour_text_subs : /* empty */
               | sour_text_subs sour_text_sub
               ;

sour_text_sub  : continuation_sub  /* 0:M */
               | no_std_sub
               ;

/* Only for compatibility */
sour_type_sect : OPEN DELIM TAG_TYPE opt_line_item  
                 { if (compat_mode(C_NONSTD_SOUR_TAGS)) {
		     $<ctxt>$ =
		       compat_generate_nonstd_sour_start(PARENT, $1, $3, $4,
							 &usertag_buffer);
		   }
                 }
                 CLOSE            
                 { if (compat_mode(C_NONSTD_SOUR_TAGS))
		     compat_generate_nonstd_sour_end(PARENT, $<ctxt>5);
		 }
               ;

/* Only for compatibility */
sour_file_sect : OPEN DELIM TAG_FILE opt_line_item  
                 { if (compat_mode(C_NONSTD_SOUR_TAGS)) {
		     $<ctxt>$ =
		       compat_generate_nonstd_sour_start(PARENT, $1, $3, $4,
							 &usertag_buffer);
		   }
                 }
                 CLOSE            
                 { if (compat_mode(C_NONSTD_SOUR_TAGS))
		     compat_generate_nonstd_sour_end(PARENT, $<ctxt>5);
		 }
               ;

/* Only for compatibility */
sour_plac_sect : OPEN DELIM TAG_PLAC opt_line_item  
                 { if (compat_mode(C_NONSTD_SOUR_TAGS)) {
		     $<ctxt>$ =
		       compat_generate_nonstd_sour_start(PARENT, $1, $3, $4,
							 &usertag_buffer);
		   }
                 }
                 CLOSE            
                 { if (compat_mode(C_NONSTD_SOUR_TAGS))
		     compat_generate_nonstd_sour_end(PARENT, $<ctxt>5);
		 }
               ;

/* Only for compatibility */
sour_date_sect : OPEN DELIM TAG_DATE opt_line_item  
                 { if (compat_mode(C_NONSTD_SOUR_TAGS)) {
		     $<ctxt>$ =
		       compat_generate_nonstd_sour_start(PARENT, $1, $3, $4,
							 &usertag_buffer);
		   }
                 }
                 CLOSE            
                 { if (compat_mode(C_NONSTD_SOUR_TAGS))
		     compat_generate_nonstd_sour_end(PARENT, $<ctxt>5);
		 }
               ;

/* Only for compatibility */
sour_medi_sect : OPEN DELIM TAG_MEDI opt_line_item  
                 { if (compat_mode(C_NONSTD_SOUR_TAGS)) {
		     $<ctxt>$ =
		       compat_generate_nonstd_sour_start(PARENT, $1, $3, $4,
							 &usertag_buffer);
		   }
                 }
                 CLOSE            
                 { if (compat_mode(C_NONSTD_SOUR_TAGS))
		     compat_generate_nonstd_sour_end(PARENT, $<ctxt>5);
		 }
               ;

/* Only for compatibility */
sour_page_sect : OPEN DELIM TAG_PAGE opt_line_item  
                 { if (compat_mode(C_NONSTD_SOUR_TAGS)) {
		     $<ctxt>$ =
		       compat_generate_nonstd_sour_start(PARENT, $1, $3, $4,
							 &usertag_buffer);
		   }
                 }
                 CLOSE            
                 { if (compat_mode(C_NONSTD_SOUR_TAGS))
		     compat_generate_nonstd_sour_end(PARENT, $<ctxt>5);
		 }
               ;

/*********************************************************************/
/**** Submission record                                           ****/
/*********************************************************************/
submis_rec  : OPEN DELIM POINTER DELIM TAG_SUBN    
              { struct xref_value *xr = gedcom_parse_xref($3, XREF_DEFINED,
							  XREF_SUBN);
	        if (xr == NULL) HANDLE_ERROR;
		$<ctxt>$ = start_record(REC_SUBN,
					$1, GEDCOM_MAKE_XREF_PTR(val1, xr), $5,
					NULL, GEDCOM_MAKE_NULL(val2));
		START(SUBN, $1, $<ctxt>$) }
              subn_subs
	      { CHECK0 }
              CLOSE
              { end_record(REC_SUBN, $<ctxt>6, GEDCOM_MAKE_NULL(val1)); }
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
                 { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							     XREF_SUBM);
		   if (xr == NULL) HANDLE_ERROR;
		   $<ctxt>$ = start_element(ELT_SUBN_SUBM,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_XREF_PTR(val1, xr));
		   START(SUBM, $1, $<ctxt>$) 
                 } 
                 no_std_subs 
                 { CHECK0 } 
                 CLOSE 
                 { end_element(ELT_SUBN_SUBM, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SUBN.FAMF */
subn_famf_sect : OPEN DELIM TAG_FAMF mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_FAMF,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(FAMF, $1, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_FAMF, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SUBN.TEMP */
subn_temp_sect : OPEN DELIM TAG_TEMP mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_TEMP,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(TEMP, $1, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_TEMP, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SUBN.ANCE */
subn_ance_sect : OPEN DELIM TAG_ANCE mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_ANCE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(ANCE, $1, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_ANCE, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SUBN.DESC */
subn_desc_sect : OPEN DELIM TAG_DESC mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_DESC,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(DESC, $1, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_DESC, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SUBN.ORDI */
subn_ordi_sect : OPEN DELIM TAG_ORDI mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_ORDI,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(ORDI, $1, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_ORDI, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SUBN.RIN */
subn_rin_sect  : OPEN DELIM TAG_RIN mand_line_item            
                 { $<ctxt>$ = start_element(ELT_SUBN_RIN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(RIN, $1, $<ctxt>$)             
                 }             
                 no_std_subs             
                 { CHECK0 }             
                 CLOSE             
                 { end_element(ELT_SUBN_RIN, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/*********************************************************************/
/**** Submitter record                                            ****/
/*********************************************************************/
submit_rec : OPEN DELIM POINTER DELIM TAG_SUBM    
             { struct xref_value *xr = gedcom_parse_xref($3, XREF_DEFINED,
							 XREF_SUBM);
	       if (xr == NULL) HANDLE_ERROR;
	       $<ctxt>$ = start_record(REC_SUBM,
				       $1, GEDCOM_MAKE_XREF_PTR(val1, xr), $5,
				       NULL, GEDCOM_MAKE_NULL(val2));
	       START(SUBM, $1, $<ctxt>$) }
             subm_subs
	     { CHECK1(NAME) }
             CLOSE
             { end_record(REC_SUBM, $<ctxt>6, GEDCOM_MAKE_NULL(val1));
	       if (compat_mode(C_SUBM_CTRY))
		 compat_free_ctry_parent_context();
	     }
           ;

subm_subs  : /* empty */
           | subm_subs subm_sub
           ;

subm_sub   : subm_name_sect  { OCCUR2(NAME, 1, 1) }
           | addr_struc_sub  /* 0:1 */
           | multim_link_sub  /* 0:M */
           | subm_lang_sect  { OCCUR2(LANG, 0, 3) }
           | subm_rfn_sect  { OCCUR2(RFN, 0, 1) }
           | subm_rin_sect  { OCCUR2(RIN, 0, 1) }
           | change_date_sub  /* 0:1 */
	   | subm_ctry_sect  { if (!compat_mode(C_SUBM_CTRY))
	                          INVALID_TAG("CTRY");
	                       OCCUR2(CTRY, 0, 1) }
           | no_std_sub
           ;

/* SUBM.NAME */
subm_name_sect : OPEN DELIM TAG_NAME mand_line_item             
                 { $<ctxt>$ = start_element(ELT_SUBM_NAME,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(NAME, $1, $<ctxt>$)              
                 }              
                 no_std_subs              
                 { CHECK0 }              
                 CLOSE              
                 { end_element(ELT_SUBM_NAME, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SUBM.LANG */
subm_lang_sect : OPEN DELIM TAG_LANG mand_line_item             
                 { $<ctxt>$ = start_element(ELT_SUBM_LANG,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(LANG, $1, $<ctxt>$)              
                 }              
                 no_std_subs              
                 { CHECK0 }              
                 CLOSE              
                 { end_element(ELT_SUBM_LANG, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SUBM.RFN */
subm_rfn_sect  : OPEN DELIM TAG_RFN mand_line_item             
                 { $<ctxt>$ = start_element(ELT_SUBM_RFN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(RFN, $1, $<ctxt>$)              
                 }              
                 no_std_subs              
                 { CHECK0 }              
                 CLOSE              
                 { end_element(ELT_SUBM_RFN, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SUBM.RIN */
subm_rin_sect  : OPEN DELIM TAG_RIN mand_line_item             
                 { $<ctxt>$ = start_element(ELT_SUBM_RIN,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(RIN, $1, $<ctxt>$)              
                 }              
                 no_std_subs              
                 { CHECK0 }              
                 CLOSE              
                 { end_element(ELT_SUBM_RIN, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* SUBM.CTRY (Only for compatibility) */
subm_ctry_sect : OPEN DELIM TAG_CTRY opt_line_item
                 { if (compat_mode(C_SUBM_CTRY)) {
		     $<ctxt>$ = compat_generate_addr_ctry_start($1, $3, $4);
                   }
                 }
                 CLOSE
                 { if (compat_mode (C_SUBM_CTRY)) {
		     compat_generate_addr_ctry_end($<ctxt>5);
		   }
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
					 GEDCOM_MAKE_STRING(val1, $4));
	        reset_buffer(&concat_buffer);
		safe_buf_append(&concat_buffer, $4);
	        START(ADDR, $1, $<ctxt>$);
		if (compat_mode(C_SUBM_CTRY))
		  compat_save_ctry_parent_context($<ctxt>$);
              }
              addr_subs
	      { CHECK0 }
              CLOSE  
              { char* complete = get_buf_string(&concat_buffer);
		end_element(ELT_SUB_ADDR, PARENT, $<ctxt>5,
			    GEDCOM_MAKE_STRING(val1, complete));
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
					    GEDCOM_MAKE_STRING(val1, $4));
		   SAFE_BUF_ADDCHAR(&concat_buffer, '\n');
		   safe_buf_append(&concat_buffer, $4);
		   START(CONT, $1, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_CONT, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;
addr_adr1_sect : OPEN DELIM TAG_ADR1 mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_ADR1,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(ADR1, $1, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_ADR1, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;
addr_adr2_sect : OPEN DELIM TAG_ADR2 mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_ADR2,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(ADR2, $1, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_ADR2, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;
addr_city_sect : OPEN DELIM TAG_CITY mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_CITY,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(CITY, $1, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_CITY, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;
addr_stae_sect : OPEN DELIM TAG_STAE mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_STAE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(STAE, $1, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_STAE, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;
addr_post_sect : OPEN DELIM TAG_POST mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_POST,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(POST, $1, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_POST, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;
addr_ctry_sect : OPEN DELIM TAG_CTRY mand_line_item              
                 { $<ctxt>$ = start_element(ELT_SUB_ADDR_CTRY,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(CTRY, $1, $<ctxt>$)               
                 }               
                 no_std_subs               
                 { CHECK0 }               
                 CLOSE               
                 { end_element(ELT_SUB_ADDR_CTRY, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

phon_sect   : OPEN DELIM TAG_PHON mand_line_item              
              { $<ctxt>$ = start_element(ELT_SUB_PHON,
					 PARENT, $1, $3, $4, 
					 GEDCOM_MAKE_STRING(val1, $4));
	        START(PHON, $1, $<ctxt>$)               
              }               
              no_std_subs               
              { CHECK0 }               
              CLOSE               
              { end_element(ELT_SUB_PHON, PARENT, $<ctxt>5,
			    GEDCOM_MAKE_NULL(val1));
	      }
            ;

/* ASSOCIATION STRUCTURE */
assoc_struc_sub : asso_sect /* 0:M */
                ;

asso_sect : OPEN DELIM TAG_ASSO mand_pointer
            { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							XREF_ANY);
	      if (xr == NULL) HANDLE_ERROR;
	      $<ctxt>$ = start_element(ELT_SUB_ASSO,
				       PARENT, $1, $3, $4, 
				       GEDCOM_MAKE_XREF_PTR(val1, xr));
	      START(ASSO, $1, $<ctxt>$) 
            }
            asso_subs
	    { CHECK2(TYPE,RELA) }
            CLOSE 
            { end_element(ELT_SUB_ASSO, PARENT, $<ctxt>5,
			  GEDCOM_MAKE_NULL(val1));
	    }
          ;

asso_subs : /* empty */
          | asso_type_sect  { OCCUR2(TYPE, 1, 1) }
          | asso_rela_sect  { OCCUR2(RELA, 1, 1) }
          | note_struc_sub
          | source_cit_sub
	  | no_std_sub
          ;

asso_type_sect : OPEN DELIM TAG_TYPE mand_line_item               
                 { $<ctxt>$ = start_element(ELT_SUB_ASSO_TYPE,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(TYPE, $1, $<ctxt>$)                
                 }                
                 no_std_subs                
                 { CHECK0 }                
                 CLOSE                
                 { end_element(ELT_SUB_ASSO_TYPE, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

asso_rela_sect : OPEN DELIM TAG_RELA mand_line_item               
                 { $<ctxt>$ = start_element(ELT_SUB_ASSO_RELA,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(RELA, $1, $<ctxt>$)                
                 }                
                 no_std_subs                
                 { CHECK0 }                
                 CLOSE                
                 { end_element(ELT_SUB_ASSO_RELA, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* CHANGE DATE */
change_date_sub : change_date_chan_sect  { OCCUR2(CHAN, 0, 1) }
                ;

change_date_chan_sect : OPEN DELIM TAG_CHAN
                        { $<ctxt>$ = start_element(ELT_SUB_CHAN,
						   PARENT, $1, $3, NULL, 
						   GEDCOM_MAKE_NULL(val1));
			  START(CHAN, $1, $<ctxt>$) 
                        }
                        change_date_chan_subs
			{ CHECK1(DATE) }
                        CLOSE 
                        { end_element(ELT_SUB_CHAN, PARENT, $<ctxt>4,
				      GEDCOM_MAKE_NULL(val1));
			}
                      ;

change_date_chan_subs : /* empty */
                      | change_date_chan_subs change_date_chan_sub
                      ;

change_date_chan_sub  : change_date_date_sect  { OCCUR2(DATE, 1, 1) }
                      | note_struc_sub
		      | no_std_sub
                      ;

change_date_date_sect : OPEN DELIM TAG_DATE mand_line_item 
                        { struct date_value dv = gedcom_parse_date($4);
			  $<ctxt>$ = start_element(ELT_SUB_CHAN_DATE,
						   PARENT, $1, $3, $4, 
						   GEDCOM_MAKE_DATE(val1, dv));
			  START(DATE, $1, $<ctxt>$) }
                        change_date_date_subs
			{ CHECK0 }
                        CLOSE 
			{ end_element(ELT_SUB_CHAN_DATE, PARENT, $<ctxt>5,
				      GEDCOM_MAKE_NULL(val1));
			}
                      ;

change_date_date_subs : /* empty */
                      | change_date_date_subs change_date_date_sub
                      ;

change_date_date_sub : change_date_date_time_sect  { OCCUR2(TIME, 0, 1) }
                     | no_std_sub
                     ;

change_date_date_time_sect : OPEN DELIM TAG_TIME mand_line_item
                             { $<ctxt>$
				 = start_element(ELT_SUB_CHAN_TIME,
						 PARENT, $1, $3, $4, 
						 GEDCOM_MAKE_STRING(val1, $4));
			       START(TIME, $1, $<ctxt>$) 
                             } 
                             no_std_subs 
                             { CHECK0 } 
                             CLOSE 
                             { end_element(ELT_SUB_CHAN_TIME, PARENT, $<ctxt>5,
					   GEDCOM_MAKE_NULL(val1));
			     }
                           ;

/* CHILD TO FAMILY LINK */
chi_fam_link_sub : famc_sect  /* 0:M */
                 ;

famc_sect : OPEN DELIM TAG_FAMC mand_pointer
            { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
							  XREF_FAM);
	      if (xr == NULL) HANDLE_ERROR;
	      $<ctxt>$ = start_element(ELT_SUB_FAMC,
				       PARENT, $1, $3, $4, 
				       GEDCOM_MAKE_XREF_PTR(val1, xr));
	      START(FAMC, $1, $<ctxt>$) 
            }
            famc_subs
	    { CHECK0 }
            CLOSE 
            { end_element(ELT_SUB_FAMC, PARENT, $<ctxt>5,
			  GEDCOM_MAKE_NULL(val1));
	    }
          ;

famc_subs : /* empty */
          | famc_subs famc_sub
          ;

famc_sub  : famc_pedi_sect  /* 0:M */
          | note_struc_sub
          | no_std_sub
          ;

famc_pedi_sect : OPEN DELIM TAG_PEDI mand_line_item 
                 { $<ctxt>$ = start_element(ELT_SUB_FAMC_PEDI,
					    PARENT, $1, $3, $4, 
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(PEDI, $1, $<ctxt>$)  
                 }  
                 no_std_subs  
                 { CHECK0 }  
                 CLOSE  
                 { end_element(ELT_SUB_FAMC_PEDI, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

/* CONTINUATION SUBSECTIONS */
continuation_sub : cont_sect  /* 0:M */
                 | conc_sect  /* 0:M */
                 ;

cont_sect : OPEN DELIM TAG_CONT opt_line_item 
            { $<ctxt>$ = start_element(ELT_SUB_CONT,
				       PARENT, $1, $3, $4, 
				       GEDCOM_MAKE_NULL_OR_STRING(val1, $4));
	      SAFE_BUF_ADDCHAR(&concat_buffer, '\n');
	      if (GEDCOM_IS_STRING(&val1))
	        safe_buf_append(&concat_buffer, $4);
	      START(CONT, $1, $<ctxt>$)  
            }  
            cont_conc_subs  
            { CHECK0 }  
            CLOSE  
            { end_element(ELT_SUB_CONT, PARENT, $<ctxt>5,
			  GEDCOM_MAKE_NULL(val1));
	    }
          ;

conc_sect : OPEN DELIM TAG_CONC mand_line_item 
            { $<ctxt>$ = start_element(ELT_SUB_CONC,
				       PARENT, $1, $3, $4, 
				       GEDCOM_MAKE_STRING(val1, $4));
	      if (compat_mode(C_CONC_NEEDS_SPACE)) {
		safe_buf_append(&concat_buffer, " ");
	      }
	      safe_buf_append(&concat_buffer, $4);
	      START(CONC, $1, $<ctxt>$)  
            }  
            cont_conc_subs  
            { CHECK0 }  
            CLOSE  
            { end_element(ELT_SUB_CONC, PARENT, $<ctxt>5,
			  GEDCOM_MAKE_NULL(val1));
	    }
          ; 

cont_conc_subs : /* empty */
               | cont_conc_subs cont_conc_sub
               ;

cont_conc_sub  : cont_conc_sour_sect { if (!compat_mode(C_NOTE_CONC_SOUR))
	                                 INVALID_TAG("SOUR");
	                               OCCUR2(SOUR, 0, 1) } 
               | no_std_sub
	       ;

/* Only for compatibility */
cont_conc_sour_sect : OPEN DELIM TAG_SOUR DELIM POINTER
                      { if (compat_mode(C_NOTE_CONC_SOUR)) {
		          $<ctxt>$
			    = compat_generate_note_sour_start(GRANDPARENT(1),
							      $1, $3, $5);
			  if ($<ctxt>$ == (void*)-1) HANDLE_ERROR;
                        }
                      }
                      no_std_subs
                      CLOSE
                      { if (compat_mode(C_NOTE_CONC_SOUR)) {
		          compat_generate_note_sour_end($<ctxt>6);
		        }
                      }
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
                         { $<ctxt>$
			     = start_element(ELT_SUB_EVT_TYPE,
					     PARENT, $1, $3, $4, 
					     GEDCOM_MAKE_STRING(val1, $4));
			   START(TYPE, $1, $<ctxt>$)  
                         }  
                         no_std_subs  
                         { CHECK0 }  
                         CLOSE  
                         { end_element(ELT_SUB_EVT_TYPE, PARENT, $<ctxt>5,
				       GEDCOM_MAKE_NULL(val1));
			 }
                       ;
event_detail_date_sect : OPEN DELIM TAG_DATE mand_line_item 
                         { struct date_value dv = gedcom_parse_date($4);
			   $<ctxt>$
			     = start_element(ELT_SUB_EVT_DATE,
					     PARENT, $1, $3, $4, 
					     GEDCOM_MAKE_DATE(val1, dv));
			   START(DATE, $1, $<ctxt>$)  
                         }  
                         no_std_subs  
                         { CHECK0 }  
                         CLOSE  
                         { end_element(ELT_SUB_EVT_DATE, PARENT, $<ctxt>5,
				       GEDCOM_MAKE_NULL(val1));
			 }
                       ;
event_detail_age_sect  : OPEN DELIM TAG_AGE mand_line_item 
                         { struct age_value age = gedcom_parse_age($4);
			   $<ctxt>$
			     = start_element(ELT_SUB_EVT_AGE,
					     PARENT, $1, $3, $4, 
					     GEDCOM_MAKE_AGE(val1, age));
			   START(AGE, $1, $<ctxt>$)  
                         }  
                         no_std_subs  
                         { CHECK0 }  
                         CLOSE  
                         { end_element(ELT_SUB_EVT_AGE, PARENT, $<ctxt>5,
				       GEDCOM_MAKE_NULL(val1));
			 }
                       ;
event_detail_agnc_sect : OPEN DELIM TAG_AGNC mand_line_item 
                         { $<ctxt>$
			     = start_element(ELT_SUB_EVT_AGNC,
					     PARENT, $1, $3, $4, 
					     GEDCOM_MAKE_STRING(val1, $4));
			   START(AGNC, $1, $<ctxt>$)  
                         }  
                         no_std_subs  
                         { CHECK0 }  
                         CLOSE  
                         { end_element(ELT_SUB_EVT_AGNC, PARENT, $<ctxt>5,
				       GEDCOM_MAKE_NULL(val1));
			 }
                       ;
event_detail_caus_sect : OPEN DELIM TAG_CAUS mand_line_item 
                         { $<ctxt>$
			     = start_element(ELT_SUB_EVT_CAUS,
					     PARENT, $1, $3, $4, 
					     GEDCOM_MAKE_STRING(val1, $4));
			   START(CAUS, $1, $<ctxt>$)  
                         }  
                         no_std_subs  
                         { CHECK0 }  
                         CLOSE  
                         { end_element(ELT_SUB_EVT_CAUS, PARENT, $<ctxt>5,
				       GEDCOM_MAKE_NULL(val1));
			 }
                       ;

/* FAMILY EVENT STRUCTURE */
fam_event_struc_sub : fam_event_sect
                    | fam_gen_even_sect  /* 0:M */
                    ;

fam_event_sect : OPEN DELIM fam_event_tag opt_value  
                 { $<ctxt>$
		     = start_element(ELT_SUB_FAM_EVT,
				     PARENT, $1, $3, $4,
				     GEDCOM_MAKE_NULL_OR_STRING(val1, $4));
		   START2($1, $<ctxt>$);
		 }
                 fam_event_subs
                 { CHECK0 }
                 CLOSE 
                 { end_element(ELT_SUB_FAM_EVT, PARENT, $<ctxt>5,
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;

fam_event_tag : TAG_ANUL { $$ = $1; START1(ANUL) }
              | TAG_CENS { $$ = $1; START1(CENS) }
              | TAG_DIV { $$ = $1; START1(DIV) }
              | TAG_DIVF { $$ = $1; START1(DIVF) }
              | TAG_ENGA { $$ = $1; START1(ENGA) }
              | TAG_MARR { $$ = $1; START1(MARR) }
              | TAG_MARB { $$ = $1; START1(MARB) }
              | TAG_MARC { $$ = $1; START1(MARC) }
              | TAG_MARL { $$ = $1; START1(MARL) }
              | TAG_MARS { $$ = $1; START1(MARS) }
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
                     { $<ctxt>$ = start_element(ELT_SUB_FAM_EVT_HUSB,
						PARENT, $1, $3, NULL,
						GEDCOM_MAKE_NULL(val1));
		       START(HUSB, $1, $<ctxt>$) 
                     }
                     fam_even_husb_subs
		     { CHECK1(AGE) }
                     CLOSE 
                     { end_element(ELT_SUB_FAM_EVT_HUSB, PARENT, $<ctxt>4,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;

fam_even_husb_subs : /* empty */
                   | fam_even_husb_subs fam_even_husb_sub
                   ;

fam_even_husb_sub : fam_even_age_sect  { OCCUR2(AGE, 1, 1) }
                  | no_std_sub
                  ;

fam_even_age_sect : OPEN DELIM TAG_AGE mand_line_item  
                    { struct age_value age = gedcom_parse_age($4);
		      $<ctxt>$ = start_element(ELT_SUB_FAM_EVT_AGE,
					       PARENT, $1, $3, $4,
					       GEDCOM_MAKE_AGE(val1, age));
		      START(AGE, $1, $<ctxt>$)   
                    }   
                    no_std_subs   
                    { CHECK0 }   
                    CLOSE   
                    { end_element(ELT_SUB_FAM_EVT_AGE, PARENT, $<ctxt>5,
				  GEDCOM_MAKE_NULL(val1));
		    }
                  ;

fam_even_wife_sect : OPEN DELIM TAG_WIFE
                     { $<ctxt>$ = start_element(ELT_SUB_FAM_EVT_WIFE,
						PARENT, $1, $3, NULL,
						GEDCOM_MAKE_NULL(val1));
		       START(WIFE, $1, $<ctxt>$) 
                     }
                     fam_even_husb_subs
		     { CHECK1(AGE) }
                     CLOSE 
                     { end_element(ELT_SUB_FAM_EVT_WIFE, PARENT, $<ctxt>4,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;

fam_gen_even_sect : OPEN DELIM TAG_EVEN
                    { $<ctxt>$ = start_element(ELT_SUB_FAM_EVT_EVEN,
						PARENT, $1, $3, NULL,
						GEDCOM_MAKE_NULL(val1));
		       START(EVEN, $1, $<ctxt>$) 
                    }
                    fam_gen_even_subs
		    { CHECK0 }
                    CLOSE 
                    { end_element(ELT_SUB_FAM_EVT_EVEN, PARENT, $<ctxt>4,
				  GEDCOM_MAKE_NULL(val1));
		    }
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
                  { $<ctxt>$ = start_element(ELT_SUB_IDENT_REFN,
					     PARENT, $1, $3, $4,
					     GEDCOM_MAKE_STRING(val1, $4));
		    START(REFN, $1, $<ctxt>$)  
                  }
                  ident_refn_subs
		  { CHECK0 }
                  CLOSE  
                  { end_element(ELT_SUB_IDENT_REFN, PARENT, $<ctxt>5,
				GEDCOM_MAKE_NULL(val1));
		  }
                ;

ident_refn_subs : /* empty */
                | ident_refn_subs ident_refn_sub
                ;

ident_refn_sub  : ident_refn_type_sect  { OCCUR2(TYPE, 0, 1) }
                | no_std_sub
                ;

ident_refn_type_sect : OPEN DELIM TAG_TYPE mand_line_item   
                       { $<ctxt>$
			   = start_element(ELT_SUB_IDENT_REFN_TYPE,
					   PARENT, $1, $3, $4,
					   GEDCOM_MAKE_STRING(val1, $4));
		         START(TYPE, $1, $<ctxt>$)    
                       }    
                       no_std_subs    
                       { CHECK0 }    
                       CLOSE    
                       { end_element(ELT_SUB_IDENT_REFN_TYPE, PARENT, $<ctxt>5,
				     GEDCOM_MAKE_NULL(val1));
		       }
                     ;

ident_rin_sect  : OPEN DELIM TAG_RIN mand_line_item   
                  { $<ctxt>$ = start_element(ELT_SUB_IDENT_RIN,
					     PARENT, $1, $3, $4,
					     GEDCOM_MAKE_STRING(val1, $4));
		    START(RIN, $1, $<ctxt>$)    
                  }    
                  no_std_subs    
                  { CHECK0 }    
                  CLOSE    
                  { end_element(ELT_SUB_IDENT_RIN, PARENT, $<ctxt>5,
				GEDCOM_MAKE_NULL(val1));
		  }
                ;

/* INDIVIDUAL ATTRIBUTE STRUCTURE */
indiv_attr_struc_sub : indiv_attr_sect   /* 0:M */
                     | indiv_resi_sect  /* 0:M */
                     ;

indiv_attr_sect : OPEN DELIM indiv_attr_tag mand_line_item
                  { $<ctxt>$ = start_element(ELT_SUB_INDIV_ATTR,
					     PARENT, $1, $3, $4,
					     GEDCOM_MAKE_STRING(val1, $4));
		    START2($1, $<ctxt>$);
		  }
                  indiv_attr_event_subs
                  { CHECK0 }
                  CLOSE
                  { end_element(ELT_SUB_INDIV_ATTR, PARENT, $<ctxt>5,
				GEDCOM_MAKE_NULL(val1));
		  }
                ;

indiv_attr_tag  : TAG_CAST { $$ = $1; START1(CAST) }
                | TAG_DSCR { $$ = $1; START1(DSCR) }
                | TAG_EDUC { $$ = $1; START1(EDUC) }
                | TAG_IDNO { $$ = $1; START1(IDNO) }
                | TAG_NATI { $$ = $1; START1(NATI) }
                | TAG_NCHI { $$ = $1; START1(NCHI) }
                | TAG_NMR  { $$ = $1; START1(NMR) }
                | TAG_OCCU { $$ = $1; START1(OCCU) }
                | TAG_PROP { $$ = $1; START1(PROP) }
                | TAG_RELI { $$ = $1; START1(RELI) }
                | TAG_SSN  { $$ = $1; START1(SSN) }
                | TAG_TITL { $$ = $1; START1(TITL) }
                ;

indiv_resi_sect : OPEN DELIM TAG_RESI 
                  { $<ctxt>$ = start_element(ELT_SUB_INDIV_RESI,
					     PARENT, $1, $3, NULL,
					     GEDCOM_MAKE_NULL(val1));
		    START(RESI, $1, $<ctxt>$)  
                  }
                  indiv_attr_event_subs 
		  { CHECK0 }
                  CLOSE  
                  { end_element(ELT_SUB_INDIV_RESI, PARENT, $<ctxt>4,
				GEDCOM_MAKE_NULL(val1));
		  }
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

indiv_birt_sect : OPEN DELIM indiv_birt_tag opt_value 
                  { $<ctxt>$
		      = start_element(ELT_SUB_INDIV_BIRT,
				      PARENT, $1, $3, $4,
				      GEDCOM_MAKE_NULL_OR_STRING(val1, $4));
		    START2($1, $<ctxt>$);
		  }
                  indiv_birt_subs
                  { CHECK0 }
                  CLOSE 
                  { end_element(ELT_SUB_INDIV_BIRT, PARENT, $<ctxt>5,
				GEDCOM_MAKE_NULL(val1));
		  }
                ;

indiv_birt_tag  : TAG_BIRT { $$ = $1; START1(BIRT) }
                | TAG_CHR { $$ = $1; START1(CHR) }
                ;

indiv_birt_subs : /* empty */
                | indiv_birt_subs indiv_birt_sub
                ;

indiv_birt_sub  : event_detail_sub
                | indiv_birt_famc_sect  { OCCUR2(FAMC,0, 1) }
                | no_std_sub
                ;

indiv_birt_famc_sect : OPEN DELIM TAG_FAMC mand_pointer
                       { struct xref_value *xr = gedcom_parse_xref($4,
								   XREF_USED,
								   XREF_FAM);
		         if (xr == NULL) HANDLE_ERROR;
			 $<ctxt>$
			   = start_element(ELT_SUB_INDIV_BIRT_FAMC,
					   PARENT, $1, $3, $4,
					   GEDCOM_MAKE_XREF_PTR(val1, xr));
		         START(FAMC, $1, $<ctxt>$) 
                       } 
                       no_std_subs 
                       { CHECK0 } 
                       CLOSE 
                       { end_element(ELT_SUB_INDIV_BIRT_FAMC, PARENT, $<ctxt>5,
				     GEDCOM_MAKE_NULL(val1));
		       }
                     ;

indiv_gen_sect  : OPEN DELIM indiv_gen_tag opt_value 
                  { $<ctxt>$
		      = start_element(ELT_SUB_INDIV_GEN,
				      PARENT, $1, $3, $4,
				      GEDCOM_MAKE_NULL_OR_STRING(val1, $4));
		    START2($1, $<ctxt>$);
		  }
                  indiv_gen_subs
                  { CHECK0 }
                  CLOSE 
                  { end_element(ELT_SUB_INDIV_GEN, PARENT, $<ctxt>5, 
				GEDCOM_MAKE_NULL(val1));
		  }
                ;

indiv_gen_tag   : TAG_DEAT { $$ = $1; START1(DEAT) }
                | TAG_BURI { $$ = $1; START1(BURI) }
                | TAG_CREM { $$ = $1; START1(CREM) }
                | TAG_BAPM { $$ = $1; START1(BAPM) }
                | TAG_BARM { $$ = $1; START1(BARM) }
                | TAG_BASM { $$ = $1; START1(BASM) }
                | TAG_BLES { $$ = $1; START1(BLES) }
                | TAG_CHRA { $$ = $1; START1(CHRA) }
                | TAG_CONF { $$ = $1; START1(CONF) }
                | TAG_FCOM { $$ = $1; START1(FCOM) }
                | TAG_ORDN { $$ = $1; START1(ORDN) }
                | TAG_NATU { $$ = $1; START1(NATU) }
                | TAG_EMIG { $$ = $1; START1(EMIG) }
                | TAG_IMMI { $$ = $1; START1(IMMI) }
                | TAG_CENS { $$ = $1; START1(CENS) }
                | TAG_PROB { $$ = $1; START1(PROB) }
                | TAG_WILL { $$ = $1; START1(WILL) }
                | TAG_GRAD { $$ = $1; START1(GRAD) }
                | TAG_RETI { $$ = $1; START1(RETI) }
                ;

indiv_gen_subs  : /* empty */
                | indiv_gen_subs indiv_gen_sub
                ;

indiv_gen_sub   : event_detail_sub
                | no_std_sub
                ;

indiv_adop_sect : OPEN DELIM TAG_ADOP opt_value 
                  { $<ctxt>$
		      = start_element(ELT_SUB_INDIV_ADOP,
				      PARENT, $1, $3, $4,
				      GEDCOM_MAKE_NULL_OR_STRING(val1, $4));
		    START(ADOP, $1, $<ctxt>$) }
                  indiv_adop_subs
		  { CHECK0 }
                  CLOSE 
                  { end_element(ELT_SUB_INDIV_ADOP, PARENT, $<ctxt>5, 
				GEDCOM_MAKE_NULL(val1));
		  }
                ;

indiv_adop_subs : /* empty */
                | indiv_adop_subs indiv_adop_sub
                ;

indiv_adop_sub  : event_detail_sub
                | indiv_adop_famc_sect  { OCCUR2(FAMC,0, 1) }
                | no_std_sub
                ;

indiv_adop_famc_sect : OPEN DELIM TAG_FAMC mand_pointer
                       { struct xref_value *xr = gedcom_parse_xref($4,
								   XREF_USED,
								   XREF_FAM);
		         if (xr == NULL) HANDLE_ERROR;
			 $<ctxt>$
			   = start_element(ELT_SUB_INDIV_ADOP_FAMC,
					   PARENT, $1, $3, $4,
					   GEDCOM_MAKE_XREF_PTR(val1, xr));
		         START(FAMC, $1, $<ctxt>$) }
                       indiv_adop_famc_subs
		       { CHECK0 }
                       CLOSE 
		       { end_element(ELT_SUB_INDIV_ADOP_FAMC, PARENT, $<ctxt>5,
				     GEDCOM_MAKE_NULL(val1));
		       }
                     ;

indiv_adop_famc_subs : /* empty */
                     | indiv_adop_famc_subs indiv_adop_famc_sub
                     ;

indiv_adop_famc_sub  : indiv_adop_famc_adop_sect  { OCCUR2(ADOP,0, 1) }
                     | no_std_sub
                     ;

indiv_adop_famc_adop_sect : OPEN DELIM TAG_ADOP mand_line_item   
                            { $<ctxt>$
				= start_element(ELT_SUB_INDIV_ADOP_FAMC_ADOP,
						PARENT, $1, $3, $4,
						GEDCOM_MAKE_STRING(val1, $4));
			      START(ADOP, $1, $<ctxt>$) }    
                            no_std_subs    
                            { CHECK0 }    
                            CLOSE    
                            { end_element(ELT_SUB_INDIV_ADOP_FAMC_ADOP,
					  PARENT, $<ctxt>5, 
					  GEDCOM_MAKE_NULL(val1));
			    }
                          ;

indiv_even_sect : OPEN DELIM TAG_EVEN
                  { $<ctxt>$ = start_element(ELT_SUB_INDIV_EVEN,
					     PARENT, $1, $3, NULL,
					     GEDCOM_MAKE_NULL(val1));
		    START(EVEN, $1, $<ctxt>$) }
                  indiv_gen_subs
		  { CHECK0 }
                  CLOSE    
                  { end_element(ELT_SUB_INDIV_EVEN, PARENT, $<ctxt>4, 
				GEDCOM_MAKE_NULL(val1));
		  }
                ;

/* LDS INDIVIDUAL ORDINANCE */
lds_indiv_ord_sub : lio_bapl_sect  /* 0:M */
                  | lio_slgc_sect  /* 0:M */
                  ;

lio_bapl_sect : OPEN DELIM lio_bapl_tag 
                { $<ctxt>$ = start_element(ELT_SUB_LIO_BAPL,
					   PARENT, $1, $3, NULL,
					   GEDCOM_MAKE_NULL(val1));
		  START2($1, $<ctxt>$);
		}
                lio_bapl_subs
                { CHECK0 }
                CLOSE 
                { end_element(ELT_SUB_LIO_BAPL, PARENT, $<ctxt>4, 
			      GEDCOM_MAKE_NULL(val1));
		}
              ;

lio_bapl_tag  : TAG_BAPL { $$ = $1; START1(BAPL) }
              | TAG_CONL { $$ = $1; START1(CONL) }
              | TAG_ENDL { $$ = $1; START1(ENDL) }
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
                     { $<ctxt>$ = start_element(ELT_SUB_LIO_BAPL_STAT,
						PARENT, $1, $3, $4,
						GEDCOM_MAKE_STRING(val1, $4));
		       START(STAT, $1, $<ctxt>$)    
                     }    
                     no_std_subs    
                     { CHECK0 }    
                     CLOSE    
                     { end_element(ELT_SUB_LIO_BAPL_STAT, PARENT, $<ctxt>5,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;
lio_bapl_date_sect : OPEN DELIM TAG_DATE mand_line_item   
                     { struct date_value dv = gedcom_parse_date($4);
		       $<ctxt>$ = start_element(ELT_SUB_LIO_BAPL_DATE,
						PARENT, $1, $3, $4,
						GEDCOM_MAKE_DATE(val1, dv));
		       START(DATE, $1, $<ctxt>$)    
                     }    
                     no_std_subs    
                     { CHECK0 }    
                     CLOSE    
                     { end_element(ELT_SUB_LIO_BAPL_DATE, PARENT, $<ctxt>5,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;
lio_bapl_temp_sect : OPEN DELIM TAG_TEMP mand_line_item   
                     { $<ctxt>$ = start_element(ELT_SUB_LIO_BAPL_TEMP,
						PARENT, $1, $3, $4,
						GEDCOM_MAKE_STRING(val1, $4));
		       START(TEMP, $1, $<ctxt>$)    
                     }    
                     no_std_subs    
                     { CHECK0 }    
                     CLOSE    
                     { end_element(ELT_SUB_LIO_BAPL_TEMP, PARENT, $<ctxt>5,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;
lio_bapl_plac_sect : OPEN DELIM TAG_PLAC mand_line_item   
                     { $<ctxt>$ = start_element(ELT_SUB_LIO_BAPL_PLAC,
						PARENT, $1, $3, $4,
						GEDCOM_MAKE_STRING(val1, $4));
		       START(PLAC, $1, $<ctxt>$)    
                     }    
                     no_std_subs    
                     { CHECK0 }    
                     CLOSE    
                     { end_element(ELT_SUB_LIO_BAPL_PLAC, PARENT, $<ctxt>5,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;

lio_slgc_sect : OPEN DELIM TAG_SLGC
                { $<ctxt>$ = start_element(ELT_SUB_LIO_SLGC,
					   PARENT, $1, $3, NULL,
					   GEDCOM_MAKE_NULL(val1));
		  START(SLGC, $1, $<ctxt>$) 
                }
                lio_slgc_subs
                { if (compat_mode(C_NO_SLGC_FAMC) && ! CHK_COND(FAMC))
		    compat_generate_slgc_famc_link($<ctxt>4);
		  else CHK(FAMC);
		  CHECK0;
		}
                CLOSE 
                { end_element(ELT_SUB_LIO_SLGC, PARENT, $<ctxt>4, 
			      GEDCOM_MAKE_NULL(val1));
		}
              ;

lio_slgc_subs : /* empty */
              | lio_slgc_subs lio_slgc_sub
              ;

lio_slgc_sub  : lio_bapl_sub
              | lio_slgc_famc_sect  { OCCUR2(FAMC, 1, 1) }
              ;

lio_slgc_famc_sect : OPEN DELIM TAG_FAMC mand_pointer
                     { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
								 XREF_FAM);
		       if (xr == NULL) HANDLE_ERROR;
		       $<ctxt>$
			 = start_element(ELT_SUB_LIO_SLGC_FAMC,
					 PARENT, $1, $3, $4,
					 GEDCOM_MAKE_XREF_PTR(val1, xr));
		       START(FAMC, $1, $<ctxt>$) 
                     } 
                     no_std_subs 
                     { CHECK0 } 
                     CLOSE 
                     { end_element(ELT_SUB_LIO_SLGC_FAMC, PARENT, $<ctxt>5,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;

/* LDS SPOUSE SEALING */
lds_spouse_seal_sub : lss_slgs_sect
                    ;

lss_slgs_sect : OPEN DELIM TAG_SLGS
                { $<ctxt>$ = start_element(ELT_SUB_LSS_SLGS,
					   PARENT, $1, $3, NULL,
					   GEDCOM_MAKE_NULL(val1));
		  START(SLGS, $1, $<ctxt>$) }
                lss_slgs_subs
		{ CHECK0 }
                CLOSE 
                { end_element(ELT_SUB_LIO_SLGC, PARENT, $<ctxt>4, 
			      GEDCOM_MAKE_NULL(val1));
		}
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
                     { $<ctxt>$ = start_element(ELT_SUB_LSS_SLGS_STAT,
						PARENT, $1, $3, $4,
						GEDCOM_MAKE_STRING(val1, $4));
		       START(STAT, $1, $<ctxt>$)    
                     }    
                     no_std_subs    
                     { CHECK0 }    
                     CLOSE    
                     { end_element(ELT_SUB_LSS_SLGS_STAT, PARENT, $<ctxt>5,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;
lss_slgs_date_sect : OPEN DELIM TAG_DATE mand_line_item   
                     { struct date_value dv = gedcom_parse_date($4);
		       $<ctxt>$ = start_element(ELT_SUB_LSS_SLGS_DATE,
						PARENT, $1, $3, $4,
						GEDCOM_MAKE_DATE(val1, dv));
		       START(DATE, $1, $<ctxt>$)    
                     }    
                     no_std_subs    
                     { CHECK0 }    
                     CLOSE    
                     { end_element(ELT_SUB_LSS_SLGS_DATE, PARENT, $<ctxt>5,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;
lss_slgs_temp_sect : OPEN DELIM TAG_TEMP mand_line_item   
                     { $<ctxt>$ = start_element(ELT_SUB_LSS_SLGS_TEMP,
						PARENT, $1, $3, $4,
						GEDCOM_MAKE_STRING(val1, $4));
		       START(TEMP, $1, $<ctxt>$)    
                     }    
                     no_std_subs    
                     { CHECK0 }    
                     CLOSE    
                     { end_element(ELT_SUB_LSS_SLGS_TEMP, PARENT, $<ctxt>5,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;
lss_slgs_plac_sect : OPEN DELIM TAG_PLAC mand_line_item   
                     { $<ctxt>$ = start_element(ELT_SUB_LSS_SLGS_PLAC,
						PARENT, $1, $3, $4,
						GEDCOM_MAKE_STRING(val1, $4));
		       START(PLAC, $1, $<ctxt>$)    
                     }    
                     no_std_subs    
                     { CHECK0 }    
                     CLOSE    
                     { end_element(ELT_SUB_LSS_SLGS_PLAC, PARENT, $<ctxt>5,
				   GEDCOM_MAKE_NULL(val1));
		     }
                   ;

/* MULTIMEDIA LINK */
multim_link_sub : multim_obje_link_sect
                | multim_obje_emb_sect
                ;

multim_obje_link_sect : OPEN DELIM TAG_OBJE DELIM POINTER    
                        { struct xref_value *xr = gedcom_parse_xref($5,
								    XREF_USED,
								    XREF_OBJE);
			  if (xr == NULL) HANDLE_ERROR;
			  $<ctxt>$
			    = start_element(ELT_SUB_MULTIM_OBJE,
					    PARENT, $1, $3, $5,
					    GEDCOM_MAKE_XREF_PTR(val1, xr));
			  START(OBJE, $1, $<ctxt>$)     
                        }     
                        no_std_subs     
                        { CHECK0 }     
                        CLOSE     
                        { end_element(ELT_SUB_MULTIM_OBJE, PARENT, $<ctxt>6,
				      GEDCOM_MAKE_NULL(val1));
			}
                      ;

multim_obje_emb_sect : OPEN DELIM TAG_OBJE
                       { $<ctxt>$ = start_element(ELT_SUB_MULTIM_OBJE,
						  PARENT, $1, $3, NULL,
						  GEDCOM_MAKE_NULL(val1));
		         START(OBJE, $1, $<ctxt>$) 
                       }
                       multim_obje_emb_subs
		       { CHECK2(FORM,FILE) }
                       CLOSE 
                       { end_element(ELT_SUB_MULTIM_OBJE, PARENT, $<ctxt>4,
				     GEDCOM_MAKE_NULL(val1));
		       }
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
                        { $<ctxt>$
			    = start_element(ELT_SUB_MULTIM_OBJE_FORM,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		          START(FORM, $1, $<ctxt>$)     
                        }     
                        no_std_subs     
                        { CHECK0 }     
                        CLOSE     
                        { end_element(ELT_SUB_MULTIM_OBJE_FORM,
				      PARENT, $<ctxt>5, 
				      GEDCOM_MAKE_NULL(val1));
			}
                      ;
multim_obje_titl_sect : OPEN DELIM TAG_TITL mand_line_item    
                        { $<ctxt>$
			    = start_element(ELT_SUB_MULTIM_OBJE_TITL,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		          START(TITL, $1, $<ctxt>$)     
                        }     
                        no_std_subs     
                        { CHECK0 }     
                        CLOSE     
                        { end_element(ELT_SUB_MULTIM_OBJE_TITL,
				      PARENT, $<ctxt>5, 
				      GEDCOM_MAKE_NULL(val1));
			}
                      ;
multim_obje_file_sect : OPEN DELIM TAG_FILE mand_line_item    
                        { $<ctxt>$
			    = start_element(ELT_SUB_MULTIM_OBJE_FILE,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		          START(FILE, $1, $<ctxt>$)     
                        }     
                        no_std_subs     
                        { CHECK0 }     
                        CLOSE     
                        { end_element(ELT_SUB_MULTIM_OBJE_FILE,
				      PARENT, $<ctxt>5, 
				      GEDCOM_MAKE_NULL(val1));
			}
                      ;

/* NOTE STRUCTURE */
note_struc_sub : note_struc_link_sect  /* 0:M */
               | note_struc_emb_sect  /* 0:M */
               ;

note_struc_link_sect : OPEN DELIM TAG_NOTE DELIM POINTER
                       { struct xref_value *xr = gedcom_parse_xref($5,
								   XREF_USED,
								   XREF_NOTE);
		         if (xr == NULL) HANDLE_ERROR;
		         $<ctxt>$
			   = start_element(ELT_SUB_NOTE,
					   PARENT, $1, $3, $5,
					   GEDCOM_MAKE_XREF_PTR(val1, xr));
			 START(NOTE, $1, $<ctxt>$) 
                       }
                       note_struc_link_subs
		       { CHECK0 }
                       CLOSE 
                       { end_element(ELT_SUB_NOTE, PARENT, $<ctxt>6, 
				     GEDCOM_MAKE_NULL(val1));
		       }
                     ;

note_struc_link_subs : /* empty */
                     | note_struc_link_subs note_struc_link_sub
                     ;

note_struc_link_sub : source_cit_sub
                    | no_std_sub
                    ;

note_struc_emb_sect : OPEN DELIM TAG_NOTE opt_line_item
                      { char* str = $4;
		        if (compat_mode(C_NOTE_TOO_LONG))
			  str = compat_long_line_get_prefix($4);
			$<ctxt>$
			  = start_element(ELT_SUB_NOTE,
					  PARENT, $1, $3, str,
					GEDCOM_MAKE_NULL_OR_STRING(val1, str));
		        reset_buffer(&concat_buffer);
			if ($4)
			  safe_buf_append(&concat_buffer, $4);
		        START(NOTE, $1, $<ctxt>$);
			if (compat_mode(C_NOTE_TOO_LONG))
			  compat_long_line_finish($<ctxt>$, $1);
                      }
                      note_struc_emb_subs
		      { CHECK0 }
                      CLOSE 
                      { char* complete = get_buf_string(&concat_buffer);
			end_element(ELT_SUB_NOTE, PARENT, $<ctxt>5,
				    GEDCOM_MAKE_STRING(val1, complete));
		      }
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
                 { $<ctxt>$ = start_element(ELT_SUB_PERS_NAME,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(NAME, $1, $<ctxt>$)  
                 }
                 pers_name_subs
		 { CHECK0 }
                 CLOSE  
                 { end_element(ELT_SUB_PERS_NAME, PARENT, $<ctxt>5, 
			       GEDCOM_MAKE_NULL(val1));
		 }
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
                      { $<ctxt>$ = start_element(ELT_SUB_PERS_NAME_NPFX,
						 PARENT, $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(NPFX, $1, $<ctxt>$)     
                      }     
                      no_std_subs     
                      { CHECK0 }     
                      CLOSE     
                      { end_element(ELT_SUB_PERS_NAME_NPFX, PARENT, $<ctxt>5,
				    GEDCOM_MAKE_NULL(val1));
		      }
                    ;
pers_name_givn_sect : OPEN DELIM TAG_GIVN mand_line_item    
                      { $<ctxt>$ = start_element(ELT_SUB_PERS_NAME_GIVN,
						 PARENT, $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(GIVN, $1, $<ctxt>$)     
                      }     
                      no_std_subs     
                      { CHECK0 }     
                      CLOSE     
                      { end_element(ELT_SUB_PERS_NAME_GIVN, PARENT, $<ctxt>5,
				    GEDCOM_MAKE_NULL(val1));
		      }
                    ;
pers_name_nick_sect : OPEN DELIM TAG_NICK mand_line_item    
                      { $<ctxt>$ = start_element(ELT_SUB_PERS_NAME_NICK,
						 PARENT, $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(NICK, $1, $<ctxt>$)     
                      }     
                      no_std_subs     
                      { CHECK0 }     
                      CLOSE     
                      { end_element(ELT_SUB_PERS_NAME_NICK, PARENT, $<ctxt>5,
				    GEDCOM_MAKE_NULL(val1));
		      }
                    ;
pers_name_spfx_sect : OPEN DELIM TAG_SPFX mand_line_item    
                      { $<ctxt>$ = start_element(ELT_SUB_PERS_NAME_SPFX,
						 PARENT, $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(SPFX, $1, $<ctxt>$)     
                      }     
                      no_std_subs     
                      { CHECK0 }     
                      CLOSE     
                      { end_element(ELT_SUB_PERS_NAME_SPFX, PARENT, $<ctxt>5,
				    GEDCOM_MAKE_NULL(val1));
		      }
                    ;
pers_name_surn_sect : OPEN DELIM TAG_SURN mand_line_item    
                      { $<ctxt>$ = start_element(ELT_SUB_PERS_NAME_SURN,
						 PARENT, $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(SURN, $1, $<ctxt>$)     
                      }     
                      no_std_subs     
                      { CHECK0 }     
                      CLOSE     
                      { end_element(ELT_SUB_PERS_NAME_SURN, PARENT, $<ctxt>5,
				    GEDCOM_MAKE_NULL(val1));
		      }
                    ;
pers_name_nsfx_sect : OPEN DELIM TAG_NSFX mand_line_item    
                      { $<ctxt>$ = start_element(ELT_SUB_PERS_NAME_NSFX,
						 PARENT, $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
		        START(NSFX, $1, $<ctxt>$)     
                      }     
                      no_std_subs     
                      { CHECK0 }     
                      CLOSE     
                      { end_element(ELT_SUB_PERS_NAME_NSFX, PARENT, $<ctxt>5,
				    GEDCOM_MAKE_NULL(val1));
		      }
                    ;

/* PLACE STRUCTURE */
place_struc_sub : place_struc_plac_sect /* 0:M */
                ;

place_struc_plac_sect : OPEN DELIM TAG_PLAC mand_line_item 
                        { $<ctxt>$
			    = start_element(ELT_SUB_PLAC,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		          START(PLAC, $1, $<ctxt>$)  
                        }
                        place_struc_plac_subs
			{ CHECK0 }
                        CLOSE  
                        { end_element(ELT_SUB_PLAC, PARENT, $<ctxt>5, 
				      GEDCOM_MAKE_NULL(val1));
			}
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
                       { $<ctxt>$
			   = start_element(ELT_SUB_PLAC_FORM,
					   PARENT, $1, $3, $4,
					   GEDCOM_MAKE_STRING(val1, $4));
		         START(FORM, $1, $<ctxt>$)     
                       }     
                       no_std_subs     
                       { CHECK0 }     
                       CLOSE     
                       { end_element(ELT_SUB_PLAC_FORM, PARENT, $<ctxt>5,
				     GEDCOM_MAKE_NULL(val1));
		       }
                     ;

/* SOURCE_CITATION */
source_cit_sub : source_cit_link_sect /* 0:M */
               | source_cit_emb_sect /* 0:M */
               ;

source_cit_link_sect : OPEN DELIM TAG_SOUR DELIM POINTER
                       { struct xref_value *xr = gedcom_parse_xref($5,
								   XREF_USED,
								   XREF_SOUR);
		         if (xr == NULL) HANDLE_ERROR;
			 $<ctxt>$
			   = start_element(ELT_SUB_SOUR,
					   PARENT, $1, $3, $5,
					   GEDCOM_MAKE_XREF_PTR(val1, xr));
		         START(SOUR, $1, $<ctxt>$) 
                       }
                       source_cit_link_subs
		       { CHECK0 }
                       CLOSE 
                       { end_element(ELT_SUB_SOUR, PARENT, $<ctxt>6, 
				     GEDCOM_MAKE_NULL(val1));
		       }
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
                       { $<ctxt>$
			   = start_element(ELT_SUB_SOUR_PAGE,
					   PARENT, $1, $3, $4,
					   GEDCOM_MAKE_STRING(val1, $4));
		         START(PAGE, $1, $<ctxt>$)     
                       }     
                       no_std_subs     
                       { CHECK0 }     
                       CLOSE     
                       { end_element(ELT_SUB_SOUR_PAGE, PARENT, $<ctxt>5,
				     GEDCOM_MAKE_NULL(val1));
		       }
                     ;

source_cit_even_sect : OPEN DELIM TAG_EVEN mand_line_item 
                       { $<ctxt>$
			   = start_element(ELT_SUB_SOUR_EVEN,
					   PARENT, $1, $3, $4,
					   GEDCOM_MAKE_STRING(val1, $4));
		         START(EVEN, $1, $<ctxt>$)     
                       }
                       source_cit_even_subs
		       { CHECK0 }
                       CLOSE     
                       { end_element(ELT_SUB_SOUR_EVEN, PARENT, $<ctxt>5,
				     GEDCOM_MAKE_NULL(val1));
		       }
                     ;

source_cit_even_subs : /* empty */
                     | source_cit_even_subs source_cit_even_sub
                     ;

source_cit_even_sub  : source_cit_even_role_sect  { OCCUR2(ROLE, 0, 1) }
                     | no_std_sub
                     ;

source_cit_even_role_sect : OPEN DELIM TAG_ROLE mand_line_item    
                          { $<ctxt>$
			      = start_element(ELT_SUB_SOUR_EVEN_ROLE,
					      PARENT, $1, $3, $4,
					      GEDCOM_MAKE_STRING(val1, $4));
			    START(ROLE, $1, $<ctxt>$)     
                          }     
                          no_std_subs     
                          { CHECK0 }     
                          CLOSE     
                          { end_element(ELT_SUB_SOUR_EVEN_ROLE,
					PARENT, $<ctxt>5, 
					GEDCOM_MAKE_NULL(val1));
			  }
                          ;

source_cit_data_sect : OPEN DELIM TAG_DATA
                       { $<ctxt>$ = start_element(ELT_SUB_SOUR_DATA,
						  PARENT, $1, $3, NULL,
						  GEDCOM_MAKE_NULL(val1));
		         START(DATA, $1, $<ctxt>$) 
                       }
                       source_cit_data_subs
		       { CHECK0 }
                       CLOSE 
                       { end_element(ELT_SUB_SOUR_DATA, PARENT, $<ctxt>4,
				     GEDCOM_MAKE_NULL(val1));
		       }
                     ;

source_cit_data_subs : /* empty */
                     | source_cit_data_subs source_cit_data_sub
                     ;

source_cit_data_sub : source_cit_data_date_sect  { OCCUR2(DATE, 0, 1) }
                    | source_cit_text_sect  /* 0:M */
		    | no_std_sub
                    ;

source_cit_data_date_sect : OPEN DELIM TAG_DATE mand_line_item    
                            { struct date_value dv = gedcom_parse_date($4);
			      $<ctxt>$
				= start_element(ELT_SUB_SOUR_DATA_DATE,
						PARENT, $1, $3, $4,
						GEDCOM_MAKE_DATE(val1, dv));
			      START(DATE, $1, $<ctxt>$)     
                            }     
                            no_std_subs     
                            { CHECK0 }     
                            CLOSE     
                            { end_element(ELT_SUB_SOUR_DATA_DATE,
					  PARENT, $<ctxt>5, 
					  GEDCOM_MAKE_NULL(val1));
			    }
                          ;

source_cit_text_sect : OPEN DELIM TAG_TEXT mand_line_item 
                       { $<ctxt>$
			   = start_element(ELT_SUB_SOUR_TEXT,
					   PARENT, $1, $3, $4,
					   GEDCOM_MAKE_STRING(val1, $4));
		        reset_buffer(&concat_buffer);
		        safe_buf_append(&concat_buffer, $4);
		        START(TEXT, $1, $<ctxt>$)  
                       }
                       source_cit_text_subs
		       { CHECK0 }
                       CLOSE  
                       { char* complete = get_buf_string(&concat_buffer);
			 end_element(ELT_SUB_SOUR_TEXT, PARENT, $<ctxt>5,
				     GEDCOM_MAKE_STRING(val1, complete));
		       }
                     ;

source_cit_text_subs : /* empty */
                     | source_cit_text_subs source_cit_text_sub
                     ;

source_cit_text_sub : continuation_sub
                    | no_std_sub
                    ;

source_cit_quay_sect : OPEN DELIM TAG_QUAY mand_line_item    
                       { $<ctxt>$
			   = start_element(ELT_SUB_SOUR_QUAY,
					   PARENT, $1, $3, $4,
					   GEDCOM_MAKE_STRING(val1, $4));
		         START(QUAY, $1, $<ctxt>$)     
                       }     
                       no_std_subs     
                       { CHECK0 }     
                       CLOSE     
                       { end_element(ELT_SUB_SOUR_QUAY, PARENT, $<ctxt>5,
				     GEDCOM_MAKE_NULL(val1));
		       }
                     ;

source_cit_emb_sect : OPEN DELIM TAG_SOUR mand_line_item
                      { $<ctxt>$ = start_element(ELT_SUB_SOUR,
						 PARENT, $1, $3, $4,
						 GEDCOM_MAKE_STRING(val1, $4));
		        reset_buffer(&concat_buffer);
		        safe_buf_append(&concat_buffer, $4);
		        START(SOUR, $1, $<ctxt>$) 
                      }
                      source_cit_emb_subs
		      { CHECK0 }
                      CLOSE 
                      { char* complete = get_buf_string(&concat_buffer);
			end_element(ELT_SUB_SOUR, PARENT, $<ctxt>5,
				    GEDCOM_MAKE_STRING(val1, complete));
		      }
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
                     | source_repos_repo_txt_sect
                       { if (!compat_mode(C_NONSTD_SOUR_TAGS))
			   INVALID_TAG("REPO");
		         OCCUR2(REPO, 0, 1)
		       }
                     ;

/* Only for compatibility */
source_repos_repo_txt_sect : OPEN DELIM TAG_REPO opt_line_item
                 { if (compat_mode(C_NONSTD_SOUR_TAGS)) {
		     $<ctxt>$ =
		       compat_generate_nonstd_sour_start(PARENT, $1, $3, $4,
							 &usertag_buffer);
		   }
                 }
                 CLOSE            
                 { if (compat_mode(C_NONSTD_SOUR_TAGS))
		     compat_generate_nonstd_sour_end(PARENT, $<ctxt>5);
		 }
               ;

source_repos_repo_sect : OPEN DELIM TAG_REPO DELIM POINTER
                         { struct xref_value *xr
			     = gedcom_parse_xref($5, XREF_USED, XREF_REPO);
			   if (xr == NULL) HANDLE_ERROR;
			   $<ctxt>$
			     = start_element(ELT_SUB_REPO,
					     PARENT, $1, $3, $5,
					     GEDCOM_MAKE_XREF_PTR(val1, xr));
			   START(REPO, $1, $<ctxt>$);
			 }
                         source_repos_repo_subs
			 { CHECK0 }
                         CLOSE 
                         { end_element(ELT_SUB_REPO, PARENT, $<ctxt>6, 
				       GEDCOM_MAKE_NULL(val1));
			 }
                       ;

source_repos_repo_subs : /* empty */
                       | source_repos_repo_subs source_repos_repo_sub
                       ;

source_repos_repo_sub  : note_struc_sub
                       | caln_sect  /* 0:M */
                       | no_std_sub
                       ;

caln_sect : OPEN DELIM TAG_CALN mand_line_item 
            { $<ctxt>$ = start_element(ELT_SUB_REPO_CALN,
				       PARENT, $1, $3, $4,
				       GEDCOM_MAKE_STRING(val1, $4));
	      START(CALN, $1, $<ctxt>$) 
	    }
            caln_subs
	    { CHECK0 }
            CLOSE  
	    { end_element(ELT_SUB_REPO_CALN, PARENT, $<ctxt>5, 
			  GEDCOM_MAKE_NULL(val1));
	    }
          ;

caln_subs : /* empty */
          | caln_subs caln_sub
          ;

caln_sub  : caln_medi_sect  { OCCUR2(MEDI, 0, 1) }
          | no_std_sub
          ;

caln_medi_sect : OPEN DELIM TAG_MEDI mand_line_item    
                 { $<ctxt>$ = start_element(ELT_SUB_REPO_CALN_MEDI,
					    PARENT, $1, $3, $4,
					    GEDCOM_MAKE_STRING(val1, $4));
		   START(MEDI, $1, $<ctxt>$)  
		 }   
		 no_std_subs   
		 { CHECK0 }   
		 CLOSE   
		 { end_element(ELT_SUB_REPO_CALN_MEDI, PARENT, $<ctxt>5, 
			       GEDCOM_MAKE_NULL(val1));
		 }
               ;
 
/* SPOUSE TO FAMILY LINK */
spou_fam_link_sub : spou_fam_fams_sect  /* 0:M */
                  ;

spou_fam_fams_sect : OPEN DELIM TAG_FAMS mand_pointer
                     { struct xref_value *xr = gedcom_parse_xref($4, XREF_USED,
								 XREF_FAM);
		       if (xr == NULL) HANDLE_ERROR;
		       $<ctxt>$
			 = start_element(ELT_SUB_FAMS,
					 PARENT, $1, $3, $4,
					 GEDCOM_MAKE_XREF_PTR(val1, xr));
		       START(FAMS, $1, $<ctxt>$) 
                     }
                     spou_fam_fams_subs
		     { CHECK0 }
                     CLOSE 
                     { end_element(ELT_SUB_FAMS, PARENT, $<ctxt>5, 
				   GEDCOM_MAKE_NULL(val1));
		     }
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
            | error error_subs
              CLOSE  { HANDLE_ERROR }
	    ;

no_std_rec  : user_rec /* 0:M */
	    | gen_rec
            | error error_subs
              CLOSE  { HANDLE_ERROR }
	    ;

user_rec    : OPEN DELIM opt_xref USERTAG
              { if ($4.string[0] != '_') {
		  if ((compat_mode(C_551_TAGS)
		       && compat_check_551_tag($4.string, &usertag_buffer))
		      ||
		      (compat_mode(C_NONSTD_SOUR_TAGS)
		       && compat_check_sour_tag($4.string, &usertag_buffer))) {
		    $4.string = get_buf_string(&usertag_buffer);
		  }
		  else {		  
		    gedcom_error(_("Undefined tag (and not a valid user tag): %s"),
				 $4);
		    YYERROR;
		  }
	        }
	      }
              opt_value
              { struct xref_value *xr = NULL;
	        if ($3 != NULL) {
		  xr = gedcom_parse_xref($3, XREF_DEFINED, XREF_USER);
		  if (xr == NULL) HANDLE_ERROR;
		}
		$<ctxt>$ = start_record(REC_USER,
					$1,
					GEDCOM_MAKE_NULL_OR_XREF_PTR(val1, xr),
					$4, $6, &val2);
	        START($4, $1, $<ctxt>$)
	      }
	      user_sects
              { CHECK0 }
	      CLOSE
              { end_record(REC_USER, $<ctxt>7, GEDCOM_MAKE_NULL(val1)); }
            ;
user_sect   : OPEN DELIM opt_xref USERTAG
              { if ($4.string[0] != '_') {
		  if ((compat_mode(C_551_TAGS)
		       && compat_check_551_tag($4.string, &usertag_buffer))
		      ||
		      (compat_mode(C_SUBM_COMM)
		       && compat_check_subm_comm($4.string, get_parenttag(0),
						 &usertag_buffer))
		      ||
		      (compat_mode(C_NONSTD_SOUR_TAGS)
		       && compat_check_sour_tag($4.string, &usertag_buffer))) {
		    $4.string = get_buf_string(&usertag_buffer);
		  }
		  else {
		    gedcom_error(_("Undefined tag (and not a valid user tag): %s"),
				 $4);
		    YYERROR;
		  }
	        }
	      }
              opt_value
              { $<ctxt>$ = start_element(ELT_USER, PARENT, $1, $4, $6, &val2);
		START($4, $1, $<ctxt>$);
	      }
	      user_sects
              { CHECK0 }
	      CLOSE
              { end_element(ELT_USER, PARENT, $<ctxt>7, 
			    GEDCOM_MAKE_NULL(val1));
	        if (compat_mode(C_SUBM_COMM))
	          compat_close_subm_comm();
	      }
            ;

user_sects   : /* empty */     { }
            | user_sects user_sect { }
            | user_sects gen_sect
              { if (compat_mode(C_SUBM_COMM)) {
                }
	        else {
		  gedcom_error(_("Standard tag not allowed in user section"));
		  YYERROR;
		}
	      }
            ;

opt_xref    : /* empty */        { $$ = NULL; }
            | POINTER DELIM        { $$ = $1; }
            ;

opt_value   : /* empty */        { GEDCOM_MAKE_NULL(val2);
                                   $$ = NULL; }
            | DELIM POINTER      { struct xref_value *xr
				     = gedcom_parse_xref($2, XREF_USED,
							 XREF_USER);
	                           GEDCOM_MAKE_XREF_PTR(val2, xr);
	                           $$ = $2; }
            | DELIM line_item    { GEDCOM_MAKE_STRING(val2, $2);
	                           $$ = $2; }
            ;

mand_pointer : /* empty */ { gedcom_error(_("Missing pointer")); YYERROR; }
             | DELIM POINTER { gedcom_debug_print("==Ptr: %s==", $2);
                               $$ = $2; }
             ;

mand_line_item : /* empty */
                 { if (compat_mode(C_NO_REQUIRED_VALUES)) {
                     gedcom_debug_print("==Val: ==");
		     $$ = VALUE_IF_MISSING;
		   }
		   else {
		     gedcom_error(_("Missing value")); YYERROR;
		   }
		 }
               | DELIM line_item { gedcom_debug_print("==Val: %s==", $2);
                                   $$ = $2; }
               ;

opt_line_item : /* empty */     { $$ = NULL; }
              | DELIM line_item { gedcom_debug_print("==Val: %s==", $2);
	                          $$ = $2; }
              ;

line_item   : anychar  { size_t i;
                         reset_buffer(&line_item_buffer); 
		         /* The following also takes care of '@@' */
			 if (!strncmp($1, "@@", 3))
			   SAFE_BUF_ADDCHAR(&line_item_buffer, '@')
			 else
			   for (i=0; i < strlen($1); i++)
			     SAFE_BUF_ADDCHAR(&line_item_buffer, $1[i])
			 $$ = get_buf_string(&line_item_buffer);
                       }
            | ESCAPE   { size_t i;
                         reset_buffer(&line_item_buffer); 
		         for (i=0; i < strlen($1); i++)
			   SAFE_BUF_ADDCHAR(&line_item_buffer, $1[i])
			 $$ = get_buf_string(&line_item_buffer);
	               }
            | line_item anychar
                  { size_t i;
		    /* The following also takes care of '@@' */
		    if (!strncmp($2, "@@", 3))
		      SAFE_BUF_ADDCHAR(&line_item_buffer, '@')
		    else
		      for (i=0; i < strlen($2); i++)
			SAFE_BUF_ADDCHAR(&line_item_buffer, $2[i])
		    $$ = get_buf_string(&line_item_buffer);
		  }
            | line_item ESCAPE
                  { size_t i;
		    for (i=0; i < strlen($2); i++)
		      SAFE_BUF_ADDCHAR(&line_item_buffer, $2[i])
		    $$ = get_buf_string(&line_item_buffer);
		  }
            | line_item error anychar { HANDLE_ERROR; }
            | line_item error ESCAPE  { HANDLE_ERROR; }
            ;

anychar     : ANYCHAR        { }
            | DELIM        { }
            ;

error_subs  : /* empty */
            | error_subs error_sect
            ;

error_sect  : OPEN DELIM opt_xref anytag opt_value error_subs CLOSE { }
            ;

gen_sect    : OPEN DELIM opt_xref anystdtag
              { if (compat_mode(C_SUBM_COMM)
		    && compat_check_subm_comm_cont($4.string)) {
		  /* Will pass here */
		}
	        else {
		  INVALID_TAG($4.string);
		}
	      }
              opt_value
              { if (compat_mode(C_SUBM_COMM)) {
		  $<ctxt>$ = compat_subm_comm_cont_start(PARENT, $6);
                }
	      }
	      opt_sects CLOSE
              { if (compat_mode(C_SUBM_COMM))
		  compat_subm_comm_cont_end(PARENT, $<ctxt>7);
	      }
            ;

gen_rec : gen_rec_top
        | gen_rec_norm
        ;

gen_rec_norm : OPEN DELIM opt_xref anystdtag
               { INVALID_TOP_TAG($4.string) }
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
            ;

%%

/* Functions that handle the counting of subtags */

int* count_arrays[MAXGEDCLEVEL+1];
char tag_stack[MAXGEDCLEVEL+1][MAXSTDTAGLEN+1];
Gedcom_ctxt ctxt_stack[MAXGEDCLEVEL+1];

void push_countarray(int level)
{
  int *count = NULL;
  gedcom_debug_print("Push Count level: %d, level: %d", count_level, level);
  if (count_level != level + 1) {
    gedcom_error(_("Internal error: count level mismatch"));
    exit(1);
  }
  if (count_level > MAXGEDCLEVEL) {
    gedcom_error(_("Internal error: count array overflow"));
    exit(1);
  }
  else {
    gedcom_debug_print("calloc countarray %d", count_level);
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

void set_parenttag(const char* tag)
{
  strncpy(tag_stack[count_level+1], tag, MAXSTDTAGLEN+1);
}

void set_parentctxt(Gedcom_ctxt ctxt)
{
  ctxt_stack[count_level+1] = ctxt;
}

char* get_parenttag(int offset)
{
  return tag_stack[count_level - offset];
}

Gedcom_ctxt get_parentctxt(int offset)
{
  return ctxt_stack[count_level - offset];
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
  gedcom_debug_print("Pop Count level: %d", count_level);
  if (count_level < 0) {
    gedcom_error(_("Internal error: count array underflow"));
    exit(1);
  }
  else {
    count = count_arrays[count_level];
    gedcom_debug_print("free countarray %d", count_level);
    free(count);
    count_arrays[count_level] = NULL;
  }
}

void clean_up()
{
  gedcom_debug_print("Cleanup countarrays");
  while (count_level > 0) {
    pop_countarray();
    --count_level;
  }
}

void cleanup_concat_buffer()
{
  cleanup_buffer(&concat_buffer);
}

void cleanup_line_item_buffer()
{
  cleanup_buffer(&line_item_buffer);
}

void cleanup_usertag_buffer()
{
  cleanup_buffer(&usertag_buffer);
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

int gedcom_debug_print(const char* s, ...)
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
