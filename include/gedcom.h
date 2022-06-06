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

#ifndef __GEDCOM_H
#define __GEDCOM_H

#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GEDCOM_INTERNAL
#include <gedcom-tags.h>
#endif

int gedcom_check_version(int major, int minor, int patch);

/**************************************************************************/
/***  First the records and elements to subscribe upon                  ***/
/**************************************************************************/
									   
enum _Gedcom_rec {
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
};

  /** \brief Record types
      \ingroup start_end

      See the
      <a href="interface.html#Record_identifiers">interface details</a>.
  */
typedef enum _Gedcom_rec Gedcom_rec;

enum _Gedcom_elt {
  ELT_HEAD_SOUR = NR_OF_RECS,
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
};
  
  /** \brief Element types
      \ingroup start_end

      See the
      <a href="interface.html#Element_identifiers">interface details</a>.
  */
typedef enum _Gedcom_elt Gedcom_elt;

/**************************************************************************/
/***  Definition of some auxiliary types                                ***/
/**************************************************************************/

  /** \addtogroup error */
  /** @{ */
  /** \brief Error handling mechanisms.
      
      These are the possible error handling mechanisms.
      \sa gedcom_set_error_handling
  */
enum _Gedcom_err_mech {
  IMMED_FAIL,    /**< immediately fail the parsing on an error (this is
		       the default) */
  DEFER_FAIL,    /**< continue parsing after an error, but return a failure
		       code eventually */
  IGNORE_ERRORS  /**< continue parsing after an error, return success always */
};

  /** \brief Error handling mechanisms. */
typedef enum _Gedcom_err_mech Gedcom_err_mech;

  /** \brief Message type in message handler callbacks
      
      This will be passed to the message callback to indicate the message type.
      \sa gedcom_set_message_handler
  */
enum _Gedcom_msg_type {
  ERROR,               /**< An error message */
  WARNING,             /**< A warning message */
  MESSAGE              /**< Just a message */
};

  /** \brief Message type in message handler callbacks */
typedef enum _Gedcom_msg_type Gedcom_msg_type;
  /** @} */

/* All Unicode characters between U+0000 and U+FFFF can be encoded in
   UTF-8 with 3 or less bytes */
#define UTF_FACTOR 3

#define MAX_DAY_LEN    2
#define MAX_MONTH_LEN  4
#define MAX_YEAR_LEN   7
#define MAX_PHRASE_LEN 35 * UTF_FACTOR

  /** \addtogroup parsed_date */
  /** @{ */
  /** \brief Date type

      This determines which one of the serial day numbers (Julian days) are
      relevant in the struct date.
   */
enum _Date_type {
  DATE_UNRECOGNIZED,   /**< Neither date#sdn1 as date#sdn2 are significant */
  DATE_EXACT,          /**< Only date#sdn1 is significant */
  DATE_BOUNDED         /**< Both date#sdn1 and date#sdn2 are significant */
};

  /** \brief Date type */
typedef enum _Date_type Date_type;

  /** \brief Calendar type

      This determines the calendary type (see calendar overview LINK TBD).
   */
enum _Calendar_type {
  CAL_GREGORIAN,  /**< The Gregorian calendar */
  CAL_JULIAN,     /**< The Julian calendar */
  CAL_HEBREW,     /**< The Hebrew (Jewish) calendar */
  CAL_FRENCH_REV, /**< The calendar used after the French Revolution */
  CAL_UNKNOWN     /**< An unknown calendar type */
};

  /** \brief Calendar type */
typedef enum _Calendar_type Calendar_type;

  /** \brief Year type

      This determines whether the year has one value (e.g. 1677) or two values
      (e.g. 1677/78, the first value is in annunciation style, the second in
      circumcision style).
   */
enum _Year_type {
  YEAR_SINGLE,  /**< There is only one value for the year */
  YEAR_DOUBLE   /**< The date#year_str contains two values.  Note that the
		   date#year
		   will then contain the circumcision style year, i.e. the
		   highest one */
};

  /** \brief Year type */
