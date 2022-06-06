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

#include "config.h"

#include <locale.h>

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

/** This function initializes the Gedcom parser library and must be called
    before any other function in this library.

    The function also initializes locale handling by calling
    <tt> setlocale(LC_ALL, "") </tt>, in case the application would not do this
    (it doesn't hurt for the application to do the same).

    \attention This function should be called as early as possible.  The
    requirement
    is that it should come before the first call to \c iconv_open (part of the
    generic character set conversion feature) in the program, either by your
    program itself, or indirectly by the library calls it makes.
    \attention Practically,
    it should e.g. come before any calls to any GTK functions, because GTK
    uses \c iconv_open in its initialization.

    \retval 0 in case of success
    \retval nonzero in case of failure (e.g. failure to set locale)
 */
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

/** This function parses the given file.  By itself, it doesn't provide any
    other information than the parse result.

    The function also empties the cross-reference table before parsing, and
    checks the validity of the
    cross-references if the parse was successful.
    The following conditions can occur in the cross-reference table:
      - An xref was defined, but not used (warning)
      - An xref was used, but not defined (error)
      - An xref was used as a different type than the defined type (error)

    \param file_name The name of the Gedcom file to parse

    \retval 0 if the parse was successful and no errors were found in the
    cross-reference table
    \retval nonzero on errors, which can include:
            - \ref gedcom_init() was not called
	    - The given file was not found
	    - The parse of the given file failed
	    - There were errors found in the cross-reference table
 */

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

/** This function starts a new model.  It does this by parsing the \c new.ged
    file in the data directory of the library (\c $PREFIX/share/gedcom-parse).
    This can be used to start from an empty model, and to build up the model
    by adding new records yourself.

    \retval 0 on success
    \retval nonzero on errors (mainly the errors from
            \ref gedcom_parse_file()).
 */

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
  if (major < PACKAGE_VERSION_MAJOR)
    return 1;
  else if (major > PACKAGE_VERSION_MAJOR)
    return 0;
  else if (minor < PACKAGE_VERSION_MINOR)
    return 1;
  else if (minor > PACKAGE_VERSION_MINOR)
    return 0;
  else if (patch <= PACKAGE_VERSION_PATCH)
    return 1;
  else
    return 0;
}
