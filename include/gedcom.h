/* External header for the Gedcom parser library.
   Copyright (C) 2001,2002 The Genes Development Team
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

#ifndef __GEDCOM_H
#define __GEDCOM_H

#include <stdio.h>

__BEGIN_DECLS

#ifndef GEDCOM_INTERNAL
#include <gedcom-tags.h>
#endif

/**************************************************************************/
/***  First the records and elements to subscribe upon                  ***/
/**************************************************************************/
									   
typedef enum _REC {
  REC_HEAD,
  REC_FAM,
  REC_INDI,
  REC_OBJE,
  REC_NOTE,
  REC_REPO,
  REC_SOUR,
  REC_SUBN,
  REC_SUBM,
  REC_USER,
  NR_OF_RECS     /* Just a final value to be used in array boundaries */
} Gedcom_rec;

typedef enum _ELT {
  ELT_HEAD_SOUR,
  ELT_HEAD_SOUR_VERS,
  ELT_HEAD_SOUR_NAME,
  ELT_HEAD_SOUR_CORP,
  ELT_HEAD_SOUR_DATA,
  ELT_HEAD_SOUR_DATA_DATE,
  ELT_HEAD_SOUR_DATA_COPR,
  ELT_HEAD_DEST,
  ELT_HEAD_DATE,
  ELT_HEAD_DATE_TIME,
  ELT_HEAD_SUBM,
  ELT_HEAD_SUBN,
  ELT_HEAD_FILE,
  ELT_HEAD_COPR,
  ELT_HEAD_GEDC,
  ELT_HEAD_GEDC_VERS,
  ELT_HEAD_GEDC_FORM,
  ELT_HEAD_CHAR,
  ELT_HEAD_CHAR_VERS,
  ELT_HEAD_LANG,
  ELT_HEAD_PLAC,
  ELT_HEAD_PLAC_FORM,
  ELT_HEAD_NOTE,
  
  ELT_FAM_HUSB,
  ELT_FAM_WIFE,
  ELT_FAM_CHIL,
  ELT_FAM_NCHI,
  ELT_FAM_SUBM,
  
  ELT_INDI_RESN,
  ELT_INDI_SEX,
  ELT_INDI_SUBM,
  ELT_INDI_ALIA,
  ELT_INDI_ANCI,
  ELT_INDI_DESI,
  ELT_INDI_RFN,
  ELT_INDI_AFN,
  
  ELT_OBJE_FORM,
  ELT_OBJE_TITL,
  ELT_OBJE_BLOB,
  ELT_OBJE_BLOB_CONT,
  ELT_OBJE_OBJE,
  
  ELT_REPO_NAME,

  ELT_SOUR_DATA,
  ELT_SOUR_DATA_EVEN,
  ELT_SOUR_DATA_EVEN_DATE,
  ELT_SOUR_DATA_EVEN_PLAC,
  ELT_SOUR_DATA_AGNC,
  ELT_SOUR_AUTH,
  ELT_SOUR_TITL,
  ELT_SOUR_ABBR,
  ELT_SOUR_PUBL,
  ELT_SOUR_TEXT,

  ELT_SUBN_SUBM,
  ELT_SUBN_FAMF,
  ELT_SUBN_TEMP,
  ELT_SUBN_ANCE,
  ELT_SUBN_DESC,
  ELT_SUBN_ORDI,
  ELT_SUBN_RIN,

  ELT_SUBM_NAME,
  ELT_SUBM_LANG,
  ELT_SUBM_RFN,
  ELT_SUBM_RIN,

  ELT_SUB_ADDR,
  ELT_SUB_ADDR_CONT,
  ELT_SUB_ADDR_ADR1,
  ELT_SUB_ADDR_ADR2,
  ELT_SUB_ADDR_CITY,
  ELT_SUB_ADDR_STAE,
  ELT_SUB_ADDR_POST,
  ELT_SUB_ADDR_CTRY,
  
  ELT_SUB_PHON,

  ELT_SUB_ASSO,
  ELT_SUB_ASSO_TYPE,
  ELT_SUB_ASSO_RELA,

  ELT_SUB_CHAN,
  ELT_SUB_CHAN_DATE,
  ELT_SUB_CHAN_TIME,

  ELT_SUB_FAMC,
  ELT_SUB_FAMC_PEDI,

  ELT_SUB_CONT,
  ELT_SUB_CONC,

  ELT_SUB_EVT_TYPE,
  ELT_SUB_EVT_DATE,
  ELT_SUB_EVT_AGE,
  ELT_SUB_EVT_AGNC,
  ELT_SUB_EVT_CAUS,

  ELT_SUB_FAM_EVT,
  ELT_SUB_FAM_EVT_HUSB,
  ELT_SUB_FAM_EVT_WIFE,
  ELT_SUB_FAM_EVT_AGE,
  ELT_SUB_FAM_EVT_EVEN,

  ELT_SUB_IDENT_REFN,
  ELT_SUB_IDENT_REFN_TYPE,
  ELT_SUB_IDENT_RIN,

  ELT_SUB_INDIV_ATTR,
  ELT_SUB_INDIV_RESI,
  ELT_SUB_INDIV_BIRT,
  ELT_SUB_INDIV_BIRT_FAMC,
  ELT_SUB_INDIV_GEN,
  ELT_SUB_INDIV_ADOP,
  ELT_SUB_INDIV_ADOP_FAMC,
  ELT_SUB_INDIV_ADOP_FAMC_ADOP,
  ELT_SUB_INDIV_EVEN,

  ELT_SUB_LIO_BAPL,
  ELT_SUB_LIO_BAPL_STAT,
  ELT_SUB_LIO_BAPL_DATE,
  ELT_SUB_LIO_BAPL_TEMP,
  ELT_SUB_LIO_BAPL_PLAC,
  ELT_SUB_LIO_SLGC,
  ELT_SUB_LIO_SLGC_FAMC,

  ELT_SUB_LSS_SLGS,
  ELT_SUB_LSS_SLGS_STAT,
  ELT_SUB_LSS_SLGS_DATE,
  ELT_SUB_LSS_SLGS_TEMP,
  ELT_SUB_LSS_SLGS_PLAC,

  ELT_SUB_MULTIM_OBJE,
  ELT_SUB_MULTIM_OBJE_FORM,
  ELT_SUB_MULTIM_OBJE_TITL,
  ELT_SUB_MULTIM_OBJE_FILE,

  ELT_SUB_NOTE,

  ELT_SUB_PERS_NAME,
  ELT_SUB_PERS_NAME_NPFX,
  ELT_SUB_PERS_NAME_GIVN,
  ELT_SUB_PERS_NAME_NICK,
  ELT_SUB_PERS_NAME_SPFX,
  ELT_SUB_PERS_NAME_SURN,
  ELT_SUB_PERS_NAME_NSFX,

  ELT_SUB_PLAC,
  ELT_SUB_PLAC_FORM,

  ELT_SUB_SOUR,
  ELT_SUB_SOUR_PAGE,
  ELT_SUB_SOUR_EVEN,
  ELT_SUB_SOUR_EVEN_ROLE,
  ELT_SUB_SOUR_DATA,
  ELT_SUB_SOUR_DATA_DATE,
  ELT_SUB_SOUR_TEXT,
  ELT_SUB_SOUR_QUAY,

  ELT_SUB_REPO,
  ELT_SUB_REPO_CALN,
  ELT_SUB_REPO_CALN_MEDI,

  ELT_SUB_FAMS,
  
  ELT_USER,
  
  NR_OF_ELTS     /* Just a final value to be used in array boundaries */
} Gedcom_elt;

