/* Encoding utility from UTF-8 to locale and vice versa
   Copyright (C) 2001, 2002 Peter Verthez

   Permission granted to do anything with this file that you want, as long
   as the above copyright is retained in all copies.
   THERE IS NO WARRANTY - USE AT YOUR OWN RISK
*/

/* $Id$ */
/* $Name$ */

#ifndef __UTF8_LOCALE_H
#define __UTF8_LOCALE_H

void  convert_set_unknown(const char* unknown);
char* convert_utf8_to_locale(char* input);
char* convert_locale_to_utf8(char* input);

#endif /* __UTF8_LOCALE_H */
