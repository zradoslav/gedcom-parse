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
#include "gom.h"
#include "output.h"
#include "dump_gom.h"
#include "portability.h"
#include "string.h"
#include <locale.h>
#include <stdio.h>

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

void show_help ()
{
  printf("gedcom-parse test program for libgedcom\n\n");
  printf("Usage:  updategomtest [options]\n");
  printf("Options:\n");
  printf("  -h    Show this help text\n");
  printf("  -q    No output to standard output\n");
  printf("  -o <outfile>  File to generate output to (def. testgedcom.out)\n");
}

int test_string_functions()
{
  struct header* head;
  struct submitter* subm;
  struct xref_value* xref;
  char* value;
  int conv_fails = 0;
  const char* orig_source_id = "GEDCOM_PARSE";
  const char* new_source_id = "TEST_UPDATE";
  const char* new_submitter_name_utf8 = "Belgi\xC3\xAB";
  const char* new_submitter_name_ansi = "Belgi\xEB";

  head = gom_get_header();
  if (head == NULL)
    return 10;
  
  value = gom_get_string(head->source.id);
  if (value == NULL)
    return 11;
  if (strcmp(value, orig_source_id))
    return 12;

  value = gom_set_string(&head->source.id, new_source_id);
  if (value == NULL)
    return 13;
  if (strcmp(value, new_source_id))
    return 14;

  value = gom_get_string(head->source.id);
  if (value == NULL)
    return 15;
  if (strcmp(value, new_source_id))
    return 16;

  xref = head->submitter;
  if (xref == NULL)
    return 17;
  
  subm = gom_get_submitter_by_xref(xref->string);
  if (subm == NULL)
    return 18;

  value = gom_set_string(&subm->name, new_submitter_name_utf8);
  if (value == NULL)
    return 19;
  if (strcmp(value, new_submitter_name_utf8))
    return 20;

  value = gom_get_string_for_locale(subm->name, &conv_fails);
  if (value == NULL)
    return 21;
  if (!strcmp(value, new_submitter_name_utf8))
    return 22;
  if (conv_fails != 1)
    return 23;

  value = gom_set_string(&subm->name, new_submitter_name_ansi);
  if (value != NULL)
    return 24;
  
  value = gom_set_string_for_locale(&subm->name, new_submitter_name_ansi);
  if (value != NULL)
    return 25;

  return 0;
}

char* print_date(const char* message, struct date_value* dv)
{
  char* date_str;
  output(0, "\n%s:", message);
  show_date(dv);
  date_str = gedcom_date_to_string(dv);
  output(0, "String: '%s'\n", str_val(date_str));
  return date_str;
}

int test_date_functions()
{
  struct header* head;
  struct date_value* dv;
  char* date_str;
  int normalized;
  
  head = gom_get_header();
  if (head == NULL)
    return 100;

  dv = head->date;
  if (dv != NULL)
    return 101;

  dv = gedcom_new_date_value(NULL);
  if (dv == NULL)
    return 102;

  head->date = dv;
  date_str = print_date("Initial date value", dv);
  if (date_str[0])
    return 103;

  dv->date1.cal = CAL_GREGORIAN;
  strcpy(dv->date1.year_str, "1990");
  normalized = gedcom_normalize_date(DI_FROM_STRINGS, dv);
  if (normalized != 0)
    return 104;
  date_str = print_date("Setting only year string", dv);
  if (! date_str[0])
    return 105;

  dv->date1.year = 1989;
  normalized = gedcom_normalize_date(DI_FROM_NUMBERS, dv);
  if (normalized != 0)
    return 106;
  date_str = print_date("Setting only year number", dv);
  if (! date_str[0])
    return 107;

  dv->date1.type = DATE_EXACT;
  dv->date1.sdn1 = 2500000;
  dv->date1.sdn2 = -1;
  normalized = gedcom_normalize_date(DI_FROM_SDN, dv);
  if (normalized != 0)
    return 108;
  date_str = print_date("Setting only SDN 1", dv);
  if (! date_str[0])
    return 109;

  dv->date1.cal = CAL_HEBREW;
  normalized = gedcom_normalize_date(DI_FROM_SDN, dv);  
  if (normalized != 0)
    return 110;
  date_str = print_date("Same date in Hebrew calendar", dv);
  if (! date_str[0])
    return 111;

  dv->date1.cal = CAL_FRENCH_REV;
  normalized = gedcom_normalize_date(DI_FROM_SDN, dv);  
  if (normalized == 0)
    return 112;
  date_str = print_date("Same date in French revolution calendar", dv);
  if (date_str[0])
    return 113;

  dv->date1.cal = CAL_GREGORIAN;
  dv->date1.day = 4;
  dv->date1.month = 2;
  dv->date1.year = 1799;
  normalized = gedcom_normalize_date(DI_FROM_NUMBERS, dv);
  if (normalized != 0)
    return 114;
  dv->date1.cal = CAL_FRENCH_REV;
  normalized = gedcom_normalize_date(DI_FROM_SDN, dv);
  if (normalized != 0)
    return 115;
  date_str = print_date("Valid French revolution date", dv);
  if (! date_str[0])
    return 116;

  return 0;
}

