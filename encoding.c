#include <string.h>
#include <iconv.h>
#include <search.h>
#include <stdio.h>
#include "gedcom.h"
#include "encoding.h"

#define INTERNAL_ENCODING "UTF8"
#define ENCODING_CONF_FILE "gedcom.enc"
#define MAXBUF 255

static iconv_t cd_to_internal = (iconv_t) -1;
static char int_buf[MAXGEDCLINELEN*2];
static void *encoding_mapping = NULL;

struct node {
  char *gedcom_name;
  char *iconv_name;
};

int node_compare(const void *node1, const void *node2)
{
  return strcmp(((const struct node *) node1)->gedcom_name,
		((const struct node *) node2)->gedcom_name);
}

void add_encoding(char *gedcom_n, char *iconv_n)
{
  void **datum;
  struct node *nodeptr = (struct node *) malloc(sizeof *nodeptr);
  nodeptr->gedcom_name = (char *) malloc(strlen(gedcom_n) + 1);
  nodeptr->iconv_name  = (char *) malloc(strlen(iconv_n) + 1);
  strcpy(nodeptr->gedcom_name, gedcom_n);
  strcpy(nodeptr->iconv_name, iconv_n);
  datum = tsearch(nodeptr, &encoding_mapping, node_compare);
  if ((datum == NULL) || (*datum != nodeptr)) {
    gedcom_warning("Duplicate entry found for encoding '%s', ignoring",
		   gedcom_n);
  }
}

char* get_encoding(char* gedcom_n)
{
  void **datum;
  struct node search_node;
  search_node.gedcom_name = gedcom_n;
  datum = tfind(&search_node, &encoding_mapping, node_compare);
  if (datum == NULL) {
    gedcom_error("No encoding found for '%s'", gedcom_n);
    return NULL;
  }
  else {
    return ((const struct node *) *datum)->iconv_name;
  }
}

void init_encodings()
{
  if (encoding_mapping == NULL) {
    FILE *in;
    char buffer[MAXBUF + 1];
    char gedcom_n[MAXBUF + 1];
    char iconv_n[MAXBUF + 1];
    in = fopen(ENCODING_CONF_FILE, "r");
    if (in != NULL) {
      while (fgets(buffer, sizeof(buffer), in) != NULL) {
	if (buffer[strlen(buffer) - 1] != '\n') {
	  gedcom_error("Line too long in encoding configuration file '%s'",
		       ENCODING_CONF_FILE);
	  return;
	}
	else if (buffer[0] != '#') {
	  if (sscanf(buffer, "%s %s", gedcom_n, iconv_n) == 2) {
	    add_encoding(gedcom_n, iconv_n);
	  }
	}
      }
      fclose(in);
    }
    else {
      gedcom_warning("Could not open encoding configuration file '%s'",
		     ENCODING_CONF_FILE);
    }
  }
}

int open_conv_to_internal(char* fromcode)
{
  char *encoding = get_encoding(fromcode);
  if (cd_to_internal != (iconv_t) -1)
    iconv_close(cd_to_internal);
  if (encoding == NULL) {
    cd_to_internal = (iconv_t) -1;
  }
  else {
    cd_to_internal = iconv_open(INTERNAL_ENCODING, encoding);
  }
  return (cd_to_internal != (iconv_t) -1);  
}

void close_conv_to_internal()
{
  iconv_close(cd_to_internal);
  cd_to_internal = (iconv_t) -1;
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