/**************************************************************************/
/***  Definition of some auxiliary types                                ***/
/**************************************************************************/
									   
typedef enum _MECH {
  IMMED_FAIL,
  DEFER_FAIL,
  IGNORE_ERRORS
} Gedcom_err_mech;

typedef enum _MSG {
  ERROR,
  WARNING,
  MESSAGE
} Gedcom_msg_type;

typedef enum _DATE_TYPE {
  DATE_UNRECOGNIZED,   /* Neither jday1 as jday2 are significant */
  DATE_EXACT,          /* Only jday1 is significant */
  DATE_BOUNDED         /* Both jday1 and jday2 are significant */
} Date_type;

typedef enum _CALENDAR_TYPE {
  CAL_GREGORIAN,
  CAL_JULIAN,
  CAL_HEBREW,
  CAL_FRENCH_REV,
  CAL_UNKNOWN
} Calendar_type;

typedef enum _YEAR_TYPE {
  YEAR_SINGLE,
  YEAR_DOUBLE     /* In this case, the 'year' indicates the last value */
} Year_type;

typedef enum _DATE_VAL_MOD {
  /* Simple date */
  DV_NO_MODIFIER,
  /* Range values */
  DV_BEFORE,
  DV_AFTER,
  DV_BETWEEN,       /* Two dates are given */
  /* Period values */
  DV_FROM,
  DV_TO,
  DV_FROM_TO,       /* Two dates are given */
  /* Approx values */
  DV_ABOUT,
  DV_CALCULATED,
  DV_ESTIMATED,
  /* Other */
  DV_INTERPRETED,   /* One date and a phrase is given */
  DV_PHRASE         /* Only phrase is given */
} Date_value_type;

/* All Unicode characters between U+0000 and U+FFFF can be encoded in
   UTF-8 with 3 or less bytes */
#define UTF_FACTOR 3

#define MAX_DAY_LEN    2
#define MAX_MONTH_LEN  4
#define MAX_YEAR_LEN   7
#define MAX_PHRASE_LEN 35 * UTF_FACTOR

