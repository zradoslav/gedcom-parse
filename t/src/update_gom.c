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