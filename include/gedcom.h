/* External header for the Gedcom parser library.
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

#ifndef __GEDCOM_H
#define __GEDCOM_H

#include <stdio.h>

__BEGIN_DECLS

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
  
  ELT_USER,
  
  NR_OF_ELTS     /* Just a final value to be used in array boundaries */
} Gedcom_elt;

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

typedef void* Gedcom_ctxt;
typedef void* Gedcom_val;

typedef void
        (*Gedcom_msg_handler)
        (Gedcom_msg_type type, char *msg);

typedef Gedcom_ctxt
        (*Gedcom_rec_start_cb)
        (int level, char *xref, char *tag);
typedef void
        (*Gedcom_rec_end_cb)
        (Gedcom_ctxt self);

typedef Gedcom_ctxt
        (*Gedcom_elt_start_cb)
        (Gedcom_ctxt parent,
	 int level, char *tag, char *raw_value, Gedcom_val parsed_value);
typedef void
        (*Gedcom_elt_end_cb)
        (Gedcom_ctxt parent, Gedcom_ctxt self, Gedcom_val parsed_value);

typedef void
        (*Gedcom_def_cb)
        (Gedcom_ctxt parent, int level, char *tag, char *raw_value);

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

__END_DECLS

#endif /* __GEDCOM_H */
