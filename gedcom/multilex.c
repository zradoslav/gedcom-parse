/* The lexer multiplexer for Gedcom.
   Copyright (C) 2001,2002 The Genes Development Team
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

#include "gedcom_internal.h"
#include "multilex.h"
#include "encoding.h"
#include "encoding_state.h"
#include "xref.h"

int line_no = 0;

typedef int (*lex_func)(void);
lex_func lf;

#define NEW_MODEL_FILE "new.ged"

int lexer_init(Encoding enc, FILE* f)
{
  if (enc == ONE_BYTE) {
    lf  = &gedcom_1byte_lex;
    gedcom_1byte_myinit(f);
    set_read_encoding_width(enc);
    return open_conv_to_internal("ASCII");
  }
  else if (enc == TWO_BYTE_HILO) {
    lf  = &gedcom_hilo_lex;
    gedcom_hilo_myinit(f);
    set_read_encoding_width(enc);
    return open_conv_to_internal("UNICODE");
  }
  else if (enc == TWO_BYTE_LOHI) {
    lf  = &gedcom_lohi_lex;
    gedcom_lohi_myinit(f);
    set_read_encoding_width(enc);
    return open_conv_to_internal("UNICODE");
  }
  else {
    return 0;
  }
}

void lexer_close()
{
  close_conv_to_internal();
}

int gedcom_lex()
{
  return (*lf)();
}

void rewind_file(FILE* f)
{
  if (fseek(f, 0, 0) != 0)
    gedcom_warning(_("Error positioning input file: %s"), strerror(errno));
}

int determine_encoding(FILE* f)
{
  char first[2];
  int read;

  set_read_encoding_bom(WITHOUT_BOM);
  read = fread(first, 1, 2, f);
  if (read != 2) {
    gedcom_warning(_("Error reading from input file: %s"), strerror(errno));
    rewind_file(f);
    return ONE_BYTE;
  }
  else if ((first[0] == '0') && (first[1] == ' ')) {
    gedcom_debug_print("One-byte encoding");
    rewind_file(f);
    return ONE_BYTE;
  }
  else if ((first[0] == '\0') && (first[1] == '0')) {
    gedcom_debug_print("Two-byte encoding, high-low");
    rewind_file(f);
    return TWO_BYTE_HILO;
  }
  else if ((first[0] == '\xFE') && (first[1] == '\xFF')) {
    gedcom_debug_print("Two-byte encoding, high-low, with BOM");
    set_read_encoding_bom(WITH_BOM);
    return TWO_BYTE_HILO;
  }
  else if ((first[0] == '0') && (first[1] == '\0')) {
    gedcom_debug_print("Two-byte encoding, low-high");
    rewind_file(f);
    return TWO_BYTE_LOHI;
  }
  else if ((first[0] == '\xFF') && (first[1] == '\xFE')) {
    gedcom_debug_print("Two-byte encoding, low-high, with BOM");
    set_read_encoding_bom(WITH_BOM);
    return TWO_BYTE_LOHI;
  }
  else if ((first[0] == '\xEF') && (first[1] == '\xBB')) {
    read = fread(first, 1, 1, f);
    if (read != 1) {
      gedcom_warning(_("Error reading from input file: %s"), strerror(errno));
      rewind_file(f);
    }
    else if (first[0] == '\xBF') {
      set_read_encoding_bom(WITH_BOM);
      gedcom_debug_print("UTF-8 encoding, with BOM");
    }
    else {
      gedcom_warning(_("Unknown encoding, falling back to one-byte"));
      rewind_file(f);
    }
    return ONE_BYTE;
  }
  else {
    gedcom_warning(_("Unknown encoding, falling back to one-byte"));
    rewind_file(f);
    return ONE_BYTE;
  }
}

int init_called = 0;

int gedcom_init()
{
  init_called = 1;
  update_gconv_search_path();
  init_encodings();
  if (!setlocale(LC_ALL, "")
      || ! bindtextdomain(PACKAGE, LOCALEDIR)
      || ! bind_textdomain_codeset(PACKAGE, INTERNAL_ENCODING))
    return 1;
  else
    return 0;
}

int gedcom_parse_file(const char* file_name)
{
  Encoding enc;
  int result = 1;
  FILE* file;

  if (!init_called) {
    gedcom_error(_("Internal error: GEDCOM parser not initialized"));
  }
  else {
    file = fopen(file_name, "r");
    if (!file) {
      gedcom_error(_("Could not open file '%s': %s"),
		   file_name, strerror(errno));
    }
    else {
      line_no = 1;
      enc = determine_encoding(file);
      
      if (lexer_init(enc, file)) {
	line_no = 0;
	make_xref_table();
	result = gedcom_parse();
	line_no = 0;
	if (result == 0)
	  result = check_xref_table();
      }
      lexer_close();
      fclose(file);
    }
  }

  return result;
}

int gedcom_new_model()
{
  int result = 1;
  FILE* file;

  file = fopen(NEW_MODEL_FILE, "r");
  if (file) {
    fclose(file);
    result = gedcom_parse_file(NEW_MODEL_FILE);
  }
  else {
    char* filename = (char*) malloc(strlen(PKGDATADIR) + strlen(NEW_MODEL_FILE)
				    + 2);
    if (!filename)
      MEMORY_ERROR;
    else {
      sprintf(filename, "%s/%s", PKGDATADIR, NEW_MODEL_FILE);
      result = gedcom_parse_file(filename);
      free(filename);
    }
  }
  return result;
}

int gedcom_check_version(int major, int minor, int patch)
{
  if (major < GEDCOM_PARSE_VERSION_MAJOR)
    return 1;
  else if (major > GEDCOM_PARSE_VERSION_MAJOR)
    return 0;
  else if (minor < GEDCOM_PARSE_VERSION_MINOR)
    return 1;
  else if (minor > GEDCOM_PARSE_VERSION_MINOR)
    return 0;
  else if (patch <= GEDCOM_PARSE_VERSION_PATCH)
    return 1;
  else
    return 0;
}
