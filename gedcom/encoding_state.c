/* Encoding state.
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

#include "gedcom_internal.h"
#include "gedcom.h"
#include "encoding.h"
#include "encoding_state.h"
#include <string.h>

struct encoding_state read_encoding;
/* SYS_NEWLINE is defined in config.h */
struct encoding_state write_encoding =
{ "ASCII", "ASCII", ONE_BYTE, WITHOUT_BOM, SYS_NEWLINE };

Enc_from write_encoding_from   = ENC_FROM_FILE;
Enc_from write_terminator_from = ENC_FROM_SYS;

const char* terminator[] = {
  /* END_CR */     "\x0D",
  /* END_LF */     "\x0A",
  /* END_CR_LF */  "\x0D\x0A",
  /* END_LF_CR */  "\x0A\x0D"
};

void set_read_encoding(const char* charset, const char* encoding)
{
  strncpy(read_encoding.charset, charset, MAX_CHARSET_LEN);
  read_encoding.encoding = encoding;
  gedcom_debug_print("Encoding state is now: ");
  gedcom_debug_print("  charset   : %s", read_encoding.charset);
  gedcom_debug_print("  encoding  : %s", read_encoding.encoding);
  gedcom_debug_print("  width     : %d", read_encoding.width);
  gedcom_debug_print("  BOM       : %d", read_encoding.bom);
  gedcom_debug_print("  terminator: 0x%02x 0x%02x",
		     read_encoding.terminator[0],
		     read_encoding.terminator[1]);
}

void set_read_encoding_width(Encoding enc)
{
  read_encoding.width = enc;
}

void set_read_encoding_bom(Enc_bom bom)
{
  read_encoding.bom = bom;
}

void set_read_encoding_terminator(char* term)
{
  strncpy(read_encoding.terminator, term, MAX_TERMINATOR_LEN);
}

int gedcom_write_set_encoding(Enc_from from, const char* new_charset,
			      Encoding width, Enc_bom bom)
{
  char* new_encoding = NULL;
  if (from == ENC_FROM_SYS) {
    return 1;
  }
  write_encoding_from = from;
  if (from == ENC_MANUAL) {
    if (!strcmp(new_charset, "UNICODE")) {
      if (width == ONE_BYTE) {
	gedcom_error(_("Unicode cannot be encoded into one byte"));
	return 1;
      }
      else {
	new_encoding = get_encoding(new_charset, width);
	if (new_encoding) {
	  write_encoding.encoding = new_encoding;
	  write_encoding.width = width;
	  write_encoding.bom   = bom;
	  strncpy(write_encoding.charset, new_charset, MAX_CHARSET_LEN);
	}
	else
	  return 1;
      }
    }
    else {
      new_encoding = get_encoding(new_charset, ONE_BYTE);
      if (new_encoding) {
	write_encoding.encoding = new_encoding;
	write_encoding.width = ONE_BYTE;
	write_encoding.bom   = bom;
	strncpy(write_encoding.charset, new_charset, MAX_CHARSET_LEN);
      }
      else
	return 1;
    }
  }
  return 0;
}

void init_write_encoding()
{
  if (write_encoding_from == ENC_FROM_FILE
      && read_encoding.charset[0] != '\0') {
    strncpy(write_encoding.charset, read_encoding.charset, MAX_CHARSET_LEN);
    write_encoding.encoding = read_encoding.encoding;
    write_encoding.width    = read_encoding.width;
    write_encoding.bom      = read_encoding.bom;
  }
}

int gedcom_write_set_line_terminator(Enc_from from, Enc_line_end end)
{
  const char* new_term = NULL;
  write_terminator_from = from;
  if (from == ENC_FROM_SYS) {
    new_term = SYS_NEWLINE;
  }
  else if (from == ENC_MANUAL) {
    new_term = terminator[end];
  }
  if (new_term)
    strncpy(write_encoding.terminator, new_term, MAX_TERMINATOR_LEN);
  return 0;
}

void init_write_terminator()
{
  if (write_terminator_from == ENC_FROM_FILE
      && read_encoding.terminator[0] != '\0') {
    strncpy(write_encoding.terminator, read_encoding.terminator,
	    MAX_TERMINATOR_LEN);
  }
}