struct date {
  Calendar_type cal;
  char day_str[MAX_DAY_LEN + 1];
  char month_str[MAX_MONTH_LEN + 1];
  char year_str[MAX_YEAR_LEN + 1];
  int day;    /* starts at 1 */
  int month;  /* starts at 1 */
  int year;   /* the highest value for double years */
  Year_type year_type;
  Date_type type;
  long int sdn1;
  long int sdn2;
};

struct date_value {
  Date_value_type type;
  struct date date1;
  struct date date2;
  char phrase[MAX_PHRASE_LEN + 1];
};

/* Type for context handling, meant to be opaque */
typedef void* Gedcom_ctxt;

/**************************************************************************/
/***  Things meant to be internal, susceptible to changes               ***/
/***  Use the GEDCOM_STRING/GEDCOM_DATE interface instead of relying    ***/
/***  on this !!                                                        ***/
/**************************************************************************/
									   
typedef enum _GEDCOM_VAL_TYPE {
  GV_NULL,
  GV_CHAR_PTR,
  GV_DATE_VALUE
} Gedcom_val_type;

union _Gedcom_val_union {
  char* string_val;
  struct date_value date_val;
};

typedef struct _Gedcom_val_struct {
  Gedcom_val_type type;
  union _Gedcom_val_union value;
} Gedcom_val_struct;

void gedcom_cast_error(char* file, int line,
		       Gedcom_val_type tried_type,
		       Gedcom_val_type real_type);

extern struct date_value def_date_val;

#define GV_CHECK_CAST(VAL, TYPE, MEMBER, DEFVAL)                              \
   ((VAL->type == TYPE) ?                                                     \
    VAL->value.MEMBER :                                                       \
    (gedcom_cast_error(__FILE__,__LINE__, TYPE, VAL->type), DEFVAL))

#define GV_IS_TYPE(VAL, TYPE)                                                 \
   (VAL->type == TYPE)

/**************************************************************************/
/***  Function interface                                                ***/
/**************************************************************************/

/* Type for parsed values, meant to be opaque */
typedef Gedcom_val_struct* Gedcom_val;

/* Check to determine whether there is a parsed value or not */
#define GEDCOM_IS_NULL(VAL) \
   GV_IS_TYPE(VAL, GV_NULL)

/* This returns the char* from a Gedcom_val, if appropriate */
/* It gives a gedcom_warning if the cast is not correct     */
#define GEDCOM_STRING(VAL) \
   GV_CHECK_CAST(VAL, GV_CHAR_PTR, string_val, "")
#define GEDCOM_IS_STRING(VAL) \
   GV_IS_TYPE(VAL, GV_CHAR_PTR)

/* This returns the struct date_value from a Gedcom_val, if appropriate */
/* It gives a gedcom_warning if the cast is not correct                 */
#define GEDCOM_DATE(VAL) \
   GV_CHECK_CAST(VAL, GV_DATE_VALUE, date_val, def_date_val)
#define GEDCOM_IS_DATE(VAL) \
   GV_IS_TYPE(VAL, GV_DATE_VALUE)

typedef void
        (*Gedcom_msg_handler)
        (Gedcom_msg_type type, char *msg);

typedef Gedcom_ctxt
        (*Gedcom_rec_start_cb)
        (int level, Gedcom_val xref, char *tag, int tag_value);
typedef void
        (*Gedcom_rec_end_cb)
        (Gedcom_ctxt self);

typedef Gedcom_ctxt
        (*Gedcom_elt_start_cb)
        (Gedcom_ctxt parent,
	 int level, char *tag, char *raw_value,
	 int tag_value, Gedcom_val parsed_value);
typedef void
        (*Gedcom_elt_end_cb)
        (Gedcom_ctxt parent, Gedcom_ctxt self, Gedcom_val parsed_value);

typedef void
        (*Gedcom_def_cb)
        (Gedcom_ctxt parent, int level, char *tag, char *raw_value,
	 int tag_value);

int     gedcom_parse_file(char* file_name);
void    gedcom_set_debug_level(int level, FILE* trace_output);
void    gedcom_set_error_handling(Gedcom_err_mech mechanism);
void    gedcom_set_compat_handling(int enable_compat);
void    gedcom_set_message_handler(Gedcom_msg_handler func);
void    gedcom_set_default_callback(Gedcom_def_cb func);

void    gedcom_subscribe_to_record(Gedcom_rec rec,
				   Gedcom_rec_start_cb cb_start,
				   Gedcom_rec_end_cb cb_end);
void    gedcom_subscribe_to_element(Gedcom_elt elt,
				    Gedcom_elt_start_cb cb_start,
				    Gedcom_elt_end_cb cb_end);

/* Separate value parsing functions */
struct date_value gedcom_parse_date(char* line_value);

__END_DECLS

#endif /* __GEDCOM_H */
