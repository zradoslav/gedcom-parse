/* Test program for the Gedcom library.
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

#include "gedcom.h"
#include "output.h"
#include "portability.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <errno.h>
#include <iconv.h>
#include "utf8tools.h"

#define BOGUS_FILE_NAME "bogus.ged"
int total_conv_fails = 0;

void show_help ()
{
  printf("gedcom-parse test program for libgedcom\n\n");
  printf("Usage:  testgedcom [options] file\n");
  printf("Options:\n");
  printf("  -h    Show this help text\n");
  printf("  -nc   Disable compatibility mode\n");
  printf("  -fi   Fail immediately on errors\n");
  printf("  -fd   Deferred fail on errors, but parse completely\n");
  printf("  -fn   No fail on errors\n");
  printf("  -dg   Debug setting: only libgedcom debug messages\n");
  printf("  -da   Debug setting: libgedcom + yacc debug messages\n");
  printf("  -2    Run the test parse 2 times instead of once\n");
  printf("  -3    Run the test parse 3 times instead of once\n");
  printf("  -b    Parse a bogus file (bogus.ged) before parsing the main file\n");
  printf("  -q    No output to standard output\n");
  printf("  -o <outfile>  File to generate output to (def. testgedcom.out)\n");
}

Gedcom_ctxt header_start(Gedcom_rec rec, int level, Gedcom_val xref, char *tag,
			 char *raw_value, int tag_value,
			 Gedcom_val parsed_value)
{
  output(1, "Header start\n");
  return (Gedcom_ctxt)1;
}

void header_end(Gedcom_rec rec, Gedcom_ctxt self, Gedcom_val parsed_value)
{
  output(1, "Header end, context is %ld\n", void_ptr_to_int(self));
}

#define MAXFAMILY 100
char family_xreftags[MAXFAMILY][255];
int  family_nr = 1;

Gedcom_ctxt family_start(Gedcom_rec rec, int level, Gedcom_val xref, char *tag,
			 char *raw_value, int tag_value,
			 Gedcom_val parsed_value)
{
  struct xref_value *xr = GEDCOM_XREF_PTR(xref);
  output(1, "Family start, xref is %s\n", xr->string);
  if (family_nr < MAXFAMILY) {
    strcpy(family_xreftags[family_nr], xr->string);
  }
  xr->object = (Gedcom_ctxt)int_to_void_ptr(family_nr);
  return (Gedcom_ctxt)int_to_void_ptr(family_nr++);
}

Gedcom_ctxt rec_start(Gedcom_rec rec, int level, Gedcom_val xref, char *tag,
		      char *raw_value, int tag_value,
		      Gedcom_val parsed_value)
{
  char* xref_str = NULL;
  if (! GEDCOM_IS_NULL(xref))
    xref_str = GEDCOM_XREF_PTR(xref)->string;
  output(1, "Rec %s start, xref is %s\n", tag, xref_str);
  return (Gedcom_ctxt)int_to_void_ptr(tag_value);
}

Gedcom_ctxt note_start(Gedcom_rec rec, int level, Gedcom_val xref, char *tag,
		       char *raw_value, int tag_value,
		       Gedcom_val parsed_value)
{
  output(1, "== %d %s (%d) %s (xref is %s)\n",
	 level, tag, tag_value, GEDCOM_STRING(parsed_value),
	 GEDCOM_XREF_PTR(xref)->string);
  return (Gedcom_ctxt)int_to_void_ptr(tag_value);
}

void family_end(Gedcom_rec rec, Gedcom_ctxt self, Gedcom_val parsed_value)
{
  char* family_xref = "<NOTSAVED>";
  int   family_nr = void_ptr_to_int(self);
  if (family_nr < MAXFAMILY)
    family_xref = family_xreftags[void_ptr_to_int(self)];
  output(1, "Family end, xref is %s\n", family_xref);
}

Gedcom_ctxt submit_start(Gedcom_rec rec, int level, Gedcom_val xref, char *tag,
			 char *raw_value, int tag_value,
			 Gedcom_val parsed_value)
{
  output(1, "Submitter, xref is %s\n", GEDCOM_XREF_PTR(xref)->string);
  return (Gedcom_ctxt)10000;
}

Gedcom_ctxt source_start(Gedcom_elt elt, Gedcom_ctxt parent, int level,
			 char *tag, char* raw_value,
			 int tag_value, Gedcom_val parsed_value)
{
  Gedcom_ctxt self = (Gedcom_ctxt)(void_ptr_to_int(parent) + 1000);
  output(1, "Source is %s (ctxt is %ld, parent is %ld)\n",
	 GEDCOM_STRING(parsed_value), void_ptr_to_int(self),
	 void_ptr_to_int(parent));
  return self;
}

void source_end(Gedcom_elt elt, Gedcom_ctxt parent, Gedcom_ctxt self,
		Gedcom_val parsed_value)
{
  output(1, "Source context %ld in parent %ld\n",
	 void_ptr_to_int(self), void_ptr_to_int(parent));
}

Gedcom_ctxt head_note_start(Gedcom_elt elt, Gedcom_ctxt parent, int level,
			    char *tag, char* raw_value,
			    int tag_value, Gedcom_val parsed_value)
{
  Gedcom_ctxt self = (Gedcom_ctxt)(void_ptr_to_int(parent));
  output(1, "Note: %s (ctxt is %ld, parent is %ld)\n",
	 GEDCOM_STRING(parsed_value), void_ptr_to_int(self),
	 void_ptr_to_int(parent));
  return self;
}

void head_note_end(Gedcom_elt elt, Gedcom_ctxt parent, Gedcom_ctxt self,
		   Gedcom_val parsed_value)
{
  output(1, "Complete note:\n%s\n", GEDCOM_STRING(parsed_value));
}

Gedcom_ctxt date_start(Gedcom_elt elt, Gedcom_ctxt parent, int level,
		       char *tag, char* raw_value,
		       int tag_value, Gedcom_val parsed_value)
{
  struct date_value dv;
  Gedcom_ctxt self = (Gedcom_ctxt)(void_ptr_to_int(parent) + 1000);
  dv = GEDCOM_DATE(parsed_value);
  output(1, "Contents of the date_value:\n");
  output(1, "  raw value: %s\n", raw_value);
  output(1, "  type: %d\n", dv.type);
  output(1, "  date1:\n");
  output(1, "    calendar type: %d\n", dv.date1.cal);
  output(1, "    day: %s\n", dv.date1.day_str);
  output(1, "    month: %s\n", dv.date1.month_str);
  output(1, "    year: %s\n", dv.date1.year_str);
  output(1, "    date type: %d\n", dv.date1.type);
  output(1, "    sdn1: %ld\n", dv.date1.sdn1);
  output(1, "    sdn2: %ld\n", dv.date1.sdn2);
  output(1, "  date2:\n");
  output(1, "    calendar type: %d\n", dv.date2.cal);
  output(1, "    day: %s\n", dv.date2.day_str);
  output(1, "    month: %s\n", dv.date2.month_str);
  output(1, "    year: %s\n", dv.date2.year_str);
  output(1, "    date type: %d\n", dv.date2.type);
  output(1, "    sdn1: %ld\n", dv.date2.sdn1);
  output(1, "    sdn2: %ld\n", dv.date2.sdn2);
  output(1, "  phrase: %s\n", dv.phrase);
  return self;
}

Gedcom_ctxt age_start(Gedcom_elt elt, Gedcom_ctxt parent, int level,
		      char *tag, char *raw_value,
		      int tag_value, Gedcom_val parsed_value)
{
  struct age_value age;
  Gedcom_ctxt self = (Gedcom_ctxt)(void_ptr_to_int(parent) + 1000);
  age = GEDCOM_AGE(parsed_value);
  output(1, "Contents of the age_value:\n");
  output(1, "  raw value: %s\n", raw_value);
  output(1, "  type: %d\n", age.type);
  output(1, "  modifier: %d\n", age.mod);
  output(1, "  years: %d\n", age.years);
  output(1, "  months: %d\n", age.months);
  output(1, "  days: %d\n", age.days);
  output(1, "  phrase: %s\n", age.phrase);
  return self;
}

void default_cb(Gedcom_elt elt, Gedcom_ctxt ctxt, int level, char *tag,
		char *raw_value, int tag_value)
{
  char   *converted = NULL;
  int    conv_fails = 0;
  if (raw_value)
    converted = convert_utf8_to_locale(raw_value, &conv_fails);
  output(0, "== %d %s (%d) %s (ctxt is %ld, conversion failures: %d)\n",
	 level, tag, tag_value, converted, void_ptr_to_int(ctxt), conv_fails);
  total_conv_fails += conv_fails;
}

void subscribe_callbacks()
{
  gedcom_subscribe_to_record(REC_HEAD, header_start, header_end);
  gedcom_subscribe_to_record(REC_FAM,  family_start, family_end);
  gedcom_subscribe_to_record(REC_INDI, rec_start, NULL);
  gedcom_subscribe_to_record(REC_OBJE, rec_start, NULL);
  gedcom_subscribe_to_record(REC_NOTE, note_start, NULL);
  gedcom_subscribe_to_record(REC_REPO, rec_start, NULL);
  gedcom_subscribe_to_record(REC_SOUR, rec_start, NULL);
  gedcom_subscribe_to_record(REC_SUBN, rec_start, NULL);
  gedcom_subscribe_to_record(REC_SUBM, submit_start, NULL);
  gedcom_subscribe_to_record(REC_USER, rec_start, NULL);
  gedcom_subscribe_to_element(ELT_HEAD_SOUR, source_start, source_end);
  gedcom_subscribe_to_element(ELT_HEAD_NOTE, head_note_start, head_note_end);
  gedcom_subscribe_to_element(ELT_SOUR_DATA_EVEN_DATE,
			      date_start, NULL);
  gedcom_subscribe_to_element(ELT_SUB_EVT_DATE, date_start, NULL);
  gedcom_subscribe_to_element(ELT_SUB_FAM_EVT_AGE, age_start, NULL);
}

void gedcom_message_handler(Gedcom_msg_type type, char *msg)
{
  if (type == MESSAGE)
    output(1, "MESSAGE: ");
  else if (type == WARNING)
    output(1, "WARNING: ");
  else if (type == ERROR)
    output(1, "ERROR: ");
  output(1, "%s\n", msg);
}

int main(int argc, char* argv[])
{
  Gedcom_err_mech mech = IMMED_FAIL;
  int compat_enabled = 1;
  int debug_level = 0;
  int run_times   = 1;
  int bogus       = 0;
  int result      = 0;
  char* outfilename = NULL;
  char* file_name = NULL;

  if (argc > 1) {
    int i;
    for (i=1; i<argc; i++) {
      if (!strncmp(argv[i], "-da", 4))
	debug_level = 2;
      else if (!strncmp(argv[i], "-dg", 4))
	debug_level = 1;
      else if (!strncmp(argv[i], "-fi", 4))
	mech = IMMED_FAIL;
      else if (!strncmp(argv[i], "-fd", 4))
	mech = DEFER_FAIL;
      else if (!strncmp(argv[i], "-fn", 4))
	mech = IGNORE_ERRORS;
      else if (!strncmp(argv[i], "-nc", 4))
	compat_enabled = 0;
      else if (!strncmp(argv[i], "-h", 3)) {
	show_help();
	exit(1);
      }
      else if (!strncmp(argv[i], "-2", 3)) {
	run_times = 2;
      }
      else if (!strncmp(argv[i], "-3", 3)) {
	run_times = 3;
      }
      else if (!strncmp(argv[i], "-b", 3)) {
	bogus = 1;
      }
      else if (!strncmp(argv[i], "-q", 3)) {
	output_set_quiet(1);
      }
      else if (!strncmp(argv[i], "-o", 3)) {
	i++;
	if (i < argc) {
	  outfilename = argv[i];
	}
	else {
	  printf ("Missing output file name\n");
	  show_help();
	  exit(1);
	}
      }
      else if (strncmp(argv[i], "-", 1)) {
	file_name = argv[i];
	break;
      }
      else {
	printf ("Unrecognized option: %s\n", argv[i]);
	show_help();
	exit(1);
      }
    }
  }
  
  if (!file_name) {
    printf("No file name given\n");
    show_help();
    exit(1);
  }

  gedcom_init();
  setlocale(LC_ALL, "");
  gedcom_set_debug_level(debug_level, NULL);
  gedcom_set_compat_handling(compat_enabled);
  gedcom_set_compat_options(COMPAT_ALLOW_OUT_OF_CONTEXT);
  gedcom_set_error_handling(mech);
  gedcom_set_message_handler(gedcom_message_handler);
  gedcom_set_default_callback(default_cb);
  
  subscribe_callbacks();
  output_open(outfilename);
  if (bogus) {
    output(0, "\n=== Parsing bogus file %s\n", BOGUS_FILE_NAME);
    gedcom_parse_file(BOGUS_FILE_NAME);
  }
  while (run_times-- > 0) {
    output(0, "\n=== Parsing file %s\n", simple_base_name(file_name));
    result |= gedcom_parse_file(file_name);
    output(0, "\n=== Total conversion failures: %d\n", total_conv_fails);
  }
  if (result == 0) {
    output(1, "Parse succeeded\n");
  }
  else {
    output(1, "Parse failed\n");
  }
  output_close();
  return result;
}
