/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

#ifndef __MULTILEX_H
#define __MULTILEX_H
#include <stdio.h>

int        gedcom_1byte_lex();
extern FILE *gedcom_1byte_in;

int        gedcom_hilo_lex();
extern FILE *gedcom_hilo_in;

int        gedcom_lohi_lex();
extern FILE *gedcom_lohi_in;
#endif /* __MULTILEX_H */
