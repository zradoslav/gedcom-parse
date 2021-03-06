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
#include "encoding_state.h"
#include "tag_data.h"
#include "buffer.h"
#include "utf8tools.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXWRITELEN MAXGEDCLINELEN

struct Gedcom_write_struct {
  int       filedesc;
  convert_t conv;
  int       total_conv_fails;
  const char* term;
  int       ctxt_stack[MAXGEDCLEVEL+1];
  int       ctxt_level;
};

void cleanup_write_buffer();
struct safe_buffer write_buffer = { NULL, 0, NULL, 0, cleanup_write_buffer };

void cleanup_convert_at_buffer();
struct safe_buffer convert_at_buffer = { NULL, 0, NULL, 0,
					 cleanup_convert_at_buffer };

void cleanup_write_buffer()
{
  cleanup_buffer(&write_buffer);
}

void cleanup_convert_at_buffer()
{
  cleanup_buffer(&convert_at_buffer);
}

int write_simple(Gedcom_write_hndl hndl,
		 int level, const char* xref, const char* tag,
		 const char* value)
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

    if (utf8_strlen(get_buf_string(&write_buffer)) > MAXGEDCLINELEN) {
      gedcom_error(_("Line too long"));
    }
    else {
      converted = convert_from_utf8(hndl->conv, get_buf_string(&write_buffer),
				    &conv_fails, &outlen);
      
      if (converted && (conv_fails == 0)) {
	line_no++;
	write(hndl->filedesc, converted, outlen);
      }
      else {
	hndl->total_conv_fails += conv_fails;
	gedcom_error
	  (_("Error converting output string: %s (%d conversion failures)"),
	   strerror(errno), conv_fails);
      }
    }
  }
  return 0;
}

int write_encoding_value(Gedcom_write_hndl hndl,
			 int level, const char* xref, const char* tag,
			 const char* value)
{
  if (strcmp(value, write_encoding.charset))
    gedcom_warning(_("Forcing HEAD.CHAR value to '%s'"),
		   write_encoding.charset);
  return write_simple(hndl, level, xref, tag, write_encoding.charset);
}

int supports_continuation(int elt_or_rec, int which_continuation)
{
  return tag_data[elt_or_rec].options & which_continuation;
}

int write_long(Gedcom_write_hndl hndl, int elt_or_rec,
	       int level, const char* xref, const char* tag, const char* value)
{
  int prefix_len, value_len = 0, term_len;
  char* nl_pos = NULL;
  if (value) nl_pos = strchr(value, '\n');

  prefix_len = utf8_strlen(tag) + 3;  /* for e.g. "0 INDI " */
  if (level > 9) prefix_len++;
  if (xref)      prefix_len += utf8_strlen(xref) + 1;
  if (value)     value_len  = utf8_strlen(value);
  term_len   = strlen(hndl->term);

  if (!nl_pos && prefix_len + value_len + term_len <= MAXWRITELEN)
    write_simple(hndl, level, xref, tag, value);
  else {
    const char* value_ptr = value;
    int cont_supported = supports_continuation(elt_or_rec, OPT_CONT);
    int cont_as_conc   = supports_continuation(elt_or_rec, OPT_CONT_AS_CONC);
    if (nl_pos && !cont_supported) {
      gedcom_error (_("The tag %s doesn't support newlines"), tag);
      return 1;
    }
    else {
      char value_part[MAXWRITELEN];
      int cont_prefix_len, write_level = level;
      cont_prefix_len = utf8_strlen("CONT") + 3;
      if (level + 1 > 9) cont_prefix_len++;

      while (value_ptr) {
	char* cont_tag = "CONT";
	int line_len = (nl_pos && cont_supported
			? nl_pos - value_ptr : value_len);

	if (prefix_len + line_len + term_len > MAXWRITELEN) {
	  line_len = MAXWRITELEN - prefix_len - term_len;
	  if (!cont_as_conc) {
	    cont_tag = "CONC";
	    while (value_ptr[line_len] == ' '
		   || value_ptr[line_len-1] == ' ') {
	      line_len--;
	    }
	  }
	}
	
	memset(value_part, 0, sizeof(value_part));
	strncpy(value_part, value_ptr, line_len);
	write_simple(hndl, write_level, xref, tag, value_part);
	
	if (line_len < value_len) {
	  value_ptr   = value_ptr + line_len;
	  value_len   = value_len - line_len;
	  if (*value_ptr == '\n') {
	    value_ptr++;
	    value_len--;
	  }
	  prefix_len  = cont_prefix_len;
	  write_level = level + 1;
	  xref        = NULL;
	  tag         = cont_tag;
	  nl_pos      = strchr(value_ptr, '\n');
	}
	else
	  value_ptr = NULL;
      }
    }
  }
  
  return 0;
}