typedef enum _Year_type Year_type;

  /** \brief Date value type

      This determines which members in the struct date_value are relevant, as
      given in the description below.
   */
enum _Date_value_type {
  /* Simple date */
  DV_NO_MODIFIER, /**< Just a simple date: date1 */
  /* Range values */
  DV_BEFORE,      /**< A range (BEFORE date1) */
  DV_AFTER,       /**< A range (AFTER date1) */
  DV_BETWEEN,     /**< A range (BETWEEN date1 AND date2) */
  /* Period values */
  DV_FROM,        /**< A period (FROM date1) */
  DV_TO,          /**< A period (TO date1) */
  DV_FROM_TO,     /**< A period (FROM date1 TO date2) */
  /* Approx values */
  DV_ABOUT,       /**< An approximation (ABOUT date1) */
  DV_CALCULATED,  /**< An approximation (CALCULATED date1) */
  DV_ESTIMATED,   /**< An approximation (ESTIMATED date1) */
  /* Other */
  DV_INTERPRETED, /**< INTERPRETED date1 FROM phrase */
  DV_PHRASE       /**< phrase */
};

  /** \brief Date value type */
typedef enum _Date_value_type Date_value_type;

  /** \brief Date input type for gedcom_normalize_date

      See explanation for gedcom_normalize_date().
   */
enum _Date_input {
  DI_FROM_STRINGS,
    /**< compute from date#day_str, date#month_str and date#year_str */
  DI_FROM_NUMBERS,
    /**< compute from date#day, date#month, date#year and date#year_type */
  DI_FROM_SDN
    /**< compute from date#type, date#sdn1 and date#sdn2 */
};

  /** \brief Date input type for gedcom_normalize_date */
typedef enum _Date_input Date_input;
  
  /** \brief Parsed date

      This struct describes exactly one date.  It contains the string
      representation, the numeric representation and the representation using
      serial day numbers (aka Julian days).

      It is possible that the #year_str is given as e.g. "1677/78".  This is
      coming from a date in a so called "annunciation style", where the year
      began on 25 March: "20 March 1677/78" is 20 March 1677 in "annunciation
      style" and 20 March 1678 in "circumcision style" (the current style).
      See calendar overview (LINK TBD).

      In this case, the #year will contain the "circumcision style" year (1678
      in the example), and #year_type will be YEAR_DOUBLE.  Normal dates will
      have a #year_type equal to YEAR_SINGLE.

      Finally, the last three fields (#type, #sdn1 and #sdn2) are probably
      the most interesting values
      for applications that want to process dates.  Basically, the date is
      converted to a serial day number (aka Julian day), which is the unique
      day number since November 25, 4714 BC in the Gregorian calendar.  The
      advantage of these day numbers is that they are unique and independent
      of the calendar system.  Furthermore, date differences can just be
      computed by subtracting the serial day numbers.

      However, since dates in GEDCOM are not necessarily exact (e.g. "MAR
      1990"), it is not possible to represent all GEDCOM dates with 1 serial
      day number.  Two cases can be distinguished:

         - <b>Exact dates</b> (e.g. "25 MAR 1990"):\n\n
           These are represented by a serial day number in #sdn1 and a
	   #type equal to DATE_EXACT.

         - <b>Incomplete dates</b> (e.g. "MAR 1990"):\n\n
           These are represented by 2 serial day numbers (#sdn1 and #sdn2)
	   and a #type equal to DATE_BOUNDED.\n\n
           For example, the Gregorian date "MAR 1990" is represented by the
	   serial day numbers for "1 MAR 1990" and "31 MAR 1990", and the
	   Gregorian date "1990" is represented by the serial day numbers
	   for "1 JAN 1990" and "31 DEC 1990".  Similarly for the other
	   calendar types.
   */
