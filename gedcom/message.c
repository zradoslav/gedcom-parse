/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

#include "gedcom_internal.h"
#include "gedcom.h"

#define INITIAL_BUF_SIZE 256
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

void init_mess_buffer()
{
  if (mess_buffer == NULL) {
    mess_buffer = (char *)malloc(INITIAL_BUF_SIZE);
    mess_buffer[0] = '\0';
    bufsize = INITIAL_BUF_SIZE;
  }
}

int safe_buf_vappend(char *s, va_list ap)
{
  int res;
  int len;
  init_mess_buffer();
  len = strlen(mess_buffer);
  while (1) {
    char *buf_ptr = mess_buffer + len;
    int rest_size = bufsize - len;
    
    res = vsnprintf(buf_ptr, rest_size, s, ap);
    
    if (res > -1 && res < rest_size) {
      break;
    }
    else  {
      bufsize *= 2;
      mess_buffer = realloc(mess_buffer, bufsize);
    }
  }
  return res;  
}

int safe_buf_append(char *s, ...)
{
  int res;
  va_list ap;
  
  va_start(ap, s);
  res = safe_buf_vappend(s, ap);
  va_end(ap);
  
  return res;
}

int gedcom_message(char* s, ...)
{
  int res;
  va_list ap;

  va_start(ap, s);
  reset_mess_buffer();
  res = safe_buf_vappend(s, ap);
  va_end(ap);
  safe_buf_append("\n");
  if (msg_handler)
    (*msg_handler)(MESSAGE, mess_buffer);
  return res;
}

int gedcom_warning(char* s, ...)
{
  int res;
  va_list ap;

  reset_mess_buffer();
  safe_buf_append("Warning on line %d: ", line_no);
  va_start(ap, s);
  res = safe_buf_vappend(s, ap);
  va_end(ap);
  safe_buf_append("\n");
  if (msg_handler)
    (*msg_handler)(WARNING, mess_buffer);
  
  return res;
}

int gedcom_error(char* s, ...)
{
  int res;
  va_list ap;

  reset_mess_buffer();
  safe_buf_append("Error on line %d: ", line_no);
  va_start(ap, s);
  res = safe_buf_vappend(s, ap);
  va_end(ap);
  safe_buf_append("\n");
  if (msg_handler)
    (*msg_handler)(ERROR, mess_buffer);
  
  return res;
}