/* Test program for the Gedcom library.
   Copyright (C) 2001, 2002 The Genes Development Team
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

#include <string.h>
#include "config.h"

char* null_str = "(null)";
char* non_null_ptr = "0x<non-null>";
char* null_ptr = "0x<null>";

char* str_val(char* input)
{
  if (input)
    return input;
  else
    return null_str;
}

char* ptr_val(void* ptr)
{
  if (ptr)
    return non_null_ptr;
  else
    return null_ptr;
}

long int void_ptr_to_int(void* ptr)
{
  long int i;
#if SIZEOF_VOID_P == 4
    i = (int)ptr;
#else
# if SIZEOF_VOID_P == 8
    i = (long int)ptr;
# else
#  error "Unhandled case for size of void pointer!"
# endif
#endif
  return i;
}

void* int_to_void_ptr(int i)
{
#if SIZEOF_VOID_P == 4
    int t;
#else
# if SIZEOF_VOID_P == 8
    long int t;
# else
#  error "Unhandled case for size of void pointer!"
# endif
#endif
  t = i;
  return (void*)t;
}

char* simple_base_name(char* filename)
{
  char* runner = NULL;
  if (filename) {
    runner = filename + strlen(filename) - 1;
    while (runner != filename && *(runner-1) != '/') runner--;
  }
  return runner;
}
