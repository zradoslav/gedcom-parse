/* Tag data header
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

#include "gedcom.h"
#include "gedcom.tabgen.h"

/* TODO: should be auto-generated */

#define TAG_NUM_START TAG_ABBR
#define TAG_NUM_END   TAG_WILL
char* tag_name[] =
{
  /* 265 */         "ABBR", "ADDR", "ADR1", "ADR2", "ADOP", "AFN" , "AGE",
  /* 272 */ "AGNC", "ALIA", "ANCE", "ANCI", "ANUL", "ASSO", "AUTH", "BAPL",
  /* 280 */ "BAPM", "BARM", "BASM", "BIRT", "BLES", "BLOB", "BURI", "CALN",
  /* 288 */ "CAST", "CAUS", "CENS", "CHAN", "CHAR", "CHIL", "CHR" , "CHRA",
  /* 296 */ "CITY", "CONC", "CONF", "CONL", "CONT", "COPR", "CORP", "CREM",
  /* 304 */ "CTRY", "DATA", "DATE", "DEAT", "DESC", "DESI", "DEST", "DIV",
  /* 312 */ "DIVF", "DSCR", "EDUC", "EMIG", "ENDL", "ENGA", "EVEN", "FAM",
  /* 320 */ "FAMC", "FAMF", "FAMS", "FCOM", "FILE", "FORM", "GEDC", "GIVN",
  /* 328 */ "GRAD", "HEAD", "HUSB", "IDNO", "IMMI", "INDI", "LANG", "LEGA",
  /* 336 */ "MARB", "MARC", "MARL", "MARR", "MARS", "MEDI", "NAME", "NATI",
  /* 344 */ "NATU", "NCHI", "NICK", "NMR",  "NOTE", "NPFX", "NSFX", "OBJE",
  /* 352 */ "OCCU", "ORDI", "ORDN", "PAGE", "PEDI", "PHON", "PLAC", "POST",
  /* 360 */ "PROB", "PROP", "PUBL", "QUAY", "REFN", "RELA", "RELI", "REPO",
  /* 368 */ "RESI", "RESN", "RETI", "RFN",  "RIN",  "ROLE", "SEX",  "SLGC",
  /* 376 */ "SLGS", "SOUR", "SPFX", "SSN",  "STAE", "STAT", "SUBM", "SUBN",
  /* 384 */ "SURN", "TEMP", "TEXT", "TIME", "TITL", "TRLR", "TYPE", "VERS",
  /* 392 */ "WIFE", "WILL", 0
};

typedef enum _OPT {
  OPT_CONC = 0x01,
  OPT_CONT = 0x02,
  OPT_CONT_AS_CONC = 0x04
} Opt;

struct tag_data {
  char *elt_name;
  int   tag;
  int   allowed_types;
  Opt   options;
};

struct tag_data tag_data[NR_OF_ELTS] =
{
  /* REC_HEAD */
  { "REC_HEAD", TAG_HEAD, GV_NULL, 0 },

  /* REC_FAM */
  { "REC_FAM", TAG_FAM, GV_NULL, 0 },

  /* REC_INDI */
  { "REC_INDI", TAG_INDI, GV_NULL, 0 },

  /* REC_OBJE */
  { "REC_OBJE", TAG_OBJE, GV_NULL, 0 },

  /* REC_NOTE */
  { "REC_NOTE", TAG_NOTE, GV_CHAR_PTR, OPT_CONC | OPT_CONT },

  /* REC_REPO */
  { "REC_REPO", TAG_REPO, GV_NULL, 0 },

  /* REC_SOUR */
  { "REC_SOUR", TAG_SOUR, GV_NULL, 0 },

  /* REC_SUBN */
  { "REC_SUBN", TAG_SUBN, GV_NULL, 0 },

  /* REC_SUBM */
  { "REC_SUBM", TAG_SUBM, GV_NULL, 0 },

  /* REC_USER */
  { "REC_USER", 0, GV_NULL | GV_CHAR_PTR | GV_XREF_PTR, 0 },

