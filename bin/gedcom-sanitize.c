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

#include "config.h"
#include "gedcom.h"
#include "gom.h"
#include "utf8tools.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#ifdef ENABLE_NLS
#include <libintl.h>

#define _(string) dgettext(PACKAGE, string)
#define N_(string) (string)
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

void show_help ()
{
  printf("Converts a GEDCOM file to strict standard GEDCOM\n\n");
  printf("Usage:  gedcom-sanitize [options] file\n");
  printf("Options:\n");
  printf("  -h    Show this help text\n");
  printf("  -dg   Debug setting: only libgedcom debug messages\n");
  printf("  -da   Debug setting: libgedcom + yacc debug messages\n");
  printf("  -e <extension>   Extension to give to file name (default 'new')\n");
  printf("  -s    Keep source string (otherwise: changed to GEDCOM_PARSE)\n");
  printf("Errors, warnings, ... are sent to stdout\n");
}

void gedcom_message_handler(Gedcom_msg_type type UNUSED, char *msg)
{
  char *converted = NULL;
  int  conv_fails = 0;
  converted = convert_utf8_to_locale(msg, &conv_fails);
  printf("%s\n", converted);
}

int update_header()
{
  struct header* head = NULL;
  head = gom_get_header();
  if (head == NULL)
    return 1;
  else {
    char* value;
    int result, i;
    
    value = gom_set_string(&head->source.id, PACKAGE);
    if (value == NULL || strcmp (value, PACKAGE))
      return 1;

    value = gom_set_string(&head->source.name, NULL);
    if (value != NULL)
      return 1;

    value = gom_set_string(&head->source.version, PACKAGE_VERSION);
    if (value == NULL || strcmp (value, PACKAGE_VERSION))
      return 1;

    value = gom_set_string(&head->source.corporation.name, NULL);
    if (value != NULL)
      return 1;

    if (head->source.corporation.address) {
      result = gom_delete_address(&head->source.corporation.address);
      if (result != 0)
	return 1;
    }

    for (i=0; i<3; i++) {
      if (head->source.corporation.phone[i]) {
	value = gom_set_string(&head->source.corporation.phone[i], NULL);
	if (value != NULL)
	  return 1;
      }
    }
    
    return 0;
  }
}

int main(int argc, char* argv[])
{
  Gedcom_err_mech mech = DEFER_FAIL;
  int compat_enabled   = 1;
  int debug_level = 0;
  int keep_source = 0;
  char* file_name = NULL;
  int result;
  char* extension = "new";
  
  if (argc > 1) {
    int i;
    for (i=1; i<argc; i++) {
      if (!strncmp(argv[i], "-da", 4))
	debug_level = 2;
      else if (!strncmp(argv[i], "-dg", 4))
	debug_level = 1;
      else if (!strncmp(argv[i], "-s", 3))
	keep_source = 1;
      else if (!strncmp(argv[i], "-e", 3)) {
	if (i<argc) {
	  extension = argv[++i];
	}
	else {
	  show_help();
	  exit(1);
	}
      }
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

  result = gom_parse_file(file_name);
  
  if (result == 0) {
    char* newfile = (char*)malloc(strlen(file_name) + strlen(extension) + 2);
    sprintf(newfile, "%s.%s", file_name, extension);
    printf(_("Parse succeeded, now writing file '%s'\n"), newfile);
    if (! keep_source) {
      result = update_header();
    }
    if (result == 0)
      result = gom_write_file(newfile, NULL);
    free(newfile);
    if (result == 0) {
      printf(_("Write succeeded\n"));
    }
    else {
      printf(_("Write failed\n"));
    }
  }
  else {
    printf(_("Parse failed\n"));
  }
  return result;
}
