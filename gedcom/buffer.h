/* Header for buffer.h
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

#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdarg.h>
#include <sys/types.h>

struct safe_buffer {
  char* buffer;
  size_t bufsize;
  void (*cleanup_func)(void);
};

void init_buffer(struct safe_buffer* b);
void reset_buffer(struct safe_buffer* b);
void cleanup_buffer(struct safe_buffer* b);

int safe_buf_vappend(struct safe_buffer* b, const char* s, va_list ap);
int safe_buf_append(struct safe_buffer* b, const char* s, ...);
char* get_buf_string(struct safe_buffer* b);

#endif /* __BUFFER_H */
