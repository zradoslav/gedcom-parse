/* $Id$ */
/* $Name$ */

#include "gedcom.h"

int main(int argc, char* argv[])
{
  if ((argc > 1) && !strncmp(argv[1], "-d", 2))
    gedcom_enable_debug();
  gedcom_set_error_handling(IGNORE_RECORD);
  gedcom_parse();
}

int gedcom_error(char* s, ...)
{
  int res;
  va_list ap;

  va_start(ap, s);
  fprintf(stderr, "Line %d: ", line_no);
  res = vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  
  return res;
}
