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
#include "portability.h"
#include <locale.h>
#include <stdio.h>

#define WRITE_GEDCOM "gom_write.ged"
#define PROG_NAME "writegomtest"
#define PROG_VERSION "3.14"

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
  printf("Usage:  writegomtest [options]\n");
  printf("Options:\n");
  printf("  -h    Show this help text\n");
  printf("  -q    No output to standard output\n");
  printf("  -o <outfile>  File to generate errors to (def. testgedcom.out)\n");
  printf("  -w <gedfile>  File to write gedcom to (def. %s)\n", WRITE_GEDCOM);
  printf("  -e <encoding> Encoding (UNICODE, ASCII, ANSEL, ...: see gedcom.enc)\n");
  printf("  -u <unicode_enc> Encoding details for Unicode\n");
  printf("        <unicode_enc> can be: HILO, LOHI, HILO_BOM, LOHI_BOM\n");
  printf("  -t <terminator>  Line terminator\n");
  printf("        <terminator> can be CR, LF, CR_LF, LF_CR\n");
}

int update_charset(char* encoding)
{
  struct header* head = NULL;
  char* value;

  head = gom_get_header();
  if (head == NULL)
    return 1;
  else {
    value = gom_set_string(&head->charset.name, encoding);
    if (value == NULL || strcmp(value, encoding))
      return 1;
    else
      return 0;
  }
}

int main(int argc, char* argv[])
{
  int result;
  int total_conv_fails = 0;
  char* outfilename = NULL;
  char* gedfilename = WRITE_GEDCOM;
  char* encoding    = "ASCII";
  Encoding enc      = ONE_BYTE;
  Enc_bom bom       = WITHOUT_BOM;
  Enc_line_end end  = END_LF;
  
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
      else if (!strncmp(argv[i], "-w", 3)) {
	i++;
	if (i < argc) {
	  gedfilename = argv[i];
	}
	else {
	  printf ("Missing output file name\n");
	  show_help();
	  exit(1);
	}
      }
      else if (!strncmp(argv[i], "-e", 3)) {
	i++;
	if (i < argc) {
	  encoding = argv[i];
	}
	else {
	  printf ("Missing encoding\n");
	  show_help();
	  exit(1);
	}
      }
      else if (!strncmp(argv[i], "-u", 3)) {
	i++;
	if (i < argc) {
	  char* details = argv[i];
	  if (!strncmp(details, "HILO", 4))
	    enc = TWO_BYTE_HILO;
	  else if (!strncmp(details, "LOHI", 4))
	    enc = TWO_BYTE_LOHI;
	  else {
	    printf("Unknown encoding details %s\n", details);
	    show_help();
	    exit(1);
	  }
	  if (!strncmp(details+5, "BOM", 4))
	    bom = WITH_BOM;
	}
	else {
	  printf ("Missing encoding details\n");
	  show_help();
	  exit(1);
	}
      }
      else if (!strncmp(argv[i], "-t", 3)) {
	i++;
	if (i < argc) {
	  char* term = argv[i];
	  if (!strncmp(term, "CR", 3))
	    end = END_CR;
	  else if (!strncmp(term, "LF", 3))
	    end = END_LF;
	  else if (!strncmp(term, "CR_LF", 6))
	    end = END_CR_LF;
	  else if (!strncmp(term, "LF_CR", 6))
	    end = END_LF_CR;
	  else {
	    printf("Unknown terminator: %s\n", term);
	    show_help();
	    exit(1);
	  }
	}
	else {
	  printf ("Missing terminator\n");
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
  gedcom_write_set_encoding(encoding, enc, bom);
  gedcom_write_set_line_terminator(end);

  output_open(outfilename);
  
  result = gom_new_model();
  /*
  if (result == 0)
    result |= update_charset(encoding);
  */
  if (result == 0)
    result |= gom_write_file(gedfilename, &total_conv_fails);
  if (result == 0 && total_conv_fails == 0) {
    output(1, "Test succeeded\n");
  }
  else {
    output(1, "Test failed: %d\n", result);
  }

  output_close();
  return result;
}