struct date {
  Calendar_type cal;   /**< The calendar type */
  char day_str[MAX_DAY_LEN + 1];
                       /**< The literal string part of the date, as parsed from
			  the value in the GEDCOM file, that denotes
			  the day (can be an empty string) */
  char month_str[MAX_MONTH_LEN + 1];
                       /**< The literal string part of the date, as parsed from
			  the value in the GEDCOM file, that denotes the month
			  (can be an empty string) */
  char year_str[MAX_YEAR_LEN + 1];
                       /**< The literal string part of the date, as parsed from
			  the value in the GEDCOM file, that denotes the year
		       */
  int day;    /**< The numeric representation of the #day_str (starting from
		 1), or -1 if the #day_str is empty */
  int month;  /**< The month number of #month_str in the given calendar type
		 #cal (starting from 1), or -1 if the #month_str is empty */
  int year;   /**< The numeric representation of the #year_str */
  Year_type year_type;
              /**< The year type (see detailed description of \ref date)*/
  Date_type type;
              /**< The date type (see detailed description of \ref date) */
  long int sdn1; /**< The first serial day number */
  long int sdn2; /**< The second serial day number */
};

  /** \brief Date value

      The main struct describing a date value.  It depends on the first member,
      the type, which members are actually relevant.
   */
struct date_value {
  Date_value_type type;  /**< The type of date, which determines which of the
			    following members are relevant */
  struct date date1;                /**< First parsed date */
  struct date date2;                /**< Second parsed date */
  char phrase[MAX_PHRASE_LEN + 1];  /**< Free-format date phrase */
};
  /** @} */


  /** \brief Type for context handling, meant to be opaque
      \ingroup start_end
  */
typedef void* Gedcom_ctxt;

  /** \addtogroup parsed_xref */
  /** @{ */

  /** \brief Cross-reference type

      This determines what kind of object the cross-reference points to.
   */
enum _Xref_type {
  XREF_NONE,   /**< Used as a default value */
  XREF_FAM,    /**< Points to a family */
  XREF_INDI,   /**< Points to an individual */
  XREF_NOTE,   /**< Points to a note */
  XREF_OBJE,   /**< Points to a multimedia object */
  XREF_REPO,   /**< Points to a source repository */
  XREF_SOUR,   /**< Points to a source */
  XREF_SUBM,   /**< Points to a submitter */
  XREF_SUBN,   /**< Points to a submission record */
  XREF_USER,   /**< For application-specific cross-references */
  XREF_ANY     /**< If the type is not known: it has to come from further
		  information.  This is the case in an association
		  (\c ELT_SUB_ASSO): the type is then given by the \c TYPE
		  subtag.
	       */
};

  /** \brief Cross-reference type */
typedef enum _Xref_type Xref_type;

  /** \brief Cross-reference value

      The struct describing a cross-reference value.
   */
struct xref_value {
  Xref_type type;  /**< Determines what the cross-reference points to */
  char *string;    /**< The actual cross-reference string from the GEDCOM
		      file. */
  Gedcom_ctxt object;
                   /**< The referenced object.  This is initially \c NULL, but
		      can be filled by the application with an object (of any
		      type) that corresponds with the cross-reference, and then
		      later extracted when the cross-reference is used or
		      defined again in the file.  This relieves the application
		      from the burden of maintaining the mapping between
		      cross-references and objects. */
};
  /** @} */

  /** \addtogroup parsed_age */
  /** @{ */

  /** \brief Age value type

      This determines which members in the struct age_value are relevant, as
      given in the description below.
   */
enum _Age_type {
  AGE_UNRECOGNIZED,  /**< Format not recognized, full raw value in phrase */
  AGE_CHILD,         /**< CHILD, with given modifier */
  AGE_INFANT,        /**< INFANT, with given modifier */
  AGE_STILLBORN,     /**< STILLBORN, with given modifier */
  AGE_NUMERIC        /**< An indication in years, months and/or days (each can
			be -1 if not given), with given modifier */
};

  /** \brief Age value type */
typedef enum _Age_type Age_type;

  /** \brief Age modifier

      This gives a modifier on the given date (less than or greater than).
   */
