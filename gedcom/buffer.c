/* Implementation of a safe string buffer
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

#include <stdlib.h>
#include <stdio.h>

#include "buffer.h"
#include "gedcom_internal.h"

#define INITIAL_BUF_SIZE 256

void reset_buffer(struct safe_buffer* b)
{
  if (b && b->buffer != NULL) {
    memset(b->buffer, 0, b->bufsize);
    b->buf_end = b->buffer;
    b->buflen  = 0;
  }
}

void cleanup_buffer(struct safe_buffer *b)
{
  if (b && b->buffer)
    free(b->buffer);
}

void init_buffer(struct safe_buffer *b)
{
  if (b && b->buffer == NULL) {
    b->buffer = (char *)malloc(INITIAL_BUF_SIZE);
    if (b->buffer) {
      b->bufsize = INITIAL_BUF_SIZE;
      memset(b->buffer, 0, b->bufsize);
      b->buf_end = b->buffer;
      b->buflen  = 0;
      if (b->cleanup_func && atexit(b->cleanup_func) != 0) {
	fprintf(stderr, _("Could not register buffer cleanup function"));
	fprintf(stderr, "\n");
      }
    }
    else {
      fprintf(stderr, _("Could not allocate memory at %s, %d"),
	      __FILE__, __LINE__);
      fprintf(stderr, "\n");
    }
  }
}

void grow_buffer(struct safe_buffer *b)
{
  char* new_buffer;
  size_t old_size = b->bufsize;
  b->bufsize *= 2;
  new_buffer = realloc(b->buffer, b->bufsize);
  if (new_buffer) {
    b->buffer = new_buffer;
    memset(b->buffer + old_size, 0, b->bufsize - old_size);
    b->buf_end = b->buffer + b->buflen;
  }
  else
    b->buffer = NULL;
}

int safe_buf_vappend(struct safe_buffer *b, const char *s, va_list ap)
{
  int res = 0;
  init_buffer(b);
  if (b && b->buffer) {
    while (1) {
      int rest_size = b->bufsize - b->buflen;
      
      res = vsnprintf(b->buf_end, rest_size, s, ap);
      
      if (res > -1 && res < rest_size) {
	b->buf_end = b->buf_end + res;
	b->buflen  = b->buflen + res;
	break;
      }
      else {
	grow_buffer(b);
      }
    }
  }
  return res;
}

int safe_buf_append(struct safe_buffer *b, const char *s, ...)
{
  int res;
  va_list ap;
  
  va_start(ap, s);
  res = safe_buf_vappend(b, s, ap);
  va_end(ap);
  
  return res;
}

char* get_buf_string(struct safe_buffer *b)
{
  return b->buffer;
}
