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

static const uint32_t to_ucs4[256] =
{
  /* 0x00 */ 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
  /* 0x08 */ 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
  /* 0x10 */ 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
  /* 0x18 */ 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
  /* 0x20 */ 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
  /* 0x28 */ 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
  /* 0x30 */ 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
  /* 0x38 */ 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
  /* 0x40 */ 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
  /* 0x48 */ 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
  /* 0x50 */ 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
  /* 0x58 */ 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
  /* 0x60 */ 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
  /* 0x68 */ 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
  /* 0x70 */ 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
  /* 0x78 */ 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
  
  /* 0x80 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 0x88 */ 0x0088, 0x0089, 0x0000, 0x0000, 0x0000, 0x200d, 0x200c, 0x0000,
  /* 0x90 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 0x98 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 0xa0 */ 0x0000, 0x0141, 0x00d8, 0x0110, 0x00de, 0x00c6, 0x0152, 0x02b9,
  /* 0xa8 */ 0x00b7, 0x266d, 0x00ae, 0x00b1, 0x01a0, 0x01af, 0x02be, 0x0000,
  /* 0xb0 */ 0x02bb, 0x0142, 0x00f8, 0x0111, 0x00fe, 0x00e6, 0x0153, 0x02ba,
  /* 0xb8 */ 0x0131, 0x00a3, 0x00f0, 0x0000, 0x01a1, 0x01b0, 0x0000, 0x0000,
  /* 0xc0 */ 0x00b0, 0x2113, 0x2117, 0x00a9, 0x266f, 0x00bf, 0x00a1, 0x0000,
  /* 0xc8 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00df,
  /* 0xd0 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 0xd8 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 0xe0 */ 0x0309, 0x0300, 0x0301, 0x0302, 0x0303, 0x0304, 0x0306, 0x0307,
  /* 0xe8 */ 0x0308, 0x030c, 0x030a, 0xfe20, 0xfe21, 0x0315, 0x030b, 0x0310,
  /* 0xf0 */ 0x0327, 0x0328, 0x0323, 0x0324, 0x0325, 0x0333, 0x0332, 0x0326,
  /* 0xf8 */ 0x031c, 0x032e, 0xfe22, 0xfe23, 0x0000, 0x0000, 0x0313, 0x0000
};

/* The outer array range runs from 0xe0 to 0xfe, the inner range from 0x20
   to 0x7f.  */
