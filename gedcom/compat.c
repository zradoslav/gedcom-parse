/* Compatibility handling for the GEDCOM parser.
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

#include "compat.h"
#include "interface.h"
#include "encoding.h"
#include "xref.h"
#include "buffer.h"
#include "gedcom_internal.h"
#include "gedcom.h"

int compat_enabled = 1;
Gedcom_compat compat_options = 0;
int compatibility  = 0;
int compatibility_program = 0;
int compatibility_version = 0;
const char* default_charset = "";

#define SUBMITTER_LINK         "@__COMPAT__SUBM__@"
#define SLGC_FAMC_LINK         "@__COMPAT__FAM_SLGC__@"
#define DEFAULT_SUBMITTER_NAME "Submitter"
#define DEFAULT_GEDCOM_VERS    "5.5"
#define DEFAULT_GEDCOM_FORM    "LINEAGE-LINKED"

struct program_data {
  const char* name;
  int         default_compat;
  const char* default_charset;
};

enum _COMPAT_PROGRAM {
  CP_FTREE = 1,
  CP_LIFELINES,
  CP_PAF,
  CP_FAMORIG,
  CP_EASYTREE
};

enum _COMPAT {
  C_FTREE        = 0x0001,
  C_LIFELINES    = 0x0002,
  C_PAF5         = 0x0004,
  C_PAF2         = 0x0008,
  C_FAMORIG      = 0x0010,
  C_EASYTREE     = 0x0020,
  C_PAF4         = 0x0040
};

struct program_data data[] = {
  /* NULL */         { "", 0, "" },
  /* CP_FTREE */     { "ftree", C_FTREE, "" },
  /* CP_LIFELINES */ { "Lifelines", C_LIFELINES, "ANSI" },
  /* CP_PAF */       { "Personal Ancestral File", C_PAF5, "" },
  /* CP_FAMORIG */   { "Family Origins", C_FAMORIG, "" },
  /* CP_EASYTREE */  { "EasyTree", C_EASYTREE, "" }
};

/* Incompatibility list (with GEDCOM 5.5):

    - ftree:
        - no submitter record, no submitter link in the header
	- INDI.ADDR instead of INDI.RESI.ADDR
	- NOTE doesn't have a value

    - Lifelines (3.0.2):
        - no submitter record, no submitter link in the header
	- no GEDC field in the header
	- no CHAR field in the header
	- HEAD.TIME instead of HEAD.DATE.TIME (will be ignored here)
	- '@' not written as '@@' in values
	- lots of missing required values

    - Personal Ancestral File 5:
        - '@' not written as '@@' in values
	- some 5.5.1 (draft) tags are used: EMAIL, FONE, ROMN
	- no FAMC field in SLGC
	- uses tab character (will be converted to 8 spaces here)
	- lines too long
	- non-standard date formats

    - Personal Ancestral File 2:
        - '@' not written as '@@' in values
	- COMM tag in submitter record
	- double dates written as e.g. '1815/1816' instead of '1815/16'
	- non-standard date formats

    - Family Origins:
        - '@' not written as '@@' in values
	- CONC needs an extra space

    - EasyTree:
        - no GEDC.FORM field
	- no submitter link in the header
	- NOTE doesn't have a value
	- NOTE.NOTE instead of NOTE.COND
	- NOTE.CONC.SOUR instead of NOTE.SOUR
	- non-standard tags in SOUR records

    - Personal Ancestral File 4:
        - '@' not written as '@@' in values
	- SUBM.CTRY instead of SUBM.ADDR.CTRY
	- lines too long
	- non-standard date formats
 */

