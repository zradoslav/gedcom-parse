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

/** Allows to change the encoding for writing files.  It should be called
    \em before calling gedcom_write_open(), i.e. it affects all files that are
    opened after it is being called.

    Valid values for the character set are given in
    the first column in the file \c gedcom.enc in the data directory of
    gedcom-parse (\c $PREFIX/share/gedcom-parse).  The character sets UNICODE,
    ASCII and ANSEL are always supported (these are standard for GEDCOM), as
    well as ANSI (not standard), but there may be others.

    Note that you still need to pass the correct charset value for the
    \c HEAD.CHAR tag, otherwise you will get a warning and the value will
    be forced to the correct value.

    \param from Indicates how you want the encoding to be set.  When
    ENC_FROM_FILE is selected, the other parameters in the function are ignored
    (they can be passed as 0).  ENC_FROM_SYS is not a valid value here.
    The default setting is ENC_FROM_FILE.
    \param charset  The character set to be used.
    \param width    The width and endianness of the character set.  You can
    pass 0 for non-UNICODE encodings.
    \param bom      Determines whether a byte-order-mark should be written in
    the file in case of UNICODE encoding (usually preferred because it then
    clearly indicates the byte ordering).  You can pass 0 for non-UNICODE
    encodings, but the byte-order-mark can also be used for UTF-8.

    \retval 0 in case of success
    \retval >0 in case of error
 */
int gedcom_write_set_encoding(Enc_from from, const char* charset,
			      Encoding width, Enc_bom bom)
{
  const char* new_charset = charset;
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

/** Allows to change the line terminator to use on writing.  It should be
    called
    \em before calling gedcom_write_open(), i.e. it affects all files that are
    opened after it is being called.

    By default, the line terminator is set to the appropriate line terminator
    on the current platform, so it only needs to be changed if there is some
    special reason for it.

    \param from Indicates how you want the encoding to be set.  When
    ENC_FROM_FILE or ENC_FROM_SYS is selected, the other parameter in the
    function is ignored (and can be passed as 0).
    The default setting is ENC_FROM_SYS.
    \param end  The wanted line terminator.

    \retval 0 if success
    \retval >0 if failure
*/
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

