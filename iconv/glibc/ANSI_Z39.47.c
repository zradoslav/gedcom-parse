/* Conversion for ANSI_Z39.47 aka ANSEL.
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

/* Generic conversion to and from ANSI Z39.47 (also known as ANSEL)
   Based on the ansi_x3.110.c file from the glibc sources
   Data coming from:
   http://lcweb.loc.gov/marc/specifications/speccharlatin.html

   Note: in ANSEL, diacritical marks come *before* the base character;
   in Unicode, they come *after*...
*/

#include <dlfcn.h>
#include <gconv.h>
#include <stdint.h>
#include <string.h>
#include "ANSI_Z39.47-tables.h"

/* From /usr/include/linux/compiler.h out of GCC 2.96+: */
/* Somewhere in the middle of the GCC 2.96 development cycle, we implemented
   a mechanism by which the user can annotate likely branch directions and
   expect the blocks to be reordered appropriately.  Define __builtin_expect
   to nothing for earlier compilers.  */

#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define __builtin_expect(x, expected_value) (x)
#endif

/* Omit first half of table: assume identity mapping (ASCII) */
static const uint32_t to_ucs4[128] = TABLE_TO_UCS4_BASIC;

/* The outer array range runs from 0xe0 to 0xfe, the inner range from 0x20
   to 0x7f.  */
static const uint32_t to_ucs4_comb[31][96] =
{
  /* 0xe0 (hook above) */                           TABLE_TO_UCS4_COMBINING_E0,
  /* 0xe1 (grave) */                                TABLE_TO_UCS4_COMBINING_E1,
  /* 0xe2 (acute) */                                TABLE_TO_UCS4_COMBINING_E2,
  /* 0xe3 (circumflex) */                           TABLE_TO_UCS4_COMBINING_E3,
  /* 0xe4 (tilde) */                                TABLE_TO_UCS4_COMBINING_E4,
  /* 0xe5 (macron) */                               TABLE_TO_UCS4_COMBINING_E5,
  /* 0xe6 (breve) */                                TABLE_TO_UCS4_COMBINING_E6,
  /* 0xe7 (dot above) */                            TABLE_TO_UCS4_COMBINING_E7,
  /* 0xe8 (umlaut, diaeresis) */                    TABLE_TO_UCS4_COMBINING_E8,
  /* 0xe9 (caron, hacek) */                         TABLE_TO_UCS4_COMBINING_E9,
  /* 0xea (ring above) */                           TABLE_TO_UCS4_COMBINING_EA,
  /* 0xeb (ligature, left half) */                  TABLE_TO_UCS4_COMBINING_EB,
  /* 0xec (ligature, right half) */                 TABLE_TO_UCS4_COMBINING_EC,
  /* 0xed (comma above right) */                    TABLE_TO_UCS4_COMBINING_ED,
  /* 0xee (double acute) */                         TABLE_TO_UCS4_COMBINING_EE,
  /* 0xef (candrabindu) */                          TABLE_TO_UCS4_COMBINING_EF,
  /* 0xf0 (cedilla) */                              TABLE_TO_UCS4_COMBINING_F0,
  /* 0xf1 (ogonek, right hook) */                   TABLE_TO_UCS4_COMBINING_F1,
  /* 0xf2 (dot below) */                            TABLE_TO_UCS4_COMBINING_F2,
  /* 0xf3 (double dot below) */                     TABLE_TO_UCS4_COMBINING_F3,
  /* 0xf4 (ring below) */                           TABLE_TO_UCS4_COMBINING_F4,
  /* 0xf5 (double low line) */                      TABLE_TO_UCS4_COMBINING_F5,
  /* 0xf6 (line below) */                           TABLE_TO_UCS4_COMBINING_F6,
  /* 0xf7 (comma below, left hook) */               TABLE_TO_UCS4_COMBINING_F7,
  /* 0xf8 (left half ring below, right cedilla) */  TABLE_TO_UCS4_COMBINING_F8,
  /* 0xf9 (breve below, half circle below) */       TABLE_TO_UCS4_COMBINING_F9,
  /* 0xfa (double tilde, left half) */              TABLE_TO_UCS4_COMBINING_FA,
  /* 0xfb (double tilde, right half) */             TABLE_TO_UCS4_COMBINING_FB,
  /* 0xfc */                                        TABLE_TO_UCS4_COMBINING_FC,
  /* 0xfd */                                        TABLE_TO_UCS4_COMBINING_FD,
  /* 0xfe (comma above, high centered comma) */     TABLE_TO_UCS4_COMBINING_FE,
};