int compat_matrix[] =
{
  /* C_NO_SUBMITTER */        C_FTREE | C_LIFELINES | C_PAF2 | C_EASYTREE,
  /* C_INDI_ADDR */           C_FTREE,
  /* C_NOTE_NO_VALUE */       C_FTREE | C_EASYTREE,
  /* C_NO_GEDC */             C_LIFELINES | C_PAF2,
  /* C_NO_CHAR */             C_LIFELINES,
  /* C_HEAD_TIME */           C_LIFELINES,
  /* C_NO_DOUBLE_AT */        C_LIFELINES | C_PAF5 | C_PAF2 | C_FAMORIG
                               | C_PAF4,
  /* C_NO_REQUIRED_VALUES */  C_LIFELINES | C_PAF5 | C_EASYTREE,
  /* C_551_TAGS */            C_PAF5,
  /* C_NO_SLGC_FAMC */        C_PAF5,
  /* C_SUBM_COMM */           C_PAF2,
  /* C_DOUBLE_DATES_4 */      C_PAF2 | C_PAF5 | C_PAF4,
  /* C_CONC_NEEDS_SPACE */    C_FAMORIG,
  /* C_NO_GEDC_FORM */        C_EASYTREE,
  /* C_NOTE_NOTE */           C_EASYTREE,
  /* C_TAB_CHARACTER */       C_PAF5,
  /* C_SUBM_CTRY */           C_PAF4,
  /* C_NOTE_TOO_LONG */       C_PAF4 | C_PAF5,
  /* C_NOTE_CONC_SOUR */      C_EASYTREE,
  /* C_NONSTD_SOUR_TAGS */    C_EASYTREE,
  /* C_PAF_DATES */           C_PAF2 | C_PAF4 | C_PAF5
};

union _COMPAT_STATE {
  int i;
  void* vp;
} compat_state[C_NR_OF_RULES];

/* Compatibility handling */

void gedcom_set_compat_handling(int enable_compat)
{
  compat_enabled = enable_compat;
}

void gedcom_set_compat_options(Gedcom_compat options)
{
  compat_options = options;
}

void enable_compat_msg(const char* program_name, int version)
{
  if (version > 0)
    gedcom_warning(_("Enabling compatibility with '%s', version %d"),
		   program_name, version);
  else
    gedcom_warning(_("Enabling compatibility with '%s'"),
		   program_name);
}

int program_equal(const char* program, const char* compare)
{
  return !strncmp(program, compare, strlen(compare)+1);
}

int program_equal_continued(const char* program, const char* compare)
{
  size_t len = strlen(compare);
  int result = strncmp(program, compare, len);
  if (result == 0) {
    if (strlen(program) > len)
      set_compatibility_version(program + len);
  }
  return !result;
}

void set_compatibility_program(const char* program)
{
  compatibility_program = 0;
  if (compat_enabled) {
    if (program_equal(program, "ftree")) {
      compatibility_program = CP_FTREE;
    }
    else if (program_equal_continued(program, "LIFELINES")) {
      compatibility_program = CP_LIFELINES;
    }
    else if (program_equal_continued(program, "PAF")) {
      compatibility_program = CP_PAF;
    }
    else if (program_equal(program, "FamilyOrigins")) {
      compatibility_program = CP_FAMORIG;
    }
    else if (program_equal(program, "EasyTree")) {
      compatibility_program = CP_EASYTREE;
    }
  }
}

void compute_compatibility()
{
  /* Reinitialize compatibility */
  int i;
  int version = 0;
  default_charset = "";
  compatibility = 0;
  for (i = 0; i < C_NR_OF_RULES; i++)
    compat_state[i].i = 0;

  switch (compatibility_program) {
    case CP_PAF:
      if (compatibility_version >= 20000 && compatibility_version < 30000) {
	compatibility = C_PAF2;
	version = 2;
      }
      if (compatibility_version >= 40000 && compatibility_version < 50000) {
	compatibility = C_PAF4;
	version = 4;
      }
      else if (compatibility_version >= 50000) {
	compatibility = C_PAF5;
	version = 5;
      }
      break;
    default:
      compatibility = data[compatibility_program].default_compat;
      break;
  }
  if (compatibility) {
    default_charset = data[compatibility_program].default_charset;
    enable_compat_msg(data[compatibility_program].name, version);
  }
}

void set_compatibility_version(const char* version)
{
  if (compat_enabled) {
    unsigned int major=0, minor=0, patch=0;
    int result;
    
    result = sscanf(version, " %u.%u.%u", &major, &minor, &patch);
    if (result > 0) {
      gedcom_debug_print("Setting compat version to %u.%u.%u",
			 major, minor, patch);
      compatibility_version = major * 10000 + minor * 100 + patch;
    }
  }
}

int compat_mode(Compat_rule rule)
{
  return (compat_matrix[rule] & compatibility);
}

/********************************************************************/
/*  C_NO_SUBMITTER                                                  */
/********************************************************************/

