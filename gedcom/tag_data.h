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

typedef enum _OPT {
  OPT_CONC = 0x01,
  OPT_CONT = 0x02
} Opt;

struct tag_data {
  char *elt_name;
  char *tag_name;
  int   allowed_types;
  Opt   options;
};

struct tag_data tag_data[NR_OF_ELTS] =
{
  /* REC_HEAD */
  { "REC_HEAD", "HEAD", GV_NULL, 0 },

  /* REC_FAM */
  { "REC_FAM", "FAM", GV_NULL, 0 },

  /* REC_INDI */
  { "REC_INDI", "INDI", GV_NULL, 0 },

  /* REC_OBJE */
  { "REC_OBJE", "OBJE", GV_NULL, 0 },

  /* REC_NOTE */
  { "REC_NOTE", "NOTE", GV_CHAR_PTR, 0 },

  /* REC_REPO */
  { "REC_REPO", "REPO", GV_NULL, 0 },

  /* REC_SOUR */
  { "REC_SOUR", "SOUR", GV_NULL, 0 },

  /* REC_SUBN */
  { "REC_SUBN", "SUBN", GV_NULL, 0 },

  /* REC_SUBM */
  { "REC_SUBM", "SUBM", GV_NULL, 0 },

  /* REC_USER */
  { "REC_USER", NULL,   GV_NULL | GV_CHAR_PTR | GV_XREF_PTR, 0 },

  /* ELT_HEAD_SOUR */
  { "ELT_HEAD_SOUR", "SOUR", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SOUR_VERS */
  { "ELT_HEAD_SOUR_VERS", "VERS", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SOUR_NAME */
  { "ELT_HEAD_SOUR_NAME", "NAME", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SOUR_CORP */
  { "ELT_HEAD_SOUR_CORP", "CORP", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SOUR_DATA */
  { "ELT_HEAD_SOUR_DATA", "DATA", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SOUR_DATA_DATE */
  { "ELT_HEAD_SOUR_DATA_DATE", "DATE", GV_DATE_VALUE, 0 },

  /* ELT_HEAD_SOUR_DATA_COPR */
  { "ELT_HEAD_SOUR_DATA_COPR", "COPR", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_DEST */
  { "ELT_HEAD_DEST", "DEST", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_DATE */
  { "ELT_HEAD_DATE", "DATE", GV_DATE_VALUE, 0 },

  /* ELT_HEAD_DATE_TIME */
  { "ELT_HEAD_DATE_TIME", "TIME", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_SUBM */
  { "ELT_HEAD_SUBM", "SUBM", GV_XREF_PTR, 0 },

  /* ELT_HEAD_SUBN */
  { "ELT_HEAD_SUBN", "SUBN", GV_XREF_PTR, 0 },

  /* ELT_HEAD_FILE */
  { "ELT_HEAD_FILE", "FILE", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_COPR */
  { "ELT_HEAD_COPR", "COPR", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_GEDC */
  { "ELT_HEAD_GEDC", "GEDC", GV_NULL, 0 },

  /* ELT_HEAD_GEDC_VERS */
  { "ELT_HEAD_GEDC_VERS", "VERS", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_GEDC_FORM */
  { "ELT_HEAD_GEDC_FORM", "FORM", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_CHAR */
  { "ELT_HEAD_CHAR", "CHAR", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_CHAR_VERS */
  { "ELT_HEAD_CHAR_VERS", "VERS", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_LANG */
  { "ELT_HEAD_LANG", "LANG", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_PLAC */
  { "ELT_HEAD_PLAC", "PLAC", GV_NULL, 0 },

  /* ELT_HEAD_PLAC_FORM */
  { "ELT_HEAD_PLAC_FORM", "FORM", GV_CHAR_PTR, 0 },

  /* ELT_HEAD_NOTE */
  { "ELT_HEAD_NOTE", "NOTE", GV_CHAR_PTR, OPT_CONC | OPT_CONT }
};