/* Omit first part of table: assume identity mapping (ASCII) */
static const char from_ucs4[][2] =      TABLE_FROM_UCS4_BASIC;
static const char from_ucs4_p01a[][2] = TABLE_FROM_UCS4_PAGE_01A;
static const char from_ucs4_p022[][2] = TABLE_FROM_UCS4_PAGE_022;
static const char from_ucs4_p02b[][2] = TABLE_FROM_UCS4_PAGE_02B;
static const char from_ucs4_p030[][2] = TABLE_FROM_UCS4_PAGE_030;
static const char from_ucs4_p1ea[][2] = TABLE_FROM_UCS4_PAGE_1EA;
static const char from_ucs4_p200[][2] = TABLE_FROM_UCS4_PAGE_200;
static const char from_ucs4_p211[][2] = TABLE_FROM_UCS4_PAGE_211;
static const char from_ucs4_p266[][2] = TABLE_FROM_UCS4_PAGE_266;
static const char from_ucs4_pfe2[][2] = TABLE_FROM_UCS4_PAGE_FE2;

/* Definitions used in the body of the `gconv' function.  */
#define CHARSET_NAME		"ANSI_Z39.47//"
#define FROM_LOOP		from_ansi_z39_47
#define TO_LOOP			to_ansi_z39_47
#define DEFINE_INIT		1
#define DEFINE_FINI		1
#define MIN_NEEDED_FROM		1
#define MAX_NEEDED_FROM		2
#define MIN_NEEDED_TO		4

/* First define the conversion function from ANSI_Z39.47 to UCS4.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_INPUT	MAX_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    uint32_t ch = *inptr;						      \
    int incr;								      \
									      \
    if (__builtin_expect (ch >= 0xe0, 0) && ch <= 0xfe)			      \
      {									      \
	/* Composed character.  First test whether the next character	      \
	   is also available.  */					      \
	uint32_t ch2;							      \
									      \
	if (inptr + 1 >= inend)						      \
	  {								      \
	    /* The second character is not available.  */		      \
	    result = __GCONV_INCOMPLETE_INPUT;				      \
            break;							      \
	  }								      \
									      \
	ch2 = inptr[1];							      \
									      \
	if (__builtin_expect (ch2 < 0x20, 0)				      \
	    || __builtin_expect (ch2 >= 0x80, 0))			      \
	  {								      \
	    /* This is illegal.  */					      \
	    if (! ignore_errors_p ())   	       			      \
	      {								      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
									      \
	    ++*irreversible;						      \
	    incr = 1;							      \
	  }								      \
	else								      \
	  {								      \
	    uint32_t ch3 = to_ucs4_comb[ch - 0xe0][ch2 - 0x20];	       	      \
            if (ch3 != 0) {                                                   \
	      ch = ch3;                                                       \
	      incr = 2;                                                       \
	    }                                                                 \
	    else {                                                            \
              /* mapping for ch2 is an identity, because is ASCII here */     \
              put32 (outptr, ch2);                                            \
              outptr += 4;                                                    \
              ch = to_ucs4[ch - 0x80];                                        \
	      incr = 2;                                                       \
	    }                                                                 \
	  }								      \
      }									      \
    else								      \
      {									      \
        if (__builtin_expect (ch >= 0x80, 0))                                 \
	  ch = to_ucs4[ch - 0x80];					      \
	incr = 1;							      \
      }									      \
									      \
    if (__builtin_expect (ch, 1) == 0 && *inptr != '\0')		      \
      {									      \
	/* This is an illegal character.  */				      \
	if (! ignore_errors_p ())					      \
	  {								      \
	    result = __GCONV_ILLEGAL_INPUT;				      \
	    break;							      \
	  }								      \
      }									      \
    else								      \
      {									      \
	put32 (outptr, ch);						      \
	outptr += 4;							      \
      }									      \
									      \
    inptr += incr;							      \
  }
#define LOOP_NEED_FLAGS
#include "loop.c"