void compat_generate_submitter_link(Gedcom_ctxt parent)
{
  struct xref_value *xr = gedcom_parse_xref(SUBMITTER_LINK, XREF_USED,
					    XREF_SUBM);
  struct tag_struct ts;
  Gedcom_ctxt self;
  
  ts.string = "SUBM";
  ts.value  = TAG_SUBM;
  gedcom_warning(_("Adding link to submitter record with xref '%s'"),
		 SUBMITTER_LINK);
  self = start_element(ELT_HEAD_SUBM,
		       parent, 1, ts, SUBMITTER_LINK,
		       GEDCOM_MAKE_XREF_PTR(val1, xr));
  end_element(ELT_HEAD_SUBM, parent, self, NULL);
  compat_state[C_NO_SUBMITTER].i = 1;
}

void compat_generate_submitter()
{
  if (compat_state[C_NO_SUBMITTER].i) {
    struct xref_value *xr = gedcom_parse_xref(SUBMITTER_LINK, XREF_DEFINED,
					      XREF_SUBM);
    struct tag_struct ts;
    Gedcom_ctxt self1, self2;
    
    /* first generate "0 SUBM" */
    ts.string = "SUBM";
    ts.value  = TAG_SUBM;
    self1 = start_record(REC_SUBM, 0, GEDCOM_MAKE_XREF_PTR(val1, xr), ts,
			 NULL, GEDCOM_MAKE_NULL(val2));
    
    /* then generate "1 NAME ..." */
    ts.string = "NAME";
    ts.value  = TAG_NAME;
    self2 = start_element(ELT_SUBM_NAME, self1, 1, ts, DEFAULT_SUBMITTER_NAME,
			  GEDCOM_MAKE_STRING(val1, DEFAULT_SUBMITTER_NAME));
    
    /* close "1 NAME ..." */
    end_element(ELT_SUBM_NAME, self1, self2, NULL);
    
    /* close "0 SUBM" */
    end_record(REC_SUBM, self1, NULL);
    compat_state[C_NO_SUBMITTER].i = 0;
  }
}

/********************************************************************/
/*  C_NO_GEDC                                                       */
/********************************************************************/

void compat_generate_gedcom(Gedcom_ctxt parent)
{
  struct tag_struct ts;
  Gedcom_ctxt self1, self2;
  
  /* first generate "1 GEDC" */
  ts.string = "GEDC";
  ts.value  = TAG_GEDC;
  self1 = start_element(ELT_HEAD_GEDC, parent, 1, ts, NULL,
			GEDCOM_MAKE_NULL(val1));
  
  /* then generate "2 VERS <DEFAULT_GEDC_VERS>" */
  ts.string = "VERS";
  ts.value  = TAG_VERS;
  self2 = start_element(ELT_HEAD_GEDC_VERS, self1, 2, ts,
			DEFAULT_GEDCOM_VERS,
			GEDCOM_MAKE_STRING(val1, DEFAULT_GEDCOM_VERS));
  
  /* close "2 VERS" */
  end_element(ELT_HEAD_GEDC_VERS, self1, self2, NULL);
  
  /* then generate "2 FORM <DEFAULT_GEDCOM_FORM> */
  compat_generate_gedcom_form(self1);
  
  /* close "1 GEDC" */
  end_element(ELT_HEAD_GEDC, parent, self1, NULL);
}

/********************************************************************/
/*  C_NO_GEDC_FORM                                                  */
/********************************************************************/

void compat_generate_gedcom_form(Gedcom_ctxt parent)
{
  struct tag_struct ts;
  Gedcom_ctxt self;
  
  /* generate "2 FORM <DEFAULT_GEDCOM_FORM> */
  ts.string = "FORM";
  ts.value  = TAG_FORM;
  self = start_element(ELT_HEAD_GEDC_FORM, parent, 2, ts,
		       DEFAULT_GEDCOM_FORM,
		       GEDCOM_MAKE_STRING(val1, DEFAULT_GEDCOM_FORM));
  
  /* close "2 FORM" */
  end_element(ELT_HEAD_GEDC_FORM, parent, self, NULL);
  
}
  
/********************************************************************/
/*  C_NO_CHAR                                                       */
/********************************************************************/

