#ifndef __EXTERNAL_H
#define __EXTERNAL_H

#include <stdio.h>

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
  LAST_REC
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
  ELT_USER,
  LAST_ELT
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

#endif /* __EXTERNAL_H */
