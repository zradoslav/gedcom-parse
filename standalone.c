/* $Id$ */
/* $Name$ */

#include "gedcom.h"

void show_help ()
{
  printf("gedcom-parse test program for libgedcom\n");
  printf("Options:\n");
  printf("  -h    Show this help text\n");
  printf("  -nc   Disable compatibility mode\n");
  printf("  -fi   Fail immediately on errors\n");
  printf("  -fd   Deferred fail on errors, but parse completely\n");
  printf("  -fn   No fail on errors\n");
  printf("  -dg   Debug setting: only libgedcom debug messages\n");
  printf("  -da   Debug setting: libgedcom + yacc debug messages\n");
}

int main(int argc, char* argv[])
{
  MECHANISM mech = IMMED_FAIL;
  int compat_enabled = 1;
  int debug_level = 0;
  if (argc > 1) {
    int i;
    for (i=1; i<argc; i++) {
      if (!strncmp(argv[i], "-da", 4))
	debug_level = 2;
      else if (!strncmp(argv[i], "-dg", 4))
	debug_level = 1;
      else if (!strncmp(argv[i], "-fi", 4))
	mech = IMMED_FAIL;
      else if (!strncmp(argv[i], "-fd", 4))
	mech = DEFER_FAIL;
      else if (!strncmp(argv[i], "-fn", 4))
	mech = IGNORE_ERRORS;
      else if (!strncmp(argv[i], "-nc", 4))
	compat_enabled = 0;
      else if (!strncmp(argv[i], "-h", 3)) {
	show_help();
	exit(1);
      }
      else {
	printf ("Unrecognized option: %s\n", argv[i]);
	show_help();
	exit(1);
      }
    }
  }
  gedcom_set_debug_level(debug_level);
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