enum _Age_modifier {
  AGE_NO_MODIFIER,   /**< No modifier */
  AGE_LESS_THAN,     /**< The modifier '<' is added */
  AGE_GREATER_THAN   /**< The modifier '>' is added */
};

  /** \brief Age modifier */
typedef enum _Age_modifier Age_modifier;

  /** \brief Age value

      The struct describing an age value.  It depends on the first member,
      the type, which members are actually relevant.
   */
struct age_value {
  Age_type type;       /**< The type of age, which determines which of the
			    following members are relevant */
  Age_modifier mod;    /**< A modifier (less than or greater than) */
  int years;           /**< The number of years */
  int months;          /**< The number of months */
  int days;            /**< The number of days */
  char phrase[MAX_PHRASE_LEN + 1];  /**< Free-format age phrase */
};
  /** @} */

  /** \addtogroup write */
  /** @{ */

  /** \brief The encoding width and endianness
      \sa gedcom_write_set_encoding */
enum _Encoding {
  ONE_BYTE      = 0x00, /**< This should be used for all character sets except
			    UNICODE */
  TWO_BYTE_HILO = 0x01, /**< High-low encoding for UNICODE (i.e. big-endian) */
  TWO_BYTE_LOHI = 0x02  /**< Low-high encoding for UNICODE (i.e. little-endian)
                         */
};

  /** \brief The encoding width and endianness */
typedef enum _Encoding Encoding;

  /** \brief The use of a byte-order-mark in the encoding
      \sa gedcom_write_set_encoding */
enum _Enc_bom {
  WITHOUT_BOM = 0x00,  /**< Without byte-order-mark */
  WITH_BOM    = 0x10   /**< With byte-order-mark */
};

  /** \brief The use of a byte-order-mark in the encoding */
typedef enum _Enc_bom Enc_bom;

  /** \brief The line terminator in the encoding
      \sa gedcom_write_set_line_terminator */
enum _Enc_line_end {
  END_CR    = 0, /**< Only carriage return ('\\r') (system value for Mac) */
  END_LF    = 1, /**< Only line feed ('\\n') (system value for Unix, Mac OSX)
                  */
  END_CR_LF = 2, /**< First carriage return, then line feed ('\\r\\n')
		    (system value for DOS, Windows) */
  END_LF_CR = 3  /**< First line feed, then carriage return ('\\n\\r') */
};

  /** \brief The line terminator in the encoding */
typedef enum _Enc_line_end Enc_line_end;

  /** \brief Source of the encoding settings
      \sa gedcom_write_set_encoding and gedcom_write_set_line_terminator */
enum _Enc_from {
  ENC_FROM_FILE = 0, /**< The same as the read file was in */
  ENC_FROM_SYS  = 1, /**< The system value */
  ENC_MANUAL    = 2  /**< From the parameters given in the function */
};

  /** \brief Source of the encoding settings */
typedef enum _Enc_from Enc_from;
  /** @} */

struct encoding_state;

  /** \addtogroup compat */
  /** @{ */
  /** \brief Compatibility mode options */
enum _Gedcom_compat {
  COMPAT_ALLOW_OUT_OF_CONTEXT = 0x01
    /**< In some compatibility cases, tags are coming out-of-order, i.e. their
       start element callback would have to come after the end element callback
       of the parent tag.  E.g. instead of the standard GEDCOM
       \code
         1 DATE ...
         2 TIME ...
       \endcode
       the genealogy program has generated something like:
       \code
         1 DATE ...
         1 TIME ...
       \endcode
       This can give a problem if your end element callbacks free some
       resources.  

       If your program can handle elements out of context, you can enable this
       option.  By default it is disabled, and so the values of these
       out-of-context tags are lost (the parser generates a warning if this is
       the case).
    */
};

  /** \brief Compatibility mode options */
typedef enum _Gedcom_compat Gedcom_compat;
  /** @} */

/**************************************************************************/
/***  Things meant to be internal, susceptible to changes               ***/
/***  Use the GEDCOM_STRING/GEDCOM_DATE interface instead of relying    ***/
/***  on this !!                                                        ***/
/**************************************************************************/

