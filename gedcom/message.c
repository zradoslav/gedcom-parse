/* Implementation of the messaging API to applications.
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
#include "gedcom.h"
#include "buffer.h"

void cleanup_mess_buffer();

struct safe_buffer mess_buffer = { NULL, 0, NULL, 0, cleanup_mess_buffer };
Gedcom_msg_handler msg_handler = NULL;

/** This function registers a callback that is called if there are errors,
    warnings or just messages coming from the parser.

    \param func The callback to be called on errors, warnings or messages; see
    \ref Gedcom_msg_handler for the signature of the callback.
*/
void gedcom_set_message_handler(Gedcom_msg_handler func)
{
  msg_handler = func;
}

void cleanup_mess_buffer()
{
  cleanup_buffer(&mess_buffer);
}

int gedcom_message(const char* s, ...)
{
  int res;
  va_list ap;

  va_start(ap, s);
  reset_buffer(&mess_buffer);
  res = safe_buf_vappend(&mess_buffer, s, ap);
  va_end(ap);
  if (msg_handler)
    (*msg_handler)(MESSAGE, get_buf_string(&mess_buffer));
  return res;
}

int gedcom_warning(const char* s, ...)
{
  int res;
  va_list ap;

  reset_buffer(&mess_buffer);
  if (line_no != 0) 
    safe_buf_append(&mess_buffer, _("Warning on line %d: "), line_no);
  else
    safe_buf_append(&mess_buffer, _("Warning: "));
  va_start(ap, s);
  res = safe_buf_vappend(&mess_buffer, s, ap);
  va_end(ap);
  if (msg_handler)
    (*msg_handler)(WARNING, get_buf_string(&mess_buffer));
  
  return res;
}

int gedcom_error(const char* s, ...)
{
  int res;
  va_list ap;

  reset_buffer(&mess_buffer);
  if (line_no != 0)
    safe_buf_append(&mess_buffer, _("Error on line %d: "), line_no);
  else
    safe_buf_append(&mess_buffer, _("Error: "));
  va_start(ap, s);
  res = safe_buf_vappend(&mess_buffer, s, ap);
  va_end(ap);
  if (msg_handler)
    (*msg_handler)(ERROR, get_buf_string(&mess_buffer));
  
  return res;
}

void gedcom_mem_error(const char *filename, int line)
{
  gedcom_error(_("Could not allocate memory at %s, %d"), filename, line);
}