/** The basic function for opening a GEDCOM file for writing.

    \param filename  The name of the file to write

    \return A write handle, which needs to be used in the writing functions,
    or \c NULL in case of errors.
 */
Gedcom_write_hndl gedcom_write_open(const char *filename)
{
  Gedcom_write_hndl hndl;

  hndl = (Gedcom_write_hndl)malloc(sizeof(struct Gedcom_write_struct));

  if (!hndl)
    MEMORY_ERROR;
  else {
    init_write_encoding();
    init_write_terminator();
    hndl->total_conv_fails = 0;
    hndl->conv = initialize_utf8_conversion(write_encoding.encoding, 0);
    if (!hndl->conv) {
      gedcom_error(_("Could not open encoding '%s' for writing: %s"),
		   write_encoding.encoding, strerror(errno));
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
	hndl->term = write_encoding.terminator;
	hndl->ctxt_level = -1;
	if (write_encoding.bom == WITH_BOM) {
	  if (write_encoding.width == TWO_BYTE_HILO)
	    write(hndl->filedesc, "\xFE\xFF", 2);
	  else if (write_encoding.width == TWO_BYTE_LOHI)
	    write(hndl->filedesc, "\xFF\xFE", 2);
	  else if (!strcmp(write_encoding.encoding, "UTF-8"))
	    write(hndl->filedesc, "\xEF\xBB\xBF", 3);
	  else
	    gedcom_warning(_("Byte order mark configured, but not relevant"));
	}
      }
    }
  }

  return hndl;
}

/** The basic function for closing a GEDCOM file for writing.

    \param hndl  The write handle as returned by gedcom_write_open().
    \param total_conv_fails  If you pass an actual integer pointer for this,
    the function will write in it the total number of conversion failures;
    you can pass \c NULL if you're not interested

    \retval 0 in case of success
    \retval >0 in case of failure.
 */
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