/* Update strings in interface.c if this changes */
typedef enum _GEDCOM_VAL_TYPE {
  GV_NULL       = 0x01,
  GV_CHAR_PTR   = 0x02,
  GV_DATE_VALUE = 0x04,
  GV_AGE_VALUE  = 0x08,
  GV_XREF_PTR   = 0x10
} Gedcom_val_type;

union _Gedcom_val_union {
  char* string_val;
  struct date_value date_val;
  struct age_value age_val;
  struct xref_value *xref_val;
};

typedef struct _Gedcom_val_struct {
  Gedcom_val_type type;
  union _Gedcom_val_union value;
} Gedcom_val_struct;

void gedcom_cast_error(const char* file, int line,
		       Gedcom_val_type tried_type,
		       Gedcom_val_type real_type);

extern struct date_value def_date_val;
extern struct age_value  def_age_val;
extern struct xref_value def_xref_val;

#define GV_CHECK_CAST(VAL, TYPE, MEMBER, DEFVAL)                              \
   (((VAL)->type == TYPE) ?                                                   \
    (VAL)->value.MEMBER :                                                     \
    (gedcom_cast_error(__FILE__,__LINE__, TYPE, (VAL)->type), DEFVAL))

#define GV_IS_TYPE(VAL, TYPE)                                                 \
   ((VAL)->type == TYPE)

/**************************************************************************/
/***  Function interface                                                ***/
/**************************************************************************/

  /** \brief Parsed value
      \ingroup parsed

      Type for the parsed value.  The struct type Gedcom_val_struct should
      be seen as internal: the Gedcom_val should be handled using the macros
      given above.
  */
typedef Gedcom_val_struct* Gedcom_val;

struct Gedcom_write_struct;
  /** \brief Write handle
      \ingroup write

      Handle for writing Gedcom files.  The struct type Gedcom_write_struct
      should be seen as internal: only the handle should be used.
   */
typedef struct Gedcom_write_struct* Gedcom_write_hndl; 

/* Check to determine whether there is a parsed value or not */  
#define GEDCOM_IS_NULL(VAL) \
   GV_IS_TYPE(VAL, GV_NULL)

/* This returns the char* from a Gedcom_val, if appropriate */
/* It gives a gedcom_warning if the cast is not correct     */
#define GEDCOM_STRING(VAL) \
   GV_CHECK_CAST(VAL, GV_CHAR_PTR, string_val, "<error>")
#define GEDCOM_IS_STRING(VAL) \
   GV_IS_TYPE(VAL, GV_CHAR_PTR)

/* This returns the struct date_value from a Gedcom_val, if appropriate */
/* It gives a gedcom_warning if the cast is not correct                 */
#define GEDCOM_DATE(VAL) \
   GV_CHECK_CAST(VAL, GV_DATE_VALUE, date_val, def_date_val)
#define GEDCOM_IS_DATE(VAL) \
   GV_IS_TYPE(VAL, GV_DATE_VALUE)

/* This returns the struct age_value from a Gedcom_val, if appropriate  */
/* It gives a gedcom_warning if the cast is not correct                 */
#define GEDCOM_AGE(VAL) \
   GV_CHECK_CAST(VAL, GV_AGE_VALUE, age_val, def_age_val)
#define GEDCOM_IS_AGE(VAL) \
   GV_IS_TYPE(VAL, GV_AGE_VALUE)

/* This returns the (struct xref_value *) from a Gedcom_val, if appropriate */
/* It gives a gedcom_warning if the cast is not correct                     */
#define GEDCOM_XREF_PTR(VAL) \
   GV_CHECK_CAST(VAL, GV_XREF_PTR, xref_val, &def_xref_val)
