#include <string.h>
#include <iconv.h>
#include "gedcom.h"
#include "encoding.h"

#define INTERNAL_ENCODING "UTF8"

static iconv_t cd_to_internal = (iconv_t) -1;
static char int_buf[MAXGEDCLINELEN*2];

int open_conv_to_internal(char* fromcode)
{
  if (cd_to_internal != (iconv_t) -1)
    iconv_close(cd_to_internal);
  cd_to_internal = iconv_open(INTERNAL_ENCODING, fromcode);
  return (cd_to_internal != (iconv_t) -1);  
}

void close_conv_to_internal()
{
  iconv_close(cd_to_internal);
}

char* to_internal(char* str, size_t len)
{
  size_t insize = len;
  size_t outsize = MAXGEDCLINELEN * 2;
  char *wrptr = int_buf;
  char *rdptr = str;
  memset(int_buf, 0, sizeof(int_buf));
  iconv(cd_to_internal, &rdptr, &insize, &wrptr, &outsize);
  return int_buf;
}