static const uint32_t to_ucs4_comb[31][96] =
{
  /* 0xe0 (hook above) */
  {
    /* 0x20 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x1ea2, 0x0000, 0x0000, 0x0000, 0x1eba, 0x0000, 0x0000,
    /* 0x48 */ 0x0000, 0x1ec8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1ece,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1ee6, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x1ef6, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x1ea3, 0x0000, 0x0000, 0x0000, 0x1ebb, 0x0000, 0x0000,
    /* 0x68 */ 0x0000, 0x1ec9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1ecf,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1ee7, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x1ef7, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xe1 (grave) */
  {
    /* 0x20 */ 0x0060, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x00c0, 0x0000, 0x0000, 0x0000, 0x00c8, 0x0000, 0x0000,
    /* 0x48 */ 0x0000, 0x00cc, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00d2,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00d9, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x00e0, 0x0000, 0x0000, 0x0000, 0x00e8, 0x0000, 0x0000,
    /* 0x68 */ 0x0000, 0x00ec, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00f2,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00f9, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xe2 (acute) */
  {
    /* 0x20 */ 0x00b4, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x00c1, 0x0000, 0x0106, 0x0000, 0x00c9, 0x0000, 0x0000,
    /* 0x48 */ 0x0000, 0x00cd, 0x0000, 0x0000, 0x0139, 0x0000, 0x0143, 0x00d3,
    /* 0x50 */ 0x0000, 0x0000, 0x0154, 0x015a, 0x0000, 0x00da, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x00dd, 0x0179, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x00e1, 0x0000, 0x0107, 0x0000, 0x00e9, 0x0000, 0x0000,
    /* 0x68 */ 0x0000, 0x00ed, 0x0000, 0x0000, 0x013a, 0x0000, 0x0144, 0x00f3,
    /* 0x70 */ 0x0000, 0x0000, 0x0155, 0x015b, 0x0000, 0x00fa, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x00fd, 0x017a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xe3 (circumflex) */
  {
    /* 0x20 */ 0x005e, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x00c2, 0x0000, 0x0108, 0x0000, 0x00ca, 0x0000, 0x011c,
    /* 0x48 */ 0x0124, 0x00ce, 0x0134, 0x0000, 0x0000, 0x0000, 0x0000, 0x00d4,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x015c, 0x0000, 0x00db, 0x0000, 0x0174,
    /* 0x58 */ 0x0000, 0x0176, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x00e2, 0x0000, 0x0109, 0x0000, 0x00ea, 0x0000, 0x011d,
    /* 0x68 */ 0x0125, 0x00ee, 0x0135, 0x0000, 0x0000, 0x0000, 0x0000, 0x00f4,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x015d, 0x0000, 0x00fb, 0x0000, 0x0175,
    /* 0x78 */ 0x0000, 0x0177, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xe4 (tilde) */
  {
    /* 0x20 */ 0x007e, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x00c3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x48 */ 0x0000, 0x0128, 0x0000, 0x0000, 0x0000, 0x0000, 0x00d1, 0x00d5,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0168, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x00e3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x68 */ 0x0000, 0x0129, 0x0000, 0x0000, 0x0000, 0x0000, 0x00f1, 0x00f5,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0169, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xe5 (macron) */
  {
    /* 0x20 */ 0x00af, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x0100, 0x0000, 0x0000, 0x0000, 0x0112, 0x0000, 0x0000,
    /* 0x48 */ 0x0000, 0x012a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x014c,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x016a, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x0101, 0x0000, 0x0000, 0x0000, 0x0113, 0x0000, 0x0000,
    /* 0x68 */ 0x0000, 0x012b, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x014d,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x016b, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xe6 (breve) */
  {
    /* 0x20 */ 0x02d8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x0102, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x011e,
    /* 0x48 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x016c, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x0103, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x011f,
    /* 0x68 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x016d, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xe7 (dot above) */
  {
    /* 0x20 */ 0x02d9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x0226, 0x0000, 0x010a, 0x0000, 0x0116, 0x0000, 0x0120,
    /* 0x48 */ 0x0000, 0x0130, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x022e,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0000, 0x017b, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x0227, 0x0000, 0x010b, 0x0000, 0x0117, 0x0000, 0x0121,
    /* 0x68 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x022f,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x0000, 0x017c, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xe8 (umlaut, diaeresis) */
  {
    /* 0x20 */ 0x00a8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x00c4, 0x0000, 0x0000, 0x0000, 0x00cb, 0x0000, 0x0000,
    /* 0x48 */ 0x0000, 0x00cf, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00d6,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00dc, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0178, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x00e4, 0x0000, 0x0000, 0x0000, 0x00eb, 0x0000, 0x0000,
    /* 0x68 */ 0x0000, 0x00ef, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00f6,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00fc, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x00ff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xe9 (caron, hacek) */
  {
    /* 0x20 */ 0x02c7, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x0000, 0x0000, 0x010c, 0x010e, 0x011a, 0x0000, 0x0000,
    /* 0x48 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x013d, 0x0000, 0x0147, 0x0000,
    /* 0x50 */ 0x0000, 0x0000, 0x0158, 0x0160, 0x0164, 0x0000, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0000, 0x017d, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x0000, 0x0000, 0x010d, 0x010f, 0x011b, 0x0000, 0x0000,
    /* 0x68 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x013e, 0x0000, 0x0148, 0x0000,
    /* 0x70 */ 0x0000, 0x0000, 0x0159, 0x0161, 0x0165, 0x0000, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x0000, 0x017e, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xea (ring above) */
  {
    /* 0x20 */ 0x02da, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x00c5, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x48 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x016e, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x00e5, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x68 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x016f, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xeb (ligature, left half) */
  {
    0x0000,
  },
  /* 0xec (ligature, right half) */
  {
    0x0000,
  },
  /* 0xed (comma above right) */
  {
    0x0000,
  },
  /* 0xee (double acute) */
  {
    /* 0x20 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x48 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0150,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0170, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x68 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0151,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0171, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xef (candrabindu) */      
  {
    0x0000,
  },
  /* 0xf0 (cedilla) */
  {
    /* 0x20 */ 0x00b8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x0000, 0x0000, 0x00c7, 0x0000, 0x0000, 0x0000, 0x0122,
    /* 0x48 */ 0x0000, 0x0000, 0x0000, 0x0136, 0x013b, 0x0000, 0x0145, 0x0000,
    /* 0x50 */ 0x0000, 0x0000, 0x0156, 0x015e, 0x0162, 0x0000, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x0000, 0x0000, 0x00e7, 0x0000, 0x0000, 0x0000, 0x0123,
    /* 0x68 */ 0x0000, 0x0000, 0x0000, 0x0137, 0x013c, 0x0000, 0x0146, 0x0000,
    /* 0x70 */ 0x0000, 0x0000, 0x0157, 0x015f, 0x0163, 0x0000, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xf1 (ogonek, right hook) */
  {
    /* 0x20 */ 0x02db, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x28 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x30 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x38 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x40 */ 0x0000, 0x0104, 0x0000, 0x0000, 0x0000, 0x0118, 0x0000, 0x0000,
    /* 0x48 */ 0x0000, 0x012e, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x50 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0172, 0x0000, 0x0000,
    /* 0x58 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x60 */ 0x0000, 0x0105, 0x0000, 0x0000, 0x0000, 0x0119, 0x0000, 0x0000,
    /* 0x68 */ 0x0000, 0x012f, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0x70 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0173, 0x0000, 0x0000,
    /* 0x78 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  },
  /* 0xf2 (dot below) */
  {
    0x0000,
  },
  /* 0xf3 (double dot below) */
  {
    0x0000,
  },
  /* 0xf4 (ring below) */
  {
    0x0000,
  },
  /* 0xf5 (double low line) */
  {
    0x0000,
  },
  /* 0xf6 (line below) */
  {
    0x0000,
  },
  /* 0xf7 (comma below, left hook) */
  {
    0x0000,
  },
  /* 0xf8 (left half ring below, right cedilla) */
  {
    0x0000,
  },
  /* 0xf9 (breve below, half circle below) */
  {
    0x0000,
  },
  /* 0xfa (double tilde, left half) */
  {
    0x0000,
  },
  /* 0xfb (double tilde, right half) */
  {
    0x0000,
  },
  /* 0xfc */
  {
    0x0000,
  },
  /* 0xfd */
  {
    0x0000,
  },
  /* 0xfe (comma above, high centered comma) */
  {
    0x0000,
  },
};


static const char from_ucs4[][2] =
{
  /* 0x0000 */ "\x00\x00", "\x01\x00", "\x02\x00", "\x03\x00", "\x04\x00",
  /* 0x0005 */ "\x05\x00", "\x06\x00", "\x07\x00", "\x08\x00", "\x09\x00",
  /* 0x000a */ "\x0a\x00", "\x0b\x00", "\x0c\x00", "\x0d\x00", "\x0e\x00",
  /* 0x000f */ "\x0f\x00", "\x10\x00", "\x11\x00", "\x12\x00", "\x13\x00",
  /* 0x0014 */ "\x14\x00", "\x15\x00", "\x16\x00", "\x17\x00", "\x18\x00",
  /* 0x0019 */ "\x19\x00", "\x1a\x00", "\x1b\x00", "\x1c\x00", "\x1d\x00",
  /* 0x001e */ "\x1e\x00", "\x1f\x00", "\x20\x00", "\x21\x00", "\x22\x00",
  /* 0x0023 */ "\xa6\x00", "\xa4\x00", "\x25\x00", "\x26\x00", "\x27\x00",
  /* 0x0028 */ "\x28\x00", "\x29\x00", "\x2a\x00", "\x2b\x00", "\x2c\x00",
  /* 0x002d */ "\x2d\x00", "\x2e\x00", "\x2f\x00", "\x30\x00", "\x31\x00",
  /* 0x0032 */ "\x32\x00", "\x33\x00", "\x34\x00", "\x35\x00", "\x36\x00",
  /* 0x0037 */ "\x37\x00", "\x38\x00", "\x39\x00", "\x3a\x00", "\x3b\x00",
  /* 0x003c */ "\x3c\x00", "\x3d\x00", "\x3e\x00", "\x3f\x00", "\x40\x00",
  /* 0x0041 */ "\x41\x00", "\x42\x00", "\x43\x00", "\x44\x00", "\x45\x00",
  /* 0x0046 */ "\x46\x00", "\x47\x00", "\x48\x00", "\x49\x00", "\x4a\x00",
  /* 0x004b */ "\x4b\x00", "\x4c\x00", "\x4d\x00", "\x4e\x00", "\x4f\x00",
  /* 0x0050 */ "\x50\x00", "\x51\x00", "\x52\x00", "\x53\x00", "\x54\x00",
  /* 0x0055 */ "\x55\x00", "\x56\x00", "\x57\x00", "\x58\x00", "\x59\x00",
  /* 0x005a */ "\x5a\x00", "\x5b\x00", "\x5c\x00", "\x5d\x00", "\x5e\x00",
  /* 0x005f */ "\x5f\x00", "\x60\x00", "\x61\x00", "\x62\x00", "\x63\x00",
  /* 0x0064 */ "\x64\x00", "\x65\x00", "\x66\x00", "\x67\x00", "\x68\x00",
  /* 0x0069 */ "\x69\x00", "\x6a\x00", "\x6b\x00", "\x6c\x00", "\x6d\x00",
  /* 0x006e */ "\x6e\x00", "\x6f\x00", "\x70\x00", "\x71\x00", "\x72\x00",
  /* 0x0073 */ "\x73\x00", "\x74\x00", "\x75\x00", "\x76\x00", "\x77\x00",
  /* 0x0078 */ "\x78\x00", "\x79\x00", "\x7a\x00", "\x7b\x00", "\x7c\x00",
  /* 0x007d */ "\x7d\x00", "\x7e\x00", "\x7f\x00", "\x00\x00", "\x00\x00",
  /* 0x0082 */ "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00",
  /* 0x0087 */ "\x00\x00", "\x88\x00", "\x89\x00", "\x00\x00", "\x00\x00",
  /* 0x008c */ "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00",
  /* 0x0091 */ "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00",
  /* 0x0096 */ "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00",
  /* 0x009b */ "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00",
  /* 0x00a0 */ "\x00\x00", "\xc6\x00", "\x00\x00", "\xb9\x00", "\x00\x00",
  /* 0x00a5 */ "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x20", "\xc3\x00",
  /* 0x00aa */ "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00", "\xaa\x00",
  /* 0x00af */ "\x00\x20", "\xc0\x00", "\xab\x00", "\x00\x00", "\x00\x00",
  /* 0x00b4 */ "\x00\x20", "\x00\x00", "\x00\x00", "\xa8\x00", "\x00\x00",
  /* 0x00b9 */ "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00", "\x00\x00",
  /* 0x00be */ "\x00\x00", "\xc5\x00", "\xe1\x41", "\xe2\x41", "\xe3\x41",
  /* 0x00c3 */ "\xe4\x41", "\xe8\x41", "\xea\x41", "\xa5\x00", "\xf0\x43",
  /* 0x00c8 */ "\xe1\x45", "\xe2\x45", "\xe3\x45", "\xe8\x45", "\xe1\x49",
  /* 0x00cd */ "\xe2\x49", "\xe3\x49", "\xe8\x49", "\xa3\x00", "\xe4\x4e",
  /* 0x00d2 */ "\xe1\x4f", "\xe2\x4f", "\xce\x4f", "\xe4\x4f", "\xe8\x4f",
  /* 0x00d7 */ "\x00\x00", "\xa2\x00", "\xe1\x55", "\xe2\x55", "\xe3\x55",
  /* 0x00dc */ "\xe8\x55", "\xe2\x59", "\xa4\x00", "\xc8\x00", "\xe1\x61",
  /* 0x00e1 */ "\xe2\x61", "\xe3\x61", "\xe4\x61", "\xe8\x61", "\xea\x61",
  /* 0x00e6 */ "\xb5\x00", "\xf0\x63", "\xe1\x65", "\xe2\x65", "\xe3\x65",
  /* 0x00eb */ "\xe8\x65", "\xe1\x69", "\xe2\x69", "\xe3\x69", "\xe8\x69",
  /* 0x00f0 */ "\xba\x00", "\xe4\x6e", "\xe1\x6f", "\xe2\x6f", "\xe3\x6f",
  /* 0x00f5 */ "\xe4\x6f", "\xe8\x6f", "\x00\x00", "\xb2\x00", "\xe1\x75",
  /* 0x00fa */ "\xe2\x75", "\xe3\x75", "\xe8\x75", "\xe2\x79", "\xb4\x00",
  /* 0x00ff */ "\xe8\x79", "\xe5\x41", "\xe5\x61", "\xe6\x41", "\xe6\x61",
  /* 0x0104 */ "\xe1\x41", "\xe1\x61", "\xe2\x43", "\xe2\x63", "\xe3\x43",
  /* 0x0109 */ "\xe3\x63", "\xe7\x43", "\xe7\x63", "\xe9\x43", "\xe9\x63",
  /* 0x010e */ "\xe9\x44", "\xe9\x64", "\xa3\x00", "\xb3\x00", "\xe5\x45",
  /* 0x0113 */ "\xe5\x65", "\xe6\x65", "\xe6\x65", "\xe7\x45", "\xe7\x65",
  /* 0x0118 */ "\xf1\x45", "\xf1\x65", "\xe9\x45", "\xe9\x65", "\xe3\x47",
  /* 0x011d */ "\xe3\x67", "\xe6\x47", "\xe6\x67", "\xe7\x47", "\xe7\x67",
  /* 0x0122 */ "\xf0\x47", "\xf0\x67", "\xe3\x48", "\x00\x00", "\x00\x00",
  /* 0x0127 */ "\xe5\x68", "\xe4\x49", "\xe4\x69", "\xe5\x49", "\xe5\x69",
  /* 0x012c */ "\xe6\x49", "\xe6\x69", "\xf1\x49", "\xf1\x69", "\xe7\x49",
  /* 0x0131 */ "\xb8\x00", "\x00\x00", "\x00\x00", "\xe3\x4a", "\xe3\x6a",
  /* 0x0136 */ "\xf0\x4b", "\xf0\x6b", "\x00\x00", "\xe2\x4c", "\xe2\x6c",
  /* 0x013b */ "\xf0\x4c", "\xf0\x6c", "\xe9\x4c", "\xe9\x6c", "\xe7\x4c",
  /* 0x0140 */ "\xe7\x6c", "\xa1\x00", "\xb1\x00", "\xe2\x4e", "\xe2\x6e",
  /* 0x0145 */ "\xf0\x4e", "\xf0\x6e", "\xe9\x4e", "\xe9\x6e", "\x00\x00",
  /* 0x014a */ "\x00\x00", "\x00\x00", "\xe5\x4f", "\xe5\x6f", "\xe6\x4f",
  /* 0x014f */ "\xe6\x6f", "\xee\x4f", "\xee\x6f", "\xa6\x00", "\xb6\x00",
  /* 0x0154 */ "\xe2\x52", "\xe2\x72", "\xf0\x52", "\xf0\x72", "\xe9\x52",
  /* 0x0159 */ "\xe9\x72", "\xe2\x53", "\xe2\x73", "\xe3\x53", "\xe3\x73",
  /* 0x015e */ "\xf0\x53", "\xf0\x73", "\xe9\x53", "\xe9\x73", "\xf0\x54",
  /* 0x0163 */ "\xf0\x74", "\xe9\x54", "\xe9\x74", "\x00\x00", "\x00\x00",
  /* 0x0168 */ "\xe4\x55", "\xe4\x75", "\xe5\x55", "\xe5\x75", "\xe6\x55",
  /* 0x016d */ "\xe6\x75", "\xea\x55", "\xea\x75", "\xee\x55", "\xee\x75",
  /* 0x0172 */ "\xf1\x55", "\xf1\x75", "\xe3\x57", "\xe3\x77", "\xe3\x59",
  /* 0x0177 */ "\xe3\x79", "\xe8\x59", "\xe2\x5a", "\xe2\x7a", "\xe7\x5a",
  /* 0x017c */ "\xe7\x7a", "\xe9\x5a", "\xe9\x7a"
/*
   This table does not cover the following positions:

     0x01a0    "\xac\x00", "\xbc\x00"
     ...
     0x01af    "\xad\x00", "\xbd\x00"
     ...
     0x0226    "\xe7\x41", "\xe7\x61"
     ...
     0x022e    "\xe7\x4f", "\xe7\x6f"
     ...
     0x02ba    "\xb7\x00"
     ...
     0x02be    "\xae\x00", "\xb0\x00"
     ...
     0x02c7    "\xe9\x20",
     ...
     0x02d8    "\xe6\x20", "\xe7\x20", "\xea\x20", "\xf1\x20", "\xe4\x20",
     0x02dd    "\xee\x20",
     ...
     0x200C    "\x8e\x00", "\x8d\x00"
     ...
     0x2113    "\xc1\x00"
     ...
     0x2117    "\xc2\x00"
     ...
     0x266d    "\xa9\x00", "\x00\x00", "\xc4\x00"
     ...
     0xfe20    "\xeb\x00", "\xec\x00", "\xfa\x00", "\xfb\x00"

   These would blow up the table and are therefore handled specially in
   the code.
*/
};


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
              ch2 = to_ucs4[ch2];                                             \
              put32 (outptr, ch2);                                            \
              outptr += 4;                                                    \
              ch = to_ucs4[ch];                                               \
	      incr = 2;                                                       \
	    }                                                                 \
	  }								      \
      }									      \
    else								      \
      {									      \
	ch = to_ucs4[ch];						      \
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
    if (__builtin_expect (ch >= sizeof (from_ucs4) / sizeof (from_ucs4[0]),   \
	0))								      \
      {									      \
	if (ch >= 0x1a0 && ch <= 0x1a1)		       			      \
          {                                                                   \
	    static const char map[2] = "\xac\xbc";                            \
	    tmp[0] = map[ch - 0x1a0];                                         \
	    tmp[1] = '\0';                                                    \
	    cp = tmp;                                                         \
	  }                                                                   \
	else if (ch >= 0x1af && ch <= 0x1b0)   	       			      \
          {                                                                   \
	    static const char map[2] = "\xad\xbd";                            \
	    tmp[0] = map[ch - 0x1af];                                         \
	    tmp[1] = '\0';                                                    \
	    cp = tmp;                                                         \
	  }                                                                   \
	else if (ch >= 0x226 && ch <= 0x227)   	       			      \
          {                                                                   \
	    static const char map[2] = "\x41\x61";                            \
	    tmp[0] = '\xe7';                                                  \
	    tmp[1] = map[ch - 0x226];                                         \
	    cp = tmp;                                                         \
	  }                                                                   \
	else if (ch >= 0x22e && ch <= 0x22f)   	       			      \
          {                                                                   \
	    static const char map[2] = "\x4f\x6f";                            \
	    tmp[0] = '\xe7';                                                  \
	    tmp[1] = map[ch - 0x22e];                                         \
	    cp = tmp;                                                         \
	  }                                                                   \
	else if (ch = 0x2ba)            	       			      \
            cp = "\xb7";                                                      \
	else if (ch >= 0x2be && ch <= 0x2bf)   	       			      \
          {                                                                   \
	    static const char map[2] = "\xae\xb0";                            \
	    tmp[0] = map[ch - 0x2be];                                         \
	    tmp[1] = '\0';                                                    \
	    cp = tmp;                                                         \
	  }                                                                   \
	else if (ch = 0x2c7)            	       			      \
            cp = "\xe9 ";                                                     \
	else if (ch >= 0x2d8 && ch <= 0x2dd && ch != 0x2dc)		      \
	  {								      \
	    static const char map[6] = "\xe6\xe7\xea\xf1\xe4\xee";	      \
									      \
	    tmp[0] = map[ch - 0x2d8];					      \
	    tmp[1] = ' ';						      \
	    cp = tmp;							      \
	  }								      \
	else if (ch = 0x200c)            	       			      \
            cp = "\x8e";                                                      \
	else if (ch = 0x200d)            	       			      \
            cp = "\x8d";                                                      \
	else if (ch = 0x2113)            	       			      \
            cp = "\xc1";                                                      \
	else if (ch = 0x2117)            	       			      \
            cp = "\xc2";                                                      \
	else if (ch = 0x266d)            	       			      \
            cp = "\xa9";                                                      \
	else if (ch = 0x266f)            	       			      \
            cp = "\xc4";                                                      \
	else if (ch >= 0xfe20 && ch <= 0xfe23)				      \
	  {								      \
	    static const char map[4] = "\xeb\xec\xfa\xfb";		      \
									      \
	    tmp[0] = map[ch - 0xfe20];					      \
	    tmp[1] = '\0';						      \
	    cp = tmp;							      \
	  }								      \
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
	cp = from_ucs4[ch];						      \
									      \
	if (__builtin_expect (cp[0], '\1') == '\0' && ch != 0)		      \
	  {								      \
	    /* Illegal characters.  */					      \
	    STANDARD_ERR_HANDLER (4);					      \
	  }								      \
      }									      \
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
