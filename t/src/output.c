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

#include "output.h"
#include <stdio.h>
#include <stdarg.h>

#define OUTFILE "check.out"
FILE* outfile = NULL;
int quiet = 0;

void output(int to_stdout_too, const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  if (outfile) {
    vfprintf(outfile, format, ap);
  }
  if (to_stdout_too && !quiet) {
    vprintf(format, ap);
  }
  va_end(ap);
}

void output_set_quiet(int q)
{
  quiet = q;
}

void output_open(const char *outfilename)
{
  if (!outfilename)
    outfilename = OUTFILE;
  outfile = fopen(outfilename, "a");
  if (!outfile) {
    printf("Could not open %s for appending\n", outfilename);
  }
}

void output_close()
{
  fclose(outfile);
}