int compat_generate_char(Gedcom_ctxt parent)
{
  struct tag_struct ts;
  Gedcom_ctxt self1;
  char* charset;
  
  /* first generate "1 CHAR <DEFAULT_CHAR>" */
  ts.string = "CHAR";
  ts.value  = TAG_CHAR;

  /* Must strdup, because default_charset is const char */
  charset   = strdup(default_charset);
  if (! charset)
    MEMORY_ERROR;
  else {
    self1 = start_element(ELT_HEAD_CHAR, parent, 1, ts, charset,
			  GEDCOM_MAKE_STRING(val1, charset));
    free(charset);
    
    /* close "1 CHAR" */
    end_element(ELT_HEAD_CHAR, parent, self1, NULL);
  }
  if (open_conv_to_internal(default_charset) == 0)
    return 1;
  else
    return 0;
}

/********************************************************************/
/*  C_HEAD_TIME                                                     */
/********************************************************************/

void compat_save_head_date_context(Gedcom_ctxt parent)
{
  compat_state[C_HEAD_TIME].vp = parent;
}

Gedcom_ctxt compat_generate_head_time_start(int level, struct tag_struct ts,
					    char* value)
{
  if (compat_options & COMPAT_ALLOW_OUT_OF_CONTEXT) {
    Gedcom_ctxt parent = compat_state[C_HEAD_TIME].vp;
    if (!value)
      value = "-";
    if (parent)
      return start_element(ELT_HEAD_DATE_TIME,
			   parent, level, ts, value,
			   GEDCOM_MAKE_STRING(val1, value));
    else
      return NULL;
  }
  else {
    gedcom_warning(_("Header change time '%s' lost in the compatibility (out of context)"),
		   value);
    return NULL;
  }
}

void compat_generate_head_time_end(Gedcom_ctxt self)
{
  if (compat_options & COMPAT_ALLOW_OUT_OF_CONTEXT) {
    Gedcom_ctxt parent = compat_state[C_HEAD_TIME].vp;
    if (parent)
      end_element(ELT_HEAD_DATE_TIME,
		  parent, self, GEDCOM_MAKE_NULL(val1));
  }
}

/********************************************************************/
/*  C_SUBM_CTRY                                                     */
/********************************************************************/

void compat_save_ctry_parent_context(Gedcom_ctxt parent)
{
  compat_state[C_SUBM_CTRY].vp = parent;
}

Gedcom_ctxt compat_generate_addr_ctry_start(int level, struct tag_struct ts,
					    char* value)
{
  if (compat_options & COMPAT_ALLOW_OUT_OF_CONTEXT) {
    Gedcom_ctxt parent = compat_state[C_SUBM_CTRY].vp;
    if (!value)
      value = "-";
    if (parent)
      return start_element(ELT_SUB_ADDR_CTRY,
			   parent, level, ts, value,
			   GEDCOM_MAKE_STRING(val1, value));
    else
      return NULL;
  }
  else {
    gedcom_warning(_("Country '%s' lost in the compatibility (out of context)"), value);
    return NULL;
  }
}

void compat_generate_addr_ctry_end(Gedcom_ctxt self)
{
  if (compat_options & COMPAT_ALLOW_OUT_OF_CONTEXT) {
    Gedcom_ctxt parent = compat_state[C_SUBM_CTRY].vp;
    if (parent)
      end_element(ELT_SUB_ADDR_CTRY,
		  parent, self, GEDCOM_MAKE_NULL(val1));
  }
}

void compat_free_ctry_parent_context()
{
  compat_state[C_SUBM_CTRY].vp = NULL;
}

/********************************************************************/
/*  C_INDI_ADDR                                                     */
/********************************************************************/

Gedcom_ctxt compat_generate_resi_start(Gedcom_ctxt parent)
{
  Gedcom_ctxt self;
  struct tag_struct ts;

  ts.string = "RESI";
  ts.value  = TAG_RESI;
  self = start_element(ELT_SUB_INDIV_RESI, parent, 1, ts, NULL,
		       GEDCOM_MAKE_NULL(val1));
  return self;
}

void compat_generate_resi_end(Gedcom_ctxt parent, Gedcom_ctxt self)
{
  end_element(ELT_SUB_INDIV_RESI, parent, self, NULL);
}

/********************************************************************/
/*  C_551_TAGS                                                      */
/********************************************************************/

int is_551_tag(const char* tag)
{
  if (strncmp(tag, "EMAIL", 6))
    return 1;
  else if (strncmp(tag, "FONE", 5))
    return 1;
  else if (strncmp(tag, "ROMN", 5))
    return 1;
  else
    return 0;
}