#define GEDCOM_IS_XREF_PTR(VAL) \
   GV_IS_TYPE(VAL, GV_XREF_PTR)

  /** \brief Message handler callback
      \ingroup error
    A callback for errors, warnings and messages.
    \sa gedcom_set_message_handler

    \param type The message type
    \param msg  For errors, this will have the format
      \code
        Error on line <lineno>: <actual_message>
      \endcode
      Note that the entire string will be properly internationalized, and
      encoded in UTF-8 (<a href=encoding.html>Why UTF-8?</a>).
      Also, no newline is appended, so that
      the application program can use it in any way it wants.  Warnings are
      similar, but use "Warning" instead of "Error".  Messages are plain
      text, without any prefix.
  */
typedef void
        (*Gedcom_msg_handler)
        (Gedcom_msg_type type, char *msg);

  /** \brief Record start callback
      \ingroup start_end
    A callback to handle the start of records.  A record entry in a GEDCOM
    file is always of the form <tt>"<level> <xref> <tag> <value>"</tt>, e.g.
    \code
      0 @REC003@ NOTE This is a note
    \endcode
    The different parameters of the callback return different parts of this
    entry.
    \sa gedcom_subscribe_to_record

    \param rec An enum identifying the record type (see the
    <a href="interface.html#Record_identifiers">interface details</a>)
    \param level The GEDCOM level (always 0 for records)
    \param xref The cross-reference key that identifies the record ("@REC003@"
    in the example above).
    \param tag The GEDCOM tag, in string format ("NOTE" in the example above).
    \param raw_value The value of the record, in string format, encoded as
    UTF-8 ("This is a note" in the example above)
    \param tag_value The GEDCOM tag, as symbolic value (e.g. \c TAG_NOTE in the
    example).  These values are defined in the header \c gedcom-tags.h that is
    installed, and included via \c gedcom.h (so no need to include
    \c gedcom-tags.h yourself).
    \param parsed_value The value of the record, in parsed format.

    \return A context to be used for this record (equivalent to a void
    pointer).  This context will be passed
    in all callbacks for direct child elements, and in the end callback for the
    record.
  */
typedef Gedcom_ctxt
        (*Gedcom_rec_start_cb)
        (Gedcom_rec rec, int level, Gedcom_val xref, char *tag,
         char *raw_value, int tag_value, Gedcom_val parsed_value);
  /** \brief Record end callback
      \ingroup start_end
    A callback to handle the end of records, i.e. this callback is called when
    all sub-elements of a record have been processed. 
    \sa gedcom_subscribe_to_record

    \param rec An enum identifying the record type (see the
    <a href="interface.html#Record_identifiers">interface details</a>)
    \param self The context as was given via the start callback.
    \param parsed_value The value of the record, in parsed format.  Here it is
    used to pass 'complete' values, e.g. the full text of a note including
    concatenated lines.
  */
typedef void
        (*Gedcom_rec_end_cb)
        (Gedcom_rec rec, Gedcom_ctxt self, Gedcom_val parsed_value);

  /** \brief Element start callback
      \ingroup start_end
    A callback to handle the start of elements.  An element in a GEDCOM
    file is part of a record, and is always of the form
    <tt>"<level> <tag> <value>"</tt>, e.g.
    \code
      1 DATE 1 JAN 1998
    \endcode
    The different parameters of the callback return different parts of this
    entry.
    \sa gedcom_subscribe_to_element

    \param elt An enum identifying the element type (see the
    <a href="interface.html#Element_identifiers">interface details</a>)
    \param parent The context of the parent element or record
    \param level The GEDCOM level (always bigger than 0 for elements)
    \param tag The GEDCOM tag, in string format ("DATE" in the example above).
    \param raw_value The value of the record, in string format, encoded as
    UTF-8 ("1 JAN 1998" in the example above)
    \param tag_value The GEDCOM tag, as symbolic value (e.g. \c TAG_DATE in the
    example).  These values are defined in the header \c gedcom-tags.h that is
    installed, and included via \c gedcom.h (so no need to include
    \c gedcom-tags.h yourself).
    \param parsed_value The value of the record, in parsed format.

    \return A context to be used for this element (equivalent to a void
    pointer).  This context will be passed
    in all callbacks for direct child elements, and in the end callback for the
    element.
  */