  /* ELT_HEAD_SOUR */
  { "ELT_HEAD_SOUR", TAG_SOUR, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SOUR_VERS */
  { "ELT_HEAD_SOUR_VERS", TAG_VERS, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SOUR_NAME */
  { "ELT_HEAD_SOUR_NAME", TAG_NAME, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SOUR_CORP */
  { "ELT_HEAD_SOUR_CORP", TAG_CORP, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SOUR_DATA */
  { "ELT_HEAD_SOUR_DATA", TAG_DATA, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SOUR_DATA_DATE */
  { "ELT_HEAD_SOUR_DATA_DATE", TAG_DATE, GV_DATE_VALUE, 0 },

  /* ELT_HEAD_SOUR_DATA_COPR */
  { "ELT_HEAD_SOUR_DATA_COPR", TAG_COPR, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_DEST */
  { "ELT_HEAD_DEST", TAG_DEST, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_DATE */
  { "ELT_HEAD_DATE", TAG_DATE, GV_DATE_VALUE, 0 },

  /* ELT_HEAD_DATE_TIME */
  { "ELT_HEAD_DATE_TIME", TAG_TIME, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SUBM */
  { "ELT_HEAD_SUBM", TAG_SUBM, GV_XREF_PTR, 0 },

  /* ELT_HEAD_SUBN */
  { "ELT_HEAD_SUBN", TAG_SUBN, GV_XREF_PTR, 0 },

  /* ELT_HEAD_FILE */
  { "ELT_HEAD_FILE", TAG_FILE, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_COPR */
  { "ELT_HEAD_COPR", TAG_COPR, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_GEDC */
  { "ELT_HEAD_GEDC", TAG_GEDC, GV_NULL, 0 },

  /* ELT_HEAD_GEDC_VERS */
  { "ELT_HEAD_GEDC_VERS", TAG_VERS, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_GEDC_FORM */
  { "ELT_HEAD_GEDC_FORM", TAG_FORM, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_CHAR */
  { "ELT_HEAD_CHAR", TAG_CHAR, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_CHAR_VERS */
  { "ELT_HEAD_CHAR_VERS", TAG_VERS, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_LANG */
  { "ELT_HEAD_LANG", TAG_LANG, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_PLAC */
  { "ELT_HEAD_PLAC", TAG_PLAC, GV_NULL, 0 },

  /* ELT_HEAD_PLAC_FORM */
  { "ELT_HEAD_PLAC_FORM", TAG_FORM, GV_CHAR_PTR, 0 },

  /* ELT_HEAD_NOTE */
  { "ELT_HEAD_NOTE", TAG_NOTE, GV_CHAR_PTR, OPT_CONC | OPT_CONT },

  /* ELT_FAM_HUSB */
  { "ELT_FAM_HUSB", TAG_HUSB, GV_XREF_PTR, 0 },

  /* ELT_FAM_WIFE */
  { "ELT_FAM_WIFE", TAG_WIFE, GV_XREF_PTR, 0 },

  /* ELT_FAM_CHIL */
  { "ELT_FAM_CHIL", TAG_CHIL, GV_XREF_PTR, 0 },

  /* ELT_FAM_NCHI */
  { "ELT_FAM_NCHI", TAG_NCHI, GV_CHAR_PTR, 0 },

  /* ELT_FAM_SUBM */
  { "ELT_FAM_SUBM", TAG_SUBM, GV_XREF_PTR, 0 },
  
  /* ELT_INDI_RESN */
  { "ELT_INDI_RESN", TAG_RESN, GV_CHAR_PTR, 0 },
  
  /* ELT_INDI_SEX */
  { "ELT_INDI_SEX", TAG_SEX, GV_CHAR_PTR, 0 },
  
  /* ELT_INDI_SUBM */
  { "ELT_INDI_SUBM", TAG_SUBM, GV_XREF_PTR, 0 },
  
  /* ELT_INDI_ALIA */
  { "ELT_INDI_ALIA", TAG_ALIA, GV_XREF_PTR, 0 },
  
  /* ELT_INDI_ANCI */
  { "ELT_INDI_ANCI", TAG_ANCI, GV_XREF_PTR, 0 },
  
  /* ELT_INDI_DESI */
  { "ELT_INDI_DESI", TAG_DESI, GV_XREF_PTR, 0 },
  
  /* ELT_INDI_RFN */
  { "ELT_INDI_RFN", TAG_RFN, GV_CHAR_PTR, 0 },
  
  /* ELT_INDI_AFN */
  { "ELT_INDI_AFN", TAG_AFN, GV_CHAR_PTR, 0 },
  
  /* ELT_OBJE_FORM */
  { "ELT_OBJE_FORM", TAG_FORM, GV_CHAR_PTR, 0 },
  
  /* ELT_OBJE_TITL */
  { "ELT_OBJE_TITL", TAG_TITL, GV_CHAR_PTR, 0 },
  
  /* ELT_OBJE_BLOB */
  { "ELT_OBJE_BLOB", TAG_BLOB, GV_NULL, OPT_CONT_AS_CONC },
  
  /* ELT_OBJE_BLOB_CONT */
  { "ELT_OBJE_BLOB_CONT", TAG_CONT, GV_CHAR_PTR, 0 },
  
  /* ELT_OBJE_OBJE */
  { "ELT_OBJE_OBJE", TAG_OBJE, GV_XREF_PTR, 0 },
  
  /* ELT_REPO_NAME */
  { "ELT_REPO_NAME", TAG_NAME, GV_CHAR_PTR, 0 },
  
  /* ELT_SOUR_DATA */
  { "ELT_SOUR_DATA", TAG_DATA, GV_NULL, 0 },
  
  /* ELT_SOUR_DATA_EVEN */
  { "ELT_SOUR_DATA_EVEN", TAG_EVEN, GV_CHAR_PTR, 0 },
  
  /* ELT_SOUR_DATA_EVEN_DATE */
  { "ELT_SOUR_DATA_EVEN_DATE", TAG_DATE, GV_DATE_VALUE, 0 },
  
  /* ELT_SOUR_DATA_EVEN_PLAC */
  { "ELT_SOUR_DATA_EVEN_PLAC", TAG_PLAC, GV_CHAR_PTR, 0 },
  
  /* ELT_SOUR_DATA_AGNC */
  { "ELT_SOUR_DATA_AGNC", TAG_AGNC, GV_CHAR_PTR, 0 },
  
  /* ELT_SOUR_AUTH */
  { "ELT_SOUR_AUTH", TAG_AUTH, GV_CHAR_PTR, OPT_CONC | OPT_CONT },
  
  /* ELT_SOUR_TITL */
  { "ELT_SOUR_TITL", TAG_TITL, GV_CHAR_PTR, OPT_CONC | OPT_CONT },
  
  /* ELT_SOUR_ABBR */
  { "ELT_SOUR_ABBR", TAG_ABBR, GV_CHAR_PTR, 0 },
  
  /* ELT_SOUR_PUBL */
  { "ELT_SOUR_PUBL", TAG_PUBL, GV_CHAR_PTR, OPT_CONC | OPT_CONT },
  
  /* ELT_SOUR_TEXT */
  { "ELT_SOUR_TEXT", TAG_TEXT, GV_CHAR_PTR, OPT_CONC | OPT_CONT },
  
  /* ELT_SUBN_SUBM */
  { "ELT_SUBN_SUBM", TAG_SUBM, GV_XREF_PTR, 0 },
  
  /* ELT_SUBN_FAMF */
  { "ELT_SUBN_FAMF", TAG_FAMF, GV_CHAR_PTR, 0 },
  
  /* ELT_SUBN_TEMP */
  { "ELT_SUBN_TEMP", TAG_TEMP, GV_CHAR_PTR, 0 },
  
  /* ELT_SUBN_ANCE */
  { "ELT_SUBN_ANCE", TAG_ANCE, GV_CHAR_PTR, 0 },
  
  /* ELT_SUBN_DESC */
  { "ELT_SUBN_DESC", TAG_DESC, GV_CHAR_PTR, 0 },
  
  /* ELT_SUBN_ORDI */
  { "ELT_SUBN_ORDI", TAG_ORDI, GV_CHAR_PTR, 0 },
  
  /* ELT_SUBN_RIN */
  { "ELT_SUBN_RIN", TAG_RIN, GV_CHAR_PTR, 0 },
  
  /* ELT_SUBM_NAME */
  { "ELT_SUBM_NAME", TAG_NAME, GV_CHAR_PTR, 0 },
  
  /* ELT_SUBM_LANG */
  { "ELT_SUBM_LANG", TAG_LANG, GV_CHAR_PTR, 0 },
  
  /* ELT_SUBM_RFN */
  { "ELT_SUBM_RFN", TAG_RFN, GV_CHAR_PTR, 0 },
  
  /* ELT_SUBM_RIN */
  { "ELT_SUBM_RIN", TAG_RIN, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_ADDR */
  { "ELT_SUB_ADDR", TAG_ADDR, GV_CHAR_PTR, OPT_CONT },
  
  /* ELT_SUB_ADDR_CONT */
  { "ELT_SUB_ADDR_CONT", TAG_CONT, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_ADDR_ADR1 */
  { "ELT_SUB_ADDR_ADR1", TAG_ADR1, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_ADDR_ADR2 */
  { "ELT_SUB_ADDR_ADR2", TAG_ADR2, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_ADDR_CITY */
  { "ELT_SUB_ADDR_CITY", TAG_CITY, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_ADDR_STAE */
  { "ELT_SUB_ADDR_STAE", TAG_STAE, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_ADDR_POST */
  { "ELT_SUB_ADDR_POST", TAG_POST, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_ADDR_CTRY */
  { "ELT_SUB_ADDR_CTRY", TAG_CTRY, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_PHON */
  { "ELT_SUB_PHON", TAG_PHON, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_ASSO */
  { "ELT_SUB_ASSO", TAG_ASSO, GV_XREF_PTR, 0 },
  
  /* ELT_SUB_ASSO_TYPE */
  { "ELT_SUB_ASSO_TYPE", TAG_TYPE, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_ASSO_RELA */
  { "ELT_SUB_ASSO_RELA", TAG_RELA, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_CHAN */
  { "ELT_SUB_CHAN", TAG_CHAN, GV_NULL, 0 },
  
  /* ELT_SUB_CHAN_DATE */
  { "ELT_SUB_CHAN_DATE", TAG_DATE, GV_DATE_VALUE, 0 },
  
  /* ELT_SUB_CHAN_TIME */
  { "ELT_SUB_CHAN_TIME", TAG_TIME, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_FAMC */
  { "ELT_SUB_FAMC", TAG_FAMC, GV_XREF_PTR, 0 },
  
  /* ELT_SUB_FAMC_PEDI */
  { "ELT_SUB_FAMC_PEDI", TAG_PEDI, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_CONT */
  { "ELT_SUB_CONT", TAG_CONT, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_CONC */
  { "ELT_SUB_CONC", TAG_CONC, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_EVT_TYPE */
  { "ELT_SUB_EVT_TYPE", TAG_TYPE, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_EVT_DATE */
  { "ELT_SUB_EVT_DATE", TAG_DATE, GV_DATE_VALUE, 0 },
  
  /* ELT_SUB_EVT_AGE */
  { "ELT_SUB_EVT_AGE", TAG_AGE, GV_AGE_VALUE, 0 },
  
  /* ELT_SUB_EVT_AGNC */
  { "ELT_SUB_EVT_AGNC", TAG_AGNC, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_EVT_CAUS */
  { "ELT_SUB_EVT_CAUS", TAG_CAUS, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_FAM_EVT */
  { "ELT_SUB_FAM_EVT", 0, GV_NULL | GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_FAM_EVT_HUSB */
  { "ELT_SUB_FAM_EVT_HUSB", TAG_HUSB, GV_NULL, 0 },
  
  /* ELT_SUB_FAM_EVT_WIFE */
  { "ELT_SUB_FAM_EVT_WIFE", TAG_WIFE, GV_NULL, 0 },
  
  /* ELT_SUB_FAM_EVT_AGE */
  { "ELT_SUB_FAM_EVT_AGE", TAG_AGE, GV_AGE_VALUE, 0 },
  
  /* ELT_SUB_FAM_EVT_EVEN */
  { "ELT_SUB_FAM_EVT_EVEN", TAG_EVEN, GV_NULL, 0 },
  
  /* ELT_SUB_IDENT_REFN */
  { "ELT_SUB_IDENT_REFN", TAG_REFN, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_IDENT_REFN_TYPE */
  { "ELT_SUB_IDENT_REFN_TYPE", TAG_TYPE, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_IDENT_RIN */
  { "ELT_SUB_IDENT_RIN", TAG_RIN, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_INDIV_ATTR */
  { "ELT_SUB_INDIV_ATTR", 0, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_INDIV_RESI */
  { "ELT_SUB_INDIV_RESI", TAG_RESI, GV_NULL, 0 },
  
  /* ELT_SUB_INDIV_BIRT */
  { "ELT_SUB_INDIV_BIRT", 0, GV_NULL | GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_INDIV_BIRT_FAMC */
  { "ELT_SUB_INDIV_BIRT_FAMC", TAG_FAMC, GV_XREF_PTR, 0 },
  
  /* ELT_SUB_INDIV_GEN */
  { "ELT_SUB_INDIV_GEN", 0, GV_NULL | GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_INDIV_ADOP */
  { "ELT_SUB_INDIV_ADOP", TAG_ADOP, GV_NULL | GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_INDIV_ADOP_FAMC */
  { "ELT_SUB_INDIV_ADOP_FAMC", TAG_FAMC, GV_XREF_PTR, 0 },
  
  /* ELT_SUB_INDIV_ADOP_FAMC_ADOP */
  { "ELT_SUB_INDIV_ADOP_FAMC_ADOP", TAG_ADOP, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_INDIV_EVEN */
  { "ELT_SUB_INDIV_EVEN", TAG_EVEN, GV_NULL, 0 },
  
  /* ELT_SUB_LIO_BAPL */
  { "ELT_SUB_LIO_BAPL", 0, GV_NULL, 0 },
  
  /* ELT_SUB_LIO_BAPL_STAT */
  { "ELT_SUB_LIO_BAPL_STAT", TAG_STAT, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_LIO_BAPL_DATE */
  { "ELT_SUB_LIO_BAPL_DATE", TAG_DATE, GV_DATE_VALUE, 0 },
  
  /* ELT_SUB_LIO_BAPL_TEMP */
  { "ELT_SUB_LIO_BAPL_TEMP", TAG_TEMP, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_LIO_BAPL_PLAC */
  { "ELT_SUB_LIO_BAPL_PLAC", TAG_PLAC, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_LIO_SLGC */
  { "ELT_SUB_LIO_SLGC", TAG_SLGC, GV_NULL, 0 },
  
  /* ELT_SUB_LIO_SLGC_FAMC */
  { "ELT_SUB_LIO_SLGC_FAMC", TAG_FAMC, GV_XREF_PTR, 0 },
  
  /* ELT_SUB_LSS_SLGS */
  { "ELT_SUB_LSS_SLGS", TAG_SLGS, GV_NULL, 0 },
  
  /* ELT_SUB_LSS_SLGS_STAT */
  { "ELT_SUB_LSS_SLGS_STAT", TAG_STAT, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_LSS_SLGS_DATE */
  { "ELT_SUB_LSS_SLGS_DATE", TAG_DATE, GV_DATE_VALUE, 0 },
  
  /* ELT_SUB_LSS_SLGS_TEMP */
  { "ELT_SUB_LSS_SLGS_TEMP", TAG_TEMP, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_LSS_SLGS_PLAC */
  { "ELT_SUB_LSS_SLGS_PLAC", TAG_PLAC, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_MULTIM_OBJE */
  { "ELT_SUB_MULTIM_OBJE", TAG_OBJE, GV_NULL | GV_XREF_PTR, 0 },
  
  /* ELT_SUB_MULTIM_OBJE_FORM */
  { "ELT_SUB_MULTIM_OBJE_FORM", TAG_FORM, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_MULTIM_OBJE_TITL */
  { "ELT_SUB_MULTIM_OBJE_TITL", TAG_TITL, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_MULTIM_OBJE_FILE */
  { "ELT_SUB_MULTIM_OBJE_FILE", TAG_FILE, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_NOTE */
  { "ELT_SUB_NOTE", TAG_NOTE, GV_NULL | GV_CHAR_PTR | GV_XREF_PTR,
    OPT_CONT | OPT_CONC },
  
  /* ELT_SUB_PERS_NAME */
  { "ELT_SUB_PERS_NAME", TAG_NAME, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_PERS_NAME_NPFX */
  { "ELT_SUB_PERS_NAME_NPFX", TAG_NPFX, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_PERS_NAME_GIVN */
  { "ELT_SUB_PERS_NAME_GIVN", TAG_GIVN, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_PERS_NAME_NICK */
  { "ELT_SUB_PERS_NAME_NICK", TAG_NICK, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_PERS_NAME_SPFX */
  { "ELT_SUB_PERS_NAME_SPFX", TAG_SPFX, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_PERS_NAME_SURN */
  { "ELT_SUB_PERS_NAME_SURN", TAG_SURN, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_PERS_NAME_NSFX */
  { "ELT_SUB_PERS_NAME_NSFX", TAG_NSFX, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_PLAC */
  { "ELT_SUB_PLAC", TAG_PLAC, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_PLAC_FORM */
  { "ELT_SUB_PLAC_FORM", TAG_FORM, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_SOUR */
  { "ELT_SUB_SOUR", TAG_SOUR, GV_CHAR_PTR | GV_XREF_PTR, OPT_CONT | OPT_CONC },
  
  /* ELT_SUB_SOUR_PAGE */
  { "ELT_SUB_SOUR_PAGE", TAG_PAGE, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_SOUR_EVEN */
  { "ELT_SUB_SOUR_EVEN", TAG_EVEN, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_SOUR_EVEN_ROLE */
  { "ELT_SUB_SOUR_EVEN_ROLE", TAG_ROLE, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_SOUR_DATA */
  { "ELT_SUB_SOUR_DATA", TAG_DATA, GV_NULL, 0 },
  
  /* ELT_SUB_SOUR_DATA_DATE */
  { "ELT_SUB_SOUR_DATA_DATE", TAG_DATE, GV_DATE_VALUE, 0 },
  
  /* ELT_SUB_SOUR_TEXT */
  { "ELT_SUB_SOUR_TEXT", TAG_TEXT, GV_CHAR_PTR, OPT_CONT | OPT_CONC },
  
  /* ELT_SUB_SOUR_QUAY */
  { "ELT_SUB_SOUR_QUAY", TAG_QUAY, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_REPO */
  { "ELT_SUB_REPO", TAG_REPO, GV_XREF_PTR, 0 },
  
  /* ELT_SUB_REPO_CALN */
  { "ELT_SUB_REPO_CALN", TAG_CALN, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_REPO_CALN_MEDI */
  { "ELT_SUB_REPO_CALN_MEDI", TAG_MEDI, GV_CHAR_PTR, 0 },
  
  /* ELT_SUB_FAMS */
  { "ELT_SUB_FAMS", TAG_FAMS, GV_XREF_PTR, 0 },
  
  /* ELT_USER */
  { "ELT_USER", 0, GV_NULL | GV_CHAR_PTR | GV_XREF_PTR, 0 },
  
};
