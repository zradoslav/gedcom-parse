/* The lexer multiplexer for Gedcom.
   Copyright (C) 2001 The Genes Development Team
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

int line_no;

typedef int (*lex_func)(void);
lex_func lf;

int lexer_init(ENCODING enc, FILE* f)
{
  if (enc == ONE_BYTE) {
    gedcom_1byte_in = f;
    lf = &gedcom_1byte_lex;
    set_encoding_width(enc);
    return open_conv_to_internal("ASCII");
  }
  else if (enc == TWO_BYTE_HILO) {
    gedcom_hilo_in = f;
    lf = &gedcom_hilo_lex;
    set_encoding_width(enc);
    return open_conv_to_internal("UNICODE");
  }
  else if (enc == TWO_BYTE_LOHI) {
    gedcom_lohi_in = f;
    lf = &gedcom_lohi_lex;
    set_encoding_width(enc);
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

int determine_encoding(FILE* f)
{
  char first[2];

  fread(first, 1, 2, f);
  if ((first[0] == '0') && (first[1] == ' ')) {
    gedcom_message("One-byte encoding");
    fseek(f, 0, 0);
    return ONE_BYTE;
  }
  else if ((first[0] == '\0') && (first[1] == '0'))
  {
    gedcom_message("Two-byte encoding, high-low");
    fseek(f, 0, 0);
    return TWO_BYTE_HILO;
  }
  else if ((first[0] == '\xFE') && (first[1] == '\xFF'))
  {
    gedcom_message("Two-byte encoding, high-low, with BOM");
    return TWO_BYTE_HILO;
  }
  else if ((first[0] == '0') && (first[1] == '\0'))
  {
    gedcom_message("Two-byte encoding, low-high");
    fseek(f, 0, 0);
    return TWO_BYTE_LOHI;
  }
  else if ((first[0] == '\xFF') && (first[1] == '\xFE'))
  {
    gedcom_message("Two-byte encoding, low-high, with BOM");
    return TWO_BYTE_LOHI;
  }
  else {
    gedcom_message("Unknown encoding, falling back to one-byte");
    fseek(f, 0, 0);
    return ONE_BYTE;
  }
}

int gedcom_parse_file(char* file_name)
{
  ENCODING enc;
  int result = 1;
  FILE* file = fopen (file_name, "r");
  line_no = 1;
  if (!file) {
    gedcom_error("Could not open file '%s'\n", file_name);
    return 1;
  }

  init_encodings();
  enc = determine_encoding(file);
  
  if (lexer_init(enc, file)) {
    result = gedcom_parse();
  }
  lexer_close();
  fclose(file);
  
  return result;
}

