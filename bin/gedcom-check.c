/* Check program using the Gedcom library.
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
#include "utf8tools.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <libintl.h>

#define _(string) dgettext(PACKAGE, string)
#define N_(string) (string)

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

void show_help ()
{
  printf("Checks a GEDCOM file on standards compliancy\n\n");
  printf("Usage:  gedcom-check [options] file\n");
  printf("Options:\n");
  printf("  -h    Show this help text\n");
  printf("  -c    Enable compatibility mode\n");
  printf("  -dg   Debug setting: only libgedcom debug messages\n");
  printf("  -da   Debug setting: libgedcom + yacc debug messages\n");
  printf("Errors, warnings, ... are sent to stdout\n");
}

void default_cb(Gedcom_elt elt UNUSED, Gedcom_ctxt ctxt UNUSED,
		int level UNUSED, char *tag UNUSED,
		char *raw_value UNUSED, int tag_value UNUSED)
{
  /* do nothing */
}

void gedcom_message_handler(Gedcom_msg_type type UNUSED, char *msg)
{
  char *converted = NULL;
  int  conv_fails = 0;
  converted = convert_utf8_to_locale(msg, &conv_fails);
  printf("%s\n", converted);
}

int main(int argc, char* argv[])
{
  Gedcom_err_mech mech = DEFER_FAIL;
  int compat_enabled   = 0;
  int debug_level = 0;
  char* file_name = NULL;
  int result;
  
  if (argc > 1) {
    int i;
    for (i=1; i<argc; i++) {
      if (!strncmp(argv[i], "-da", 4))
	debug_level = 2;
      else if (!strncmp(argv[i], "-dg", 4))
	debug_level = 1;
      else if (!strncmp(argv[i], "-c", 3))
	compat_enabled = 1;
      else if (!strncmp(argv[i], "-h", 3)) {
	show_help();
	exit(1);
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
  gedcom_set_error_handling(mech);
  gedcom_set_message_handler(gedcom_message_handler);
  gedcom_set_default_callback(default_cb);

  result = gedcom_parse_file(file_name);
  
  if (result == 0) {
    printf(_("Parse succeeded\n"));
  }
  else {
    printf(_("Parse failed\n"));
  }
  return result;
}