typedef Gedcom_ctxt
        (*Gedcom_elt_start_cb)
        (Gedcom_elt elt, Gedcom_ctxt parent,
	 int level, char *tag, char *raw_value,
	 int tag_value, Gedcom_val parsed_value);
  /** \brief Element end callback
      \ingroup start_end
    A callback to handle the end of element, i.e. this callback is called when
    all sub-elements of an element have been processed. 
    \sa gedcom_subscribe_to_element

    \param elt An enum identifying the element type (see the
    <a href="interface.html#Element_identifiers">interface details</a>)
    \param parent The context of the parent element or record.
    \param self The context as was given via the start callback of the element.
    \param parsed_value The value of the element, in parsed format.  Here it is
    used to pass 'complete' values, e.g. the full text of a note including
    concatenated lines.
  */
typedef void
        (*Gedcom_elt_end_cb)
        (Gedcom_elt elt, Gedcom_ctxt parent, Gedcom_ctxt self,
         Gedcom_val parsed_value);

  /** \brief Default callback
      \ingroup defcb
    The default callback, i.e. this callback is called when no specific
    callback has been registered. 
    \sa gedcom_set_default_callback

    \param elt An enum identifying the element type (see the
    <a href="interface.html#Element_identifiers">interface details</a>)
    \param parent The context of the parent element or record.
    \param level The GEDCOM level (always bigger than 0 for elements)
    \param tag The GEDCOM tag, in string format.
    \param raw_value The value of the record, in string format, encoded as
    UTF-8
    \param tag_value The GEDCOM tag, as symbolic value.  These values are
    defined in the header \c gedcom-tags.h that is
    installed, and included via \c gedcom.h (so no need to include
    \c gedcom-tags.h yourself).
  */
typedef void
        (*Gedcom_def_cb)
        (Gedcom_elt elt, Gedcom_ctxt parent, int level, char *tag,
         char *raw_value, int tag_value);

  /** \addtogroup maingedcom */
  /** @{ */
  /** \brief Initializes the Gedcom parser library */
int     gedcom_init();
  /** \brief Parses an existing Gedcom file */
int     gedcom_parse_file(const char* file_name);
  /** \brief Starts a new Gedcom model */
int     gedcom_new_model();
  /** @} */

  /** \addtogroup error */
  /** @{ */
  /** \brief Sets the error handler callback */
void    gedcom_set_message_handler(Gedcom_msg_handler func);
  /** \brief Determine what happens on an error */
void    gedcom_set_error_handling(Gedcom_err_mech mechanism);
  /** @} */

  /** \addtogroup debug */
  /** @{ */
  /** \brief Set the debugging level */
void    gedcom_set_debug_level(int level, FILE* trace_output);
  /** @} */

  /** \addtogroup compat */
  /** @{ */
  /** \brief Enable or disable compatibility mode */
void    gedcom_set_compat_handling(int enable_compat);
  /** \brief Set some options for the compatibility mode */
void    gedcom_set_compat_options(Gedcom_compat options);
  /** @} */
  
  /** \addtogroup defcb */
  /** @{ */
  /** \brief Set the default callback */
void    gedcom_set_default_callback(Gedcom_def_cb func);
  /** @} */

  /** \addtogroup start_end */
  /** @{ */
  /** \brief Subscribe to records, with given start and end callback */
void    gedcom_subscribe_to_record(Gedcom_rec rec,
				   Gedcom_rec_start_cb cb_start,
				   Gedcom_rec_end_cb cb_end);
  /** \brief Subscribe to elements, with given start and end callback */
void    gedcom_subscribe_to_element(Gedcom_elt elt,
				    Gedcom_elt_start_cb cb_start,
				    Gedcom_elt_end_cb cb_end);
  /** @} */

/* Separate value parsing functions */
  /** \addtogroup parsed_date */
  /** @{ */
  /** \brief Parse the given string into a date value */
