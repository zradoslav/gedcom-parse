/* $Id$ */
/* $Name$ */

#include "gedcom.h"

int main(int argc, char* argv[])
{
  MECHANISM mech = IMMED_FAIL;
  if (argc > 1) {
    int i;
    for (i=1; i<argc; i++) {
      if (!strncmp(argv[i], "-d", 2))
	gedcom_enable_debug();
      else if (!strncmp(argv[i], "-fi", 3))
	mech = IMMED_FAIL;
      else if (!strncmp(argv[i], "-fd", 3))
	mech = DEFER_FAIL;
      else if (!strncmp(argv[i], "-fn", 3))
	mech = IGNORE_ERRORS;
      else {
	printf ("Unrecognized option: %s\n", argv[i]);
	exit(1);
      }
    }
  }
  gedcom_set_error_handling(mech);
  if (gedcom_parse() == 0) {
    printf("Parse succeeded\n");
  }
  else {
    printf("Parse failed\n");
  }
}

int gedcom_error(char* s, ...)
{
  int res;
  va_list ap;

  va_start(ap, s);
  fprintf(stderr, "Error on line %d: ", line_no);
  res = vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  
  return res;
}
