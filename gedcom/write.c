/* Write functions for Gedcom.
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
#include "gedcom.h"
#include "encoding.h"
#include "tag_data.h"
#include "buffer.h"
#include "utf8.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char* encoding = "ASCII";
int write_encoding_details = ONE_BYTE;
/* SYS_NEWLINE is defined in config.h */
const char* write_terminator = SYS_NEWLINE;

struct Gedcom_write_struct {
  int       filedesc;
  convert_t conv;
  int       total_conv_fails;
  const char* term;
  int       ctxt_stack[MAXGEDCLEVEL+1];
  int       ctxt_level;
};

const char* default_encoding[] = {
  /* ONE_BYTE */      "ASCII",
  /* TWO_BYTE_HILO */ "UCS-2BE",
  /* TWO_BYTE_LOHI */ "UCS-2LE"
};

const char* terminator[] = {
  /* END_CR */     "\x0D",
  /* END_LF */     "\x0A",
  /* END_CR_LF */  "\x0D\x0A",
  /* END_LF_CR */  "\x0A\x0D"
};

void cleanup_write_buffer();

struct safe_buffer write_buffer = { NULL, 0, cleanup_write_buffer };

void cleanup_write_buffer()
{
  cleanup_buffer(&write_buffer);
}

int write_simple(Gedcom_write_hndl hndl,
		 int level, char* xref, char* tag, char* value)
{
  int res;
  
  if (hndl) {
    char* converted;
    int conv_fails;
    size_t outlen;
    
    reset_buffer(&write_buffer);
    res = safe_buf_append(&write_buffer, "%d", level);
    if (xref)
      res += safe_buf_append(&write_buffer, " %s", xref);
    res += safe_buf_append(&write_buffer, " %s", tag);
    if (value)
      res += safe_buf_append(&write_buffer, " %s", value);
    res += safe_buf_append(&write_buffer, hndl->term);

    converted = convert_from_utf8(hndl->conv, get_buf_string(&write_buffer),
				  &conv_fails, &outlen);
    
    if (converted && (conv_fails == 0))
      write(hndl->filedesc, converted, outlen);
    else {
      hndl->total_conv_fails += conv_fails;
      gedcom_error
	(_("Error converting output string: %s (%d conversion failures)"),
	 strerror(errno), conv_fails);
    }
  }
  return 0;
}

int gedcom_write_set_encoding(const char* charset,
			      Encoding width, Enc_bom bom)
{
  char* new_encoding = NULL;
  if (!strcmp(charset, "UNICODE")) {
    if (width == ONE_BYTE) {
      gedcom_error(_("Unicode cannot be encoded into one byte"));
      return 1;
    }
    else {
      new_encoding = get_encoding(charset, width);
      if (new_encoding) {
	encoding = new_encoding;
	write_encoding_details = width | bom;
      }
    }
  }
  else {
    new_encoding = get_encoding(charset, ONE_BYTE);
    if (new_encoding) {
      encoding = new_encoding;
      write_encoding_details = ONE_BYTE;
    }
  }
  return 0;
}

int gedcom_write_set_line_terminator(Enc_line_end end)
{
  write_terminator = terminator[end];
  return 0;
}

Gedcom_write_hndl gedcom_write_open(const char *filename)
{
  Gedcom_write_hndl hndl;

  hndl = (Gedcom_write_hndl)malloc(sizeof(struct Gedcom_write_struct));

  if (!hndl)
    MEMORY_ERROR;
  else {
    hndl->total_conv_fails = 0;
    hndl->conv = initialize_utf8_conversion(encoding, 0);
    if (!hndl->conv) {
      gedcom_error(_("Could not open encoding '%s' for writing: %s"),
		   encoding, strerror(errno));
      free(hndl);
      hndl = NULL;
    }
    else {
      hndl->filedesc = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0666);
      if (!hndl->filedesc) {
	gedcom_error(_("Could not open file '%s' for writing: %s"),
		     filename, strerror(errno));
	cleanup_utf8_conversion(hndl->conv);
	free(hndl);
	hndl = NULL;
      }
      else {
	hndl->term = write_terminator;
	hndl->ctxt_level = -1;
	if (write_encoding_details & WITH_BOM) {
	  if (write_encoding_details & TWO_BYTE_HILO)
	    write(hndl->filedesc, "\xFE\xFF", 2);
	  else if (write_encoding_details & TWO_BYTE_LOHI)
	    write(hndl->filedesc, "\xFF\xFE", 2);
	  else
	    gedcom_warning(_("Byte order mark configured, but no Unicode"));
	}
      }
    }
  }

  return hndl;
}

int gedcom_write_close(Gedcom_write_hndl hndl, int* total_conv_fails)
{
  int result = 0;
  if (hndl) {
    write_simple(hndl, 0, NULL, "TRLR", NULL);
    if (total_conv_fails)  *total_conv_fails = hndl->total_conv_fails;
    result = close(hndl->filedesc);
    cleanup_utf8_conversion(hndl->conv);
    free(hndl);
  }
  return result;
}

char* get_tag_string(int elt_or_rec, char* tag)
{
  char* result = tag_data[elt_or_rec].tag_name;

  if (result)
    return result;
  else if (tag)
    return tag;
  else {
    gedcom_error(_("The element or record type '%s' requires a specific tag"
		   "for writing"),
		 tag_data[elt_or_rec].elt_name);
    return NULL;
  }
}

int check_type(int elt_or_rec, Gedcom_val_type type)
{
  int allowed = tag_data[elt_or_rec].allowed_types;
  if (allowed & type)
    return 1;
  else {
    gedcom_error(_("Wrong data type for writing element or record type '%s'"),
		 tag_data[elt_or_rec].elt_name);
    return 0;
  }
}

int get_level(Gedcom_write_hndl hndl, int elt_or_rec, int parent)
{
  if (parent == -1) {
    hndl->ctxt_level = 0;
  }
  else {
    while (hndl->ctxt_level && hndl->ctxt_stack[hndl->ctxt_level] != parent)
      hndl->ctxt_level--;
    if (hndl->ctxt_stack[hndl->ctxt_level] == parent) {
      hndl->ctxt_level++;
    }
    else {
      gedcom_error(_("Parent %d not found during write of %d"),
		   parent, elt_or_rec);
      return -1;
    }
  }
  hndl->ctxt_stack[hndl->ctxt_level] = elt_or_rec;
  return hndl->ctxt_level;
}

int gedcom_write_record_str(Gedcom_write_hndl hndl,
			    Gedcom_rec rec, char* tag,
			    struct xref_value* xref, char* val)
{
  int result = 1;
  int level = 0;
  char* tag_str = NULL;
  char* xref_str = NULL;

  tag_str = get_tag_string(rec, tag);
  level   = get_level(hndl, rec, -1);
  if (tag_str && check_type(rec, (val ? GV_CHAR_PTR : GV_NULL))) {
    if (xref)
      xref_str = xref->string;
    result = write_simple(hndl, level, xref_str, tag_str, val);
  }

  return result;
}

int gedcom_write_element_str(Gedcom_write_hndl hndl,
			     Gedcom_elt elt, char* tag, int parent_rec_or_elt,
			     char* val)
{
  int result = 1;
  int level  = -1;
  char* tag_str = NULL;

  tag_str = get_tag_string(elt, tag);
  level   = get_level(hndl, elt, parent_rec_or_elt);
  if (tag_str && (level != -1)
      && check_type(elt, (val ? GV_CHAR_PTR : GV_NULL))) {
    result = write_simple(hndl, level, NULL, tag_str, val);
  }

  return result;
}