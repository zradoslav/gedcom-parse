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

#if HAVE_VSNPRINTF
#define INITIAL_BUF_SIZE 256
#else
/* Risk on overflowing buffer, so make size big */
#define INITIAL_BUF_SIZE 65536
#endif

char *mess_buffer = NULL;
size_t bufsize;

Gedcom_msg_handler msg_handler = NULL;

void gedcom_set_message_handler(Gedcom_msg_handler func)
{
  msg_handler = func;
}

void reset_mess_buffer()
{
  if (mess_buffer != NULL)
    mess_buffer[0] = '\0';
}

void cleanup_mess_buffer()
{
  if (mess_buffer)
    free(mess_buffer);
}

void init_mess_buffer()
{
  if (mess_buffer == NULL) {
    mess_buffer = (char *)malloc(INITIAL_BUF_SIZE);
    if (mess_buffer) {
      mess_buffer[0] = '\0';
      bufsize = INITIAL_BUF_SIZE;
      if (atexit(cleanup_mess_buffer) != 0)
	gedcom_warning(_("Could not register buffer cleanup function"));
    }
    else {
      fprintf(stderr, _("Could not allocate memory at %s, %d"),
	      __FILE__, __LINE__);
      fprintf(stderr, "\n");
    }
  }
}

int safe_buf_vappend(const char *s, va_list ap)
{
  int res = 0;
  int len;
  init_mess_buffer();
  if (mess_buffer) {
    len = strlen(mess_buffer);
    while (1) {
      char *buf_ptr = mess_buffer + len;
      int rest_size = bufsize - len;
      
#if HAVE_VSNPRINTF
      res = vsnprintf(buf_ptr, rest_size, s, ap);
      
      if (res > -1 && res < rest_size) {
	break;
      }
      else  {
	bufsize *= 2;
	mess_buffer = realloc(mess_buffer, bufsize);
      }
#else /* not HAVE_VSNPRINTF */
#  if HAVE_VSPRINTF
#     warning "Using VSPRINTF. Buffer overflow could happen!"
      vsprintf(buf_ptr, s, ap);
      break;
#  else /* not HAVE_VPRINTF */
#     error "Your standard library has neither vsnprintf nor vsprintf defined. One of them is required!"
#  endif
#endif
    }
  }
  return res;
}

int safe_buf_append(const char *s, ...)
{
  int res;
  va_list ap;
  
  va_start(ap, s);
  res = safe_buf_vappend(s, ap);
  va_end(ap);
  
  return res;
}

int gedcom_message(const char* s, ...)
{
  int res;
  va_list ap;

  va_start(ap, s);
  reset_mess_buffer();
  res = safe_buf_vappend(s, ap);
  va_end(ap);
  if (msg_handler)
    (*msg_handler)(MESSAGE, mess_buffer);
  return res;
}

int gedcom_warning(const char* s, ...)
{
  int res;
  va_list ap;

  reset_mess_buffer();
  if (line_no != 0) 
    safe_buf_append(_("Warning on line %d: "), line_no);
  else
    safe_buf_append(_("Warning: "));
  va_start(ap, s);
  res = safe_buf_vappend(s, ap);
  va_end(ap);
  if (msg_handler)
    (*msg_handler)(WARNING, mess_buffer);
  
  return res;
}

int gedcom_error(const char* s, ...)
{
  int res;
  va_list ap;

  reset_mess_buffer();
  if (line_no != 0)
    safe_buf_append(_("Error on line %d: "), line_no);
  else
    safe_buf_append(_("Error: "));
  va_start(ap, s);
  res = safe_buf_vappend(s, ap);
  va_end(ap);
  if (msg_handler)
    (*msg_handler)(ERROR, mess_buffer);
  
  return res;
}

void gedcom_mem_error(const char *filename, int line)
{
  gedcom_error(_("Could not allocate memory at %s, %d"), filename, line);
}