int compat_check_551_tag(const char* tag, struct safe_buffer* b)
{
  if (is_551_tag(tag)) {
    reset_buffer(b);
    SAFE_BUF_ADDCHAR(b, '_');
    safe_buf_append(b, tag);
    gedcom_warning(_("Converting 5.5.1 tag '%s' to standard 5.5 user tag '%s'"),
		   tag, get_buf_string(b));
    return 1;
  }
  else
    return 0;
}

/********************************************************************/
/*  C_NO_SLGC_FAMC                                                  */
/********************************************************************/

void compat_generate_slgc_famc_link(Gedcom_ctxt parent)
{
  struct xref_value *xr = gedcom_parse_xref(SLGC_FAMC_LINK, XREF_USED,
					    XREF_FAM);
  struct tag_struct ts;
  Gedcom_ctxt self;
  
  ts.string = "FAMC";
  ts.value  = TAG_FAMC;
  gedcom_warning(_("Adding link to family record with xref '%s'"),
		 SLGC_FAMC_LINK);
  self = start_element(ELT_SUB_LIO_SLGC_FAMC,
		       parent, 2, ts, SLGC_FAMC_LINK,
		       GEDCOM_MAKE_XREF_PTR(val1, xr));
  end_element(ELT_SUB_LIO_SLGC_FAMC, parent, self, NULL);
  compat_state[C_NO_SLGC_FAMC].i++;
}

void compat_generate_slgc_famc_fam()
{
  /* If bigger than 1, then the FAM record has already been generated */
  if (compat_state[C_NO_SLGC_FAMC].i == 1) {
    struct xref_value *xr = gedcom_parse_xref(SLGC_FAMC_LINK, XREF_DEFINED,
					      XREF_FAM);
    struct tag_struct ts;
    Gedcom_ctxt self;
    
    /* generate "0 FAM" */
    ts.string = "FAM";
    ts.value  = TAG_FAM;
    self = start_record(REC_FAM, 0, GEDCOM_MAKE_XREF_PTR(val1, xr), ts,
			NULL, GEDCOM_MAKE_NULL(val2));
    
    /* close "0 FAM" */
    end_record(REC_FAM, self, NULL);
  }
}

/********************************************************************/
/*  C_SUBM_COMM                                                     */
/********************************************************************/

int compat_check_subm_comm(const char* tag, const char* parent_tag,
			   struct safe_buffer* b)
{
  if (!strcmp(tag, "COMM") && !strcmp(parent_tag, "SUBM")) {
    reset_buffer(b);
    SAFE_BUF_ADDCHAR(b, '_');
    safe_buf_append(b, tag);
    gedcom_warning(_("Converting non-standard tag '%s' to user tag '%s'"),
		   tag, get_buf_string(b));
    compat_state[C_SUBM_COMM].i = 1;
    return 1;
  }
  else
    return 0;
}

void compat_close_subm_comm()
{
  compat_state[C_SUBM_COMM].i = 0;
}

int compat_check_subm_comm_cont(const char* tag)
{
  if (compat_state[C_SUBM_COMM].i && !strcmp(tag, "CONT")) {
    compat_state[C_SUBM_COMM].i = 2;
    return 1;
  }
  else
    return 0;
}

Gedcom_ctxt compat_subm_comm_cont_start(Gedcom_ctxt parent, char* str)
{
  Gedcom_ctxt self = NULL;
  struct tag_struct ts;

  if (compat_state[C_SUBM_COMM].i == 2) {
    ts.string = "_CONT";
    ts.value  = USERTAG;
    self = start_element(ELT_USER, parent, 2, ts, str, &val2);
  }

  return self;
}

void compat_subm_comm_cont_end(Gedcom_ctxt parent, Gedcom_ctxt self)
{
  if (compat_state[C_SUBM_COMM].i == 2) {
    end_element(ELT_USER, parent, self, NULL);
    compat_state[C_SUBM_COMM].i = 1;
  }
}

/********************************************************************/
/*  C_NOTE_TOO_LONG                                                 */
/********************************************************************/

char compat_prefix[MAXGEDCLINELEN];

int compat_long_line(int level, int tag)
{
  return compat_mode(C_NOTE_TOO_LONG) && (level > 0) && (tag == TAG_NOTE);
}