/* Next, define the other direction.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_OUTPUT	MAX_NEEDED_FROM
#define LOOPFCT			TO_LOOP
#define BODY \
  {									      \
    char tmp[2];							      \
    uint32_t ch = get32 (inptr);					      \
    const char *cp;							      \
        								      \
    if (__builtin_expect (ch > 0x017e, 0))				      \
      {									      \
	if (ch >= 0x1a0 && ch < 0x1b4)		       			      \
          cp = from_ucs4_p01a[ch - 0x1a0];                                    \
	else if (ch >= 0x220 && ch < 0x234)   	       			      \
          cp = from_ucs4_p022[ch - 0x220];                                    \
	else if (ch >= 0x2b0 && ch < 0x2e2)   	       			      \
          cp = from_ucs4_p02b[ch - 0x2b0];                                    \
	else if (ch >= 0x300 && ch < 0x337)          			      \
          cp = from_ucs4_p030[ch - 0x300];                                    \
	else if (ch >= 0x1ea0 && ch < 0x1efa)          			      \
          cp = from_ucs4_p1ea[ch - 0x1ea0];                                   \
	else if (ch >= 0x2000 && ch < 0x200f)          			      \
          cp = from_ucs4_p200[ch - 0x2000];                                   \
	else if (ch >= 0x2110 && ch < 0x211a)          			      \
          cp = from_ucs4_p211[ch - 0x2110];                                   \
	else if (ch >= 0x2660 && ch < 0x2674)          			      \
          cp = from_ucs4_p266[ch - 0x2660];                                   \
	else if (ch >= 0xfe20 && ch < 0xfe25)          			      \
          cp = from_ucs4_pfe2[ch - 0xfe20];                                   \
	else								      \
	  {								      \
	    UNICODE_TAG_HANDLER (ch, 4);				      \
									      \
	    /* Illegal characters.  */					      \
	    STANDARD_ERR_HANDLER (4);					      \
	  }								      \
      }									      \
    else								      \
      {									      \
        if (__builtin_expect (ch < 0x80, 1)) {                                \
	  tmp[0] = ch;                                                        \
	  tmp[1] = '\0';                                                      \
	  cp = tmp;                                                           \
	}                                                                     \
        else                                                                  \
          cp = from_ucs4[ch-0x80];					      \
        if (__builtin_expect (ch >= 0x20, 1)                                  \
	    && __builtin_expect (ch < 0x80, 1))                               \
        {                                                                     \
	  /* Check whether the next character is an accent, if so, then */    \
	  /* output it first */                                               \
	  uint32_t ch2;                                                       \
          inptr += 4;                                                         \
          ch2 = get32 (inptr);                                                \
	  if (ch2 >= 0x300 && ch2 < 0x337) {                                  \
	    const char* cp2 = from_ucs4_p030[ch2 - 0x300];                    \
	    if (cp2[0] != '\0') {                                             \
	      *outptr++ = cp2[0];                                             \
	    }                                                                 \
            else                                                              \
              inptr -= 4;                                                     \
	  }                                                                   \
          else if (ch2 >= 0xfe20 && ch2 < 0xfe25) {                           \
	    const char* cp2 = from_ucs4_pfe2[ch2 - 0xfe20];                   \
	    if (cp2[0] != '\0') {                                             \
	      *outptr++ = cp2[0];                                             \
	    }                                                                 \
	    else                                                              \
	      inptr -= 4;                                                     \
	  }                                                                   \
          else                                                                \
            inptr -= 4;                                                       \
	}                                                                     \
      }         							      \
                                                                              \
    if (__builtin_expect (cp[0], '\1') == '\0' && ch != 0)		      \
      {								              \
        /* Illegal characters.  */					      \
	STANDARD_ERR_HANDLER (4);					      \
      }								              \
									      \
    *outptr++ = cp[0];							      \
    /* Now test for a possible second byte and write this if possible.  */    \
    if (cp[1] != '\0')							      \
      {									      \
	if (__builtin_expect (outptr >= outend, 0))	 		      \
	  {								      \
	    /* The result does not fit into the buffer.  */		      \
	    --outptr;							      \
	    result = __GCONV_FULL_OUTPUT;				      \
	    break;							      \
	  }								      \
									      \
	*outptr++ = cp[1];						      \
      }									      \
									      \
    inptr += 4;								      \
  }
#define LOOP_NEED_FLAGS
#include "loop.c"


/* Now define the toplevel functions.  */
#include "skeleton.c"