char* get_tag_string(int elt_or_rec, int tag)
{
  int tagnum = tag_data[elt_or_rec].tag;
  if (!tagnum) tagnum = tag;

  if (tagnum) {
    if (tagnum >= TAG_NUM_START && tagnum <= TAG_NUM_END)
      return tag_name[tagnum - TAG_NUM_START];
    else {
      gedcom_error(_("Not a valid tag: %d"), tagnum);
      return NULL;
    }
  }
  else {
    gedcom_error(_("The element or record type '%s' requires a specific tag "
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

char* convert_at(const char* input)
{
  if (input) {
    const char* ptr = input;
    reset_buffer(&convert_at_buffer);
    while (*ptr) {
      if (*ptr == '@') {
	SAFE_BUF_ADDCHAR(&convert_at_buffer, '@');
	SAFE_BUF_ADDCHAR(&convert_at_buffer, '@');
      }
      else {
	SAFE_BUF_ADDCHAR(&convert_at_buffer, *ptr);
      }
      ptr++;
    }
    return get_buf_string(&convert_at_buffer);
  }
  else
    return NULL;
}

int _gedcom_write_val(Gedcom_write_hndl hndl,
		      int rec_or_elt, int tag, int parent_rec_or_elt,
		      const char* xrefstr, const char* val)
{
  int result = 1;
  int level = 0;
  char* tag_str = NULL;

  tag_str = get_tag_string(rec_or_elt, tag);
  level   = get_level(hndl, rec_or_elt, parent_rec_or_elt);
  if (tag_str && (level != -1)) {
    if (rec_or_elt == ELT_HEAD_CHAR)
      result = write_encoding_value(hndl, level, xrefstr, tag_str, val);
    else if (supports_continuation(rec_or_elt, OPT_CONT|OPT_CONC))
      result = write_long(hndl, rec_or_elt, level, xrefstr, tag_str, val);
    else
      result = write_simple(hndl, level, xrefstr, tag_str, val);
  }

  return result;
}

/** Function for writing lines corresponding to standard records (i.e. on
    level 0).

    \param hndl The write handle that was returned by gedcom_write_open().
    \param rec  One of the identifiers given in the first column in
    <a href=interface.html#Record_identifiers>this table</a> (except REC_USER).
    \param xrefstr The cross-reference key of the record (something like
    \c "@FAM01@".
    \param val  The value of the record line, which should be \c NULL for some
    record types, according to
    <a href=interface.html#Record_identifiers>this table</a>.

    \retval 0 on success
    \retval >0 on failure
*/  
int gedcom_write_record_str(Gedcom_write_hndl hndl,
			    Gedcom_rec rec, const char* xrefstr,
			    const char* val)
{
  int result = 1;
  if (check_type(rec, (val ? GV_CHAR_PTR : GV_NULL)))
    result = _gedcom_write_val(hndl, rec, 0, -1, xrefstr, convert_at(val));
  return result;
}

/** Function for writing lines corresponding to standard elements (i.e. on
    level bigger than 0), with a string as value.

    \param hndl The write handle that was returned by gedcom_write_open().
    \param elt  One of the identifiers given in the first column in
    <a href=interface.html#Element_identifiers>this table</a>
    (except ELT_USER).
    \param tag Some of the \c elt identifiers can actually stand for different
    tags.  For this reason, the \c tag has to be passed for some of them.  This
    parsed tag is the same as was returned by the callback functions, and is
    an identifier of the form <code>TAG_<em>name</em></code>.  This parameter
    is needed whenever the second column in 
    <a href=interface.html#Element_identifiers>this table</a> shows several
    possible tags (this is e.g. the case for \c ELT_SUB_FAM_EVT).  Otherwise,
    you can pass 0.
    \param parent_rec_or_elt The corresponding \c rec or \c elt identifier of
    the logically enclosing statement: this will determine the level number
    written on the line, as the level number of the parent + 1.
    \param val  The value of the element line, which should be \c NULL for some
    element types, according to
    <a href=interface.html#Element_identifiers>this table</a>.

    \retval 0 on success
    \retval >0 on failure
*/  
int gedcom_write_element_str(Gedcom_write_hndl hndl,
			     Gedcom_elt elt, int tag, int parent_rec_or_elt,
			     const char* val)
{
  int result = 1;
  if (check_type(elt, (val ? GV_CHAR_PTR : GV_NULL)))
    result = _gedcom_write_val(hndl, elt, tag, parent_rec_or_elt, NULL,
			       convert_at(val));
  return result;
}

/** Function for writing lines corresponding to standard elements (i.e. on
    level bigger than 0), with a cross-reference as value.

    See gedcom_write_element_str() for details.
*/  
int gedcom_write_element_xref(Gedcom_write_hndl hndl,
			      Gedcom_elt elt, int tag, int parent_rec_or_elt,
			      const struct xref_value* val)
{
  int result = 1;
  if (check_type(elt, (val ? GV_XREF_PTR : GV_NULL)))
    result = _gedcom_write_val(hndl, elt, tag, parent_rec_or_elt, NULL,
			       val->string);
  return result;
}

/** Function for writing lines corresponding to standard elements (i.e. on
    level bigger than 0), with a date as value.

    See gedcom_write_element_str() for details.
*/  
int gedcom_write_element_date(Gedcom_write_hndl hndl,
			      Gedcom_elt elt, int tag, int parent_rec_or_elt,
			      const struct date_value* val)
{
  int result = 1;
  if (check_type(elt, (val ? GV_DATE_VALUE : GV_NULL)))
    result = _gedcom_write_val(hndl, elt, tag, parent_rec_or_elt, NULL,
			       gedcom_date_to_string(val));
  return result;
}

/** Function for writing lines corresponding to standard elements (i.e. on
    level bigger than 0), with an age as value.

    See gedcom_write_element_str() for details.
*/  
int gedcom_write_element_age(Gedcom_write_hndl hndl,
			     Gedcom_elt elt, int tag, int parent_rec_or_elt,
			     const struct age_value* val)
{
  int result = 1;
  if (check_type(elt, (val ? GV_AGE_VALUE : GV_NULL)))
    result = _gedcom_write_val(hndl, elt, tag, parent_rec_or_elt, NULL,
			       gedcom_age_to_string(val));
  return result;
}

/** Function for writing lines corresponding to user-defined records and
    elements, with a string as value.

    In the case of user-defined tags, the
    level and tag string are passed verbatim (not controlled by the library).
    This allows to write any extra data that doesn't use a standard tag, but
    is only allowed for tags starting with an underscore.

    \param hndl The write handle that was returned by gedcom_write_open().
    \param level  The integer level of the GEDCOM line
    \param tag  The tag, as a literal string
    \param xrefstr An optional cross-reference of the record or element.
    \param value The value of the record or element line.

    \retval 0 on success
    \retval >0 on failure
*/  
int gedcom_write_user_str(Gedcom_write_hndl hndl, int level, const char* tag,
			  const char* xrefstr, const char* value)
{
  int result = 1;
  if (tag && tag[0] == '_')
    result = write_simple(hndl, level, xrefstr, tag, convert_at(value));
  return result;
}

/** Function for writing lines corresponding to user-defined records and
    elements, with a cross-reference as value.

    See gedcom_write_user_str() for details.
*/  
int gedcom_write_user_xref(Gedcom_write_hndl hndl, int level, const char* tag,
			   const char* xrefstr, const struct xref_value* val)
{
  int result = 1;
  if (tag && tag[0] == '_')
    result = write_simple(hndl, level, xrefstr, tag, val->string);
  return result;
}