char* compat_long_line_get_prefix(char* str)
{
  if (str && utf8_strlen(str) > MAXGEDCLINELEN - 7) {
    int len = MAXGEDCLINELEN - 7;
    char* ch     = nth_utf8_char(str, len - 1);
    char* nextch = next_utf8_char(ch);
    memset(compat_prefix, 0, MAXGEDCLINELEN);
    while (len > 1 && (*ch == ' ' || *nextch == ' ')) {
      len--;
      nextch = ch;
      ch     = nth_utf8_char(str, len - 1);
    }
    len = nextch - str;
    strncpy(compat_prefix, str, len);
    compat_state[C_NOTE_TOO_LONG].vp = (void*)nextch;
    return compat_prefix;
  }
  else {
    compat_state[C_NOTE_TOO_LONG].vp = NULL;
    return str;
  }
}

void compat_long_line_finish(Gedcom_ctxt parent, int level)
{
  struct tag_struct ts;
  ts.string = "CONC";
  ts.value  = TAG_CONC;
  
  while (compat_state[C_NOTE_TOO_LONG].vp) {
    Gedcom_ctxt ctxt;
    char* input  = (char*)compat_state[C_NOTE_TOO_LONG].vp;
    char* output = compat_long_line_get_prefix(input);

    ctxt = start_element(ELT_SUB_CONC, parent, level + 1, ts, output,
			 GEDCOM_MAKE_STRING(val1, output));
    end_element(ELT_SUB_CONC, parent, ctxt, GEDCOM_MAKE_NULL(val1));
  }
}

/********************************************************************/
/*  C_NOTE_CONC_SOUR                                                */
/********************************************************************/

Gedcom_ctxt compat_generate_note_sour_start(Gedcom_ctxt parent,
					    int level, struct tag_struct ts,
					    char* pointer)
{
  Gedcom_ctxt self;
  struct xref_value *xr = gedcom_parse_xref(pointer, XREF_USED, XREF_SOUR);
  if (xr == NULL) {
    self = (void*)-1;
  }
  else {
    self = start_element(ELT_SUB_SOUR, parent, level-1, ts, pointer,
			 GEDCOM_MAKE_XREF_PTR(val1, xr));
  }
  compat_state[C_NOTE_CONC_SOUR].vp = parent;
  return self;
}

void compat_generate_note_sour_end(Gedcom_ctxt self)
{
  if (self != (void*) -1) {
    end_element(ELT_SUB_SOUR, compat_state[C_NOTE_CONC_SOUR].vp,
		self, GEDCOM_MAKE_NULL(val1));
  }
}

/********************************************************************/
/*  C_NONSTD_SOUR_TAGS                                              */
/********************************************************************/

int is_nonstd_sour_tag(const char* tag)
{
  if (strncmp(tag, "FILN", 5))
    return 1;
  else if (strncmp(tag, "URL", 4))
    return 1;
  else if (strncmp(tag, "LOCA", 5))
    return 1;
  else if (strncmp(tag, "REGI", 5))
    return 1;
  else if (strncmp(tag, "VOL", 4))
    return 1;
  else
    return 0;
}

int compat_check_sour_tag(const char* tag, struct safe_buffer* b)
{
  if (is_nonstd_sour_tag(tag)) {
    reset_buffer(b);
    SAFE_BUF_ADDCHAR(b, '_');
    safe_buf_append(b, tag);
    gedcom_warning(_("Converting undefined tag '%s' to user tag '%s'"),
		   tag, get_buf_string(b));
    return 1;
  }
  else
    return 0;
}

Gedcom_ctxt compat_generate_nonstd_sour_start(Gedcom_ctxt parent, int level,
					      struct tag_struct ts,
					      char* value,
					      struct safe_buffer* b)
{
  Gedcom_ctxt self = NULL;
  reset_buffer(b);
  SAFE_BUF_ADDCHAR(b, '_');
  safe_buf_append(b, ts.string);
  gedcom_warning(_("Converting invalidly used tag '%s' to user tag '%s'"),
		 ts.string, get_buf_string(b));
  ts.string = get_buf_string(b);

  self = start_element(ELT_USER, parent, level, ts, value,
		       GEDCOM_MAKE_NULL_OR_STRING(val1, value));
  compat_state[C_NONSTD_SOUR_TAGS].i = 1;
  return self;
}

void compat_generate_nonstd_sour_end(Gedcom_ctxt parent, Gedcom_ctxt self)
{
  end_element(ELT_USER, parent, self, NULL);
  compat_state[C_NONSTD_SOUR_TAGS].i = 0;
}

int compat_generate_nonstd_sour_state()
{
  return compat_state[C_NONSTD_SOUR_TAGS].i;
}
