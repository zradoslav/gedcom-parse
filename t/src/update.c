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
  printf("Usage:  updatetest [options] file\n");
  printf("Options:\n");
  printf("  -h    Show this help text\n");
  printf("  -q    No output to standard output\n");
}

int test_xref_functions()
{
  struct xref_value* xr;
  int result;

  xr = gedcom_get_by_xref("@NOTHING_THERE@");
  if (xr != NULL)
    return 10;

  xr = gedcom_add_xref(XREF_FAM, "NOT_AN_XREF", (Gedcom_ctxt)1);
  if (xr != NULL)
    return 11;
  
  xr = gedcom_add_xref(XREF_FAM, "@XREF@ BUT EXTRA", (Gedcom_ctxt)1);
  if (xr != NULL)
    return 12;
  
  xr = gedcom_add_xref(XREF_FAM, "@NEWFAM@", (Gedcom_ctxt)1);
  if (xr == NULL)
    return 13;

  xr = gedcom_add_xref(XREF_FAM, "@NEWFAM@", (Gedcom_ctxt)2);
  if (xr != NULL)
    return 14;

  xr = gedcom_get_by_xref("@NEWFAM@");
  if (xr == NULL)
    return 15;

  if (xr->type != XREF_FAM) {
    output(1, "Not the correct cross-reference type\n");
    return 16;
  }

  if ((int)xr->object != 1) {
    output(1, "Not the correct cross-reference object\n");
    return 17;
  }
  
  xr = gedcom_link_xref(XREF_INDI, "@NEWFAM@");
  if (xr != NULL)
    return 18;

  xr = gedcom_link_xref(XREF_FAM, "@NEWFAM@");
  if (xr == NULL)
    return 19;

  xr = gedcom_link_xref(XREF_FAM, "@NEWFAM");
  if (xr != NULL)
    return 20;

  xr = gedcom_link_xref(XREF_FAM, "@NEWFAM@");
  if (xr == NULL)
    return 21;

  xr = gedcom_link_xref(XREF_INDI, "@OLDINDI@");
  if (xr != NULL)
    return 22;

  result = gedcom_delete_xref("@NEWFAM@");
  if (result == 0)
    return 23;

  xr = gedcom_unlink_xref(XREF_INDI, "@NEWFAM@");
  if (xr != NULL)
    return 24;

  xr = gedcom_unlink_xref(XREF_FAM, "@NEWFAM@");
  if (xr == NULL)
    return 25;

  result = gedcom_delete_xref("@NEWFAM@");
  if (result == 0)
    return 26;

  xr = gedcom_unlink_xref(XREF_FAM, "@NEWFAM@");
  if (xr == NULL)
    return 27;

  result = gedcom_delete_xref("@NEWFAM@");
  if (result != 0)
    return 28;

  return 0;
}

int main(int argc, char* argv[])
{
  int result;
  
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

  output_open();
  
  result = gedcom_new_model();
  result |= test_xref_functions();
  if (result == 0) {
    output(1, "Test succeeded\n");
  }
  else {
    output(1, "Test failed: %d\n", result);
  }

  output_close();
  return result;
}