struct date_value  gedcom_parse_date(const char* line_value);
  /** \brief Convert the given date value into a string */
char*              gedcom_date_to_string(const struct date_value* val);
  /** \brief Create a new date value */
struct date_value* gedcom_new_date_value(const struct date_value* copy_from);
  /** \brief Normalize the given date value */
int   gedcom_normalize_date(Date_input compute_from, struct date_value *val);
  /** @} */

  /** \addtogroup parsed_age */
  /** @{ */
  /** \brief Parse the given string into an age value */
struct age_value  gedcom_parse_age(const char* line_value);
  /** \brief Convert the given age value into a string */
char*             gedcom_age_to_string(const struct age_value* val);
  /** \brief Create a new age value */
struct age_value* gedcom_new_age_value(const struct age_value* copy_from);
  /** @} */

  /** \addtogroup parsed_xref */
  /** @{ */
  /** \brief Retrieve an xref_value using the cross-reference key */
struct xref_value *gedcom_get_by_xref(const char *key);
  /** \brief Add a cross-reference  */
struct xref_value *gedcom_add_xref(Xref_type type, const char* xrefstr,
                                   Gedcom_ctxt object);
  /** \brief Link via a cross-reference  */
struct xref_value *gedcom_link_xref(Xref_type type, const char* xrefstr);
  /** \brief Remove a link via a cross-reference  */
struct xref_value *gedcom_unlink_xref(Xref_type type, const char* xrefstr);
  /** \brief Delete a cross-reference  */
int                gedcom_delete_xref(const char* xrefstr);
  /** @} */

  /** \addtogroup write */
  /** @{ */
  /** \brief Open a file for writing GEDCOM */
Gedcom_write_hndl  gedcom_write_open(const char* filename);
  /** \brief Close the file */
int  gedcom_write_close(Gedcom_write_hndl hndl, int *total_conv_fails);
  /** \brief Set the encoding for writing GEDCOM files */
int  gedcom_write_set_encoding(Enc_from from,
			       const char* charset, Encoding width,
                               Enc_bom bom);
  /** \brief Set the line terminator for writing GEDCOM files */
int  gedcom_write_set_line_terminator(Enc_from from, Enc_line_end end);

  /** \brief Write a record line */
int  gedcom_write_record_str(Gedcom_write_hndl hndl,
			     Gedcom_rec rec, const char* xrefstr,
                             const char* val);
  
  /** \brief Write an element line, with string value */
int gedcom_write_element_str(Gedcom_write_hndl hndl, Gedcom_elt elt,
			     int parsed_tag, int parent_rec_or_elt,
			     const char* val);
  /** \brief Write an element line, with cross-reference value */
int gedcom_write_element_xref(Gedcom_write_hndl hndl, Gedcom_elt elt,
                              int parsed_tag, int parent_rec_or_elt,
			      const struct xref_value* val);

  /** \brief Write an element line, with date value */
int gedcom_write_element_date(Gedcom_write_hndl hndl,
			      Gedcom_elt elt, int tag, int parent_rec_or_elt,
			      const struct date_value* val);
  /** \brief Write an element line, with age value */
int gedcom_write_element_age(Gedcom_write_hndl hndl,
			     Gedcom_elt elt, int tag, int parent_rec_or_elt,
			     const struct age_value* val);

  /** \brief Write an user defined element line, with string value */
int gedcom_write_user_str(Gedcom_write_hndl hndl, int level, const char* tag,
			  const char* xrefstr, const char* value);
  /** \brief Write an user defined element line, with cross-reference value */
int gedcom_write_user_xref(Gedcom_write_hndl hndl, int level, const char* tag,
			   const char* xrefstr, const struct xref_value* val);
  /** @} */
  
/* For use in gom */
int        gedcom_error(const char* s, ...);
int        gedcom_warning(const char* s, ...);
int        gedcom_message(const char* s, ...);
int        gedcom_debug_print(const char* s, ...);

#ifdef __cplusplus
}
#endif

#endif /* __GEDCOM_H */
