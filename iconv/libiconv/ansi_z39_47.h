/*
 * Copyright (C) 1999-2002 Free Software Foundation, Inc.
 * This file is part of the GNU LIBICONV Library.
 *
 * The GNU LIBICONV Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * The GNU LIBICONV Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the GNU LIBICONV Library; see the file COPYING.LIB.
 * If not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * ANSI_Z39.47
 */

#include "ANSI_Z39.47-tables.h"
#include <stdio.h>

/* Omit first half of table: assume identity mapping (ASCII) */
static const unsigned short ansi_z39_47_2uni[128] = TABLE_TO_UCS4_BASIC;

/* The outer array range runs from 0xe0 to 0xfe, the inner range from 0x20
   to 0x7f.  */
static const unsigned short ansi_z39_47_2uni_comb[31][96] =
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

#define BASE_PASSED 0x10000

static int
ansi_z39_47_mbtowc (conv_t conv, ucs4_t *pwc, const unsigned char *s, int n)
{
  unsigned char c = *s;
  ucs4_t last_wc = conv->istate;
  int retval = 1;
  if (last_wc & BASE_PASSED) {
    /* base character was already output, reset the state and output the
       diacritical mark */
    unsigned char dc = (unsigned char)(last_wc & ~BASE_PASSED);
    *pwc = ansi_z39_47_2uni[dc-0x80];
    conv->istate = 0;
    return 1;
  }
  if (last_wc) {
    conv->istate |= BASE_PASSED;
    retval = 0;
  }
  if (c < 0x80) {
    if (last_wc && c >= 0x20) {
      /* Check if we can combine the character with the diacritical mark */
      unsigned char dc = (unsigned char)(last_wc & ~BASE_PASSED);
      unsigned short wc = ansi_z39_47_2uni_comb[dc-0xe0][c-0x20];
      if (wc != 0x0000) {
	*pwc = (ucs4_t) wc;
	conv->istate = 0;
	return 1;
      }
    }
    *pwc = (ucs4_t) c;
    return retval;
  }
  else if (c < 0xe0) {
    unsigned short wc = ansi_z39_47_2uni[c-0x80];
    if (wc != 0x0000) {
      *pwc = (ucs4_t) wc;
      return retval;
    }
  }
  else {
    /* The range from 0xe0 to 0xfe are diacritical marks.
       Note that in ANSEL they come *before* the base characters, in Unicode,
       they come *after*, so we have to buffer them ... */
    conv->istate = (state_t)c;
    return RET_TOOFEW(1);
  }
  return RET_ILSEQ;
}

static const unsigned char ansi_z39_47_page080[][2] = TABLE_FROM_UCS4_BASIC;
static const unsigned char ansi_z39_47_page01a[][2] = TABLE_FROM_UCS4_PAGE_01A;
static const unsigned char ansi_z39_47_page022[][2] = TABLE_FROM_UCS4_PAGE_022;
static const unsigned char ansi_z39_47_page02b[][2] = TABLE_FROM_UCS4_PAGE_02B;
static const unsigned char ansi_z39_47_page030[][2] = TABLE_FROM_UCS4_PAGE_030;
static const unsigned char ansi_z39_47_page1ea[][2] = TABLE_FROM_UCS4_PAGE_1EA;
static const unsigned char ansi_z39_47_page200[][2] = TABLE_FROM_UCS4_PAGE_200;
static const unsigned char ansi_z39_47_page211[][2] = TABLE_FROM_UCS4_PAGE_211;
static const unsigned char ansi_z39_47_page266[][2] = TABLE_FROM_UCS4_PAGE_266;
static const unsigned char ansi_z39_47_pagefe2[][2] = TABLE_FROM_UCS4_PAGE_FE2;

static int
ansi_z39_47_wctomb (conv_t conv, unsigned char *r, ucs4_t wc, int n)
{
  const unsigned char* ch = NULL;
  int output = 0;
  
#define OUTPUT(c)  ++output; if (n < output) return RET_TOOSMALL; *(r++) = (c);

  /* Since in UTF-8 diacritical marks come after the base character and in
     ANSEL before, we need to buffer possible base characters (0x20 to 0x7f)
     to put the diacritical mark before it if there is one following */
  if (wc < 0x0080) {
    if (conv->ostate) {
      OUTPUT(conv->ostate);
      conv->ostate = 0;
    }
    if (wc >= 0x0020) {
      conv->ostate = (state_t) wc;
    }
    else {
      OUTPUT(wc);
    }
    return output;
  }
  else if (wc >= 0x0080 && wc < 0x017f)
    ch = ansi_z39_47_page080[wc-0x0080];
  else if (wc >= 0x01a0 && wc < 0x01b4)
    ch = ansi_z39_47_page01a[wc-0x01a0];
  else if (wc >= 0x0220 && wc < 0x0234)
    ch = ansi_z39_47_page022[wc-0x0220];
  else if (wc >= 0x02b0 && wc < 0x02e2)
    ch = ansi_z39_47_page02b[wc-0x02b0];
  else if (wc >= 0x0300 && wc < 0x0337)
    ch = ansi_z39_47_page030[wc-0x0300];
  else if (wc >= 0x1ea0 && wc < 0x1efa)
    ch = ansi_z39_47_page1ea[wc-0x1ea0];
  else if (wc >= 0x2000 && wc < 0x200f)
    ch = ansi_z39_47_page200[wc-0x2000];
  else if (wc >= 0x2110 && wc < 0x211a)
    ch = ansi_z39_47_page211[wc-0x2110];
  else if (wc >= 0x2660 && wc < 0x2674)
    ch = ansi_z39_47_page266[wc-0x2660];
  else if (wc >= 0xfe20 && wc < 0xfe25)
    ch = ansi_z39_47_pagefe2[wc-0xfe20];
  if (ch && ch[0] != 0) {
    if (ch[1] == 0 && ch[0] >= 0xe0 && ch[0] <= 0xfe) {
      /* Diacritical mark following a base character, buffered in ostate */
      /* Output diacritical mark, then base character */
      if (conv->ostate) {
	OUTPUT(ch[0]);
	OUTPUT(conv->ostate);
	conv->ostate = 0;
      }
      else
	return RET_ILUNI;
    }
    else {
      if (conv->ostate) {
	OUTPUT(conv->ostate);
	conv->ostate = 0;
      }
      OUTPUT(ch[0]);
    }
    if (ch[1] != 0) {
      OUTPUT(ch[1]);
    }
    return output;
  }
  return RET_ILUNI;
}
