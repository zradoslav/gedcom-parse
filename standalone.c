/* $Id$ */
/* $Name$ */

#include "gedcom.h"

void show_help ()
{
  printf("gedcom-parse test program for libgedcom\n\n");
  printf("Usage:  gedcom-parse [options] file\n");
  printf("Options:\n");
  printf("  -h    Show this help text\n");
  printf("  -nc   Disable compatibility mode\n");
  printf("  -fi   Fail immediately on errors\n");
  printf("  -fd   Deferred fail on errors, but parse completely\n");
  printf("  -fn   No fail on errors\n");
  printf("  -dg   Debug setting: only libgedcom debug messages\n");
  printf("  -da   Debug setting: libgedcom + yacc debug messages\n");
}

int determine_encoding(FILE* f)
{
  char first[2];

  fread(first, 1, 2, f);
  if ((first[0] == '0') && (first[1] == ' ')) {
    gedcom_warning("One-byte encoding");
    fseek(f, 0, 0);
    return ONE_BYTE;
  }
  else if ((first[0] == '\0') && (first[1] == '0'))
  {
    gedcom_warning("Two-byte encoding, high-low");
    fseek(f, 0, 0);
    return TWO_BYTE_HILO;
  }
  else if ((first[0] == '\xFE') && (first[1] == '\xFF'))
  {
    gedcom_warning("Two-byte encoding, high-low, with BOM");
    return TWO_BYTE_HILO;
  }
  else if ((first[0] == '0') && (first[1] == '\0'))
  {
    gedcom_warning("Two-byte encoding, low-high");
    fseek(f, 0, 0);
    return TWO_BYTE_LOHI;
  }
  else if ((first[0] == '\xFF') && (first[1] == '\xFE'))
  {
    gedcom_warning("Two-byte encoding, low-high, with BOM");
    return TWO_BYTE_LOHI;
  }
  else {
    gedcom_warning("Unknown encoding, falling back to one-byte");
    fseek(f, 0, 0);
    return ONE_BYTE;
  }
}

int gedcom_xxx_parse(char* file_name)
{
  ENCODING enc;
  FILE* file = fopen (file_name, "r");
  if (!file) {
    printf("Could not open file '%s'\n", file_name);
    exit(1);
  }
  enc = determine_encoding(file);

  if (enc == ONE_BYTE) {
    gedcom_in = file;
    return gedcom_parse();
  }
  else {
    printf("No parser yet for encoding\n");
    exit(1);
  }
}

int main(int argc, char* argv[])
{
  MECHANISM mech = IMMED_FAIL;
  int compat_enabled = 1;
  int debug_level = 0;
  char* file_name = NULL;

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
      else if (strncmp(argv[i], "-", 1)) {
	file_name = argv[i];
	break;
      }
      else {
	printf ("Unrecognized option: %s\n", argv[i]);
	show_help();
	exit(1);
      }
    }
  }
  
  if (!file_name) {
    printf("No file name given\n");
    show_help();
    exit(1);
  }

  gedcom_set_debug_level(debug_level);
  gedcom_set_compat_handling(compat_enabled);
  gedcom_set_error_handling(mech);
  
  if (gedcom_xxx_parse(file_name) == 0) {
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