int test_record_add_delete_functions()
{
  struct family* fam1;
  struct individual* ind1;
  struct multimedia* mm1;
  struct note* note1;
  struct repository* repo1;
  struct source* sour1;
  struct submitter* subm2;
  struct submission* subn1;
  struct user_rec* user1;
  int result;
  char* value;
  const char* new_nr_of_children = "3";
  const char* note_text = "This is some text";

  fam1 = gom_add_family("@FAM1@");
  if (!fam1) return 201;
  
  value = gom_set_string(&fam1->nr_of_children, new_nr_of_children);
  if (value == NULL)
    return 202;
  if (strcmp(value, new_nr_of_children))
    return 203;

  ind1 = gom_add_individual("@FAM1@");
  if (ind1) return 204;

  ind1 = gom_add_individual("@IND1@");
  if (!ind1) return 205;

  mm1 = gom_add_multimedia("@OBJ1@");
  if (!mm1) return 206;

  note1 = gom_add_note("@NOTE1@");
  if (!note1) return 207;
  
  value = gom_set_string(&note1->text, note_text);
  if (value == NULL)
    return 208;
  if (strcmp(value, note_text))
    return 209;

  repo1 = gom_add_repository("@REPO1@");
  if (!repo1) return 210;

  sour1 = gom_add_source("@SOUR1@");
  if (!sour1) return 211;

  subm2 = gom_add_submitter("@SUBMITTER@");
  if (subm2) return 212;

  subm2 = gom_add_submitter("@SUBM2@");
  if (!subm2) return 213;

  subn1 = gom_add_submission("@SUBMISSION@");
  if (!subn1) return 214;

  user1 = gom_add_user_rec("@USER1@", "WRTAG");
  if (user1) return 215;

  user1 = gom_add_user_rec("@USER1@", "_TAG");
  if (!user1) return 216;

  output(0, "Intermediate output:\n");
  show_data();

  result = gom_delete_family(fam1);
  if (result != 0) return 217;

  result = gom_delete_individual(ind1);
  if (result != 0) return 218;

  result = gom_delete_multimedia(mm1);
  if (result != 0) return 219;

  result = gom_delete_note(note1);
  if (result != 0) return 220;

  result = gom_delete_repository(repo1);
  if (result != 0) return 221;

  result = gom_delete_source(sour1);
  if (result != 0) return 222;

  result = gom_delete_submitter(subm2);
  if (result != 0) return 223;

  result = gom_delete_submission(subn1);
  if (result != 0) return 224;

  result = gom_delete_user_rec(user1);
  if (result != 0) return 225;
  
  return 0;
}

int main(int argc, char* argv[])
{
  int result;
  char* outfilename = NULL;
  
  if (argc > 1) {
    int i;
    for (i=1; i<argc; i++) {
      if (!strncmp(argv[i], "-h", 3)) {
	show_help();
	exit(1);
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
      else {
	printf ("Unrecognized option: %s\n", argv[i]);
	show_help();
	exit(1);
      }
    }
  }
  
  gedcom_init();
  setlocale(LC_ALL, "");
  gedcom_set_message_handler(gedcom_message_handler);

  output_open(outfilename);
  
  result = gom_new_model();
  if (result == 0)
    result |= test_string_functions();
  if (result == 0)
    result |= test_date_functions();
  if (result == 0)
    result |= test_record_add_delete_functions();
  if (result == 0) {
    output(1, "Test succeeded\n");
  }
  else {
    output(1, "Test failed: %d\n", result);
  }

  show_data();
  output_close();
  return result;
}
