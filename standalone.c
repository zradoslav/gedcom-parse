/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "external.h"

void show_help ()
{
  printf("gedcom-parse test program for libgedcom\n\n");
  printf("Usage:  gedcom-parse [options] file\n");
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
}

Gedcom_ctxt header_start(char *xreftag __attribute__ ((unused)))
{
  printf("Header start\n");
  return (Gedcom_ctxt)0;
}

void header_end(Gedcom_ctxt self)
{
  printf("Header end, context is %d\n", (int)self);
}

char family_xreftags[100][255];
int  family_nr = 0;

Gedcom_ctxt family_start(char *xreftag)
{
  printf("Family start, xreftag is %s\n", xreftag);
  strcpy(family_xreftags[family_nr], xreftag);
  return (Gedcom_ctxt)(family_nr++);
}

void family_end(Gedcom_ctxt self)
{
  printf("Family end, xreftag is %s\n", family_xreftags[(int)self]);
}

void subscribe_callbacks()
{
  subscribe_to_record(REC_HEAD, header_start, header_end);
  subscribe_to_record(REC_FAM,  family_start, family_end);
}

void gedcom_message_handler(Gedcom_msg_type type, char *msg)
{
  if (type == MESSAGE)
    fprintf(stderr, "MESSAGE: ");
  else if (type == WARNING)
    fprintf(stderr, "WARNING: ");
  else if (type == ERROR)
    fprintf(stderr, "ERROR: ");
  fprintf(stderr, msg);
}

int main(int argc, char* argv[])
{
  Gedcom_err_mech mech = IMMED_FAIL;
  int compat_enabled = 1;
  int debug_level = 0;
  int run_times   = 1;
  int result      = 0;
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

  gedcom_set_debug_level(debug_level, NULL);
  gedcom_set_compat_handling(compat_enabled);
  gedcom_set_error_handling(mech);
  gedcom_set_message_handler(gedcom_message_handler);
  
  subscribe_callbacks();
  while (run_times-- > 0) {
    result |= gedcom_parse_file(file_name);
  }
  if (result == 0) {
    printf("Parse succeeded\n");
    return 0;
  }
  else {
    printf("Parse failed\n");
    return 1;
  }  
}
