/* $Id$ */
/* $Name$ */

#include "gedcom.h"

int main(int argc, char* argv[])
{
  MECHANISM mech = IMMED_FAIL;
  int compat_enabled = 1;
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
      else if (!strncmp(argv[i], "-nc", 3))
	compat_enabled = 0;
      else {
	printf ("Unrecognized option: %s\n", argv[i]);
	exit(1);
      }
    }
  }
  gedcom_set_compat_handling(compat_enabled);
  gedcom_set_error_handling(mech);
  if (gedcom_parse() == 0) {
    printf("Parse succeeded\n");
    return 0;
  }
  else {
    printf("Parse failed\n");
    return 1;
  }
}

int gedcom_debug_print(char* s, ...)
{
  int res;
#if YYDEBUG != 0
  if (gedcom_debug) {
    va_list ap;
    va_start(ap, s);
    res = vfprintf(stderr, s, ap);
    va_end(ap);
  }
#endif
  return(res);
}

int gedcom_warning(char* s, ...)
{
  int res;
  va_list ap;

  va_start(ap, s);
  fprintf(stderr, "Warning on line %d: ", line_no);
  res = vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  
  return res;
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
