/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

#include <string.h>
#include <iconv.h>
#include <search.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "gedcom_internal.h"
#include "encoding.h"

#define INTERNAL_ENCODING "UTF8"
#define ENCODING_CONF_FILE "gedcom.enc"
#define GCONV_SEARCH_PATH "GCONV_PATH"
#define MAXBUF 255

static iconv_t cd_to_internal = (iconv_t) -1;
static void *encoding_mapping = NULL;
static ENCODING the_enc = ONE_BYTE;

struct node {
  char *gedcom_name;
  char *iconv_name;
};

char* charwidth_string[] = { "1", "2_HILO", "2_LOHI" };

int node_compare(const void *node1, const void *node2)
{
  return strcmp(((const struct node *) node1)->gedcom_name,
		((const struct node *) node2)->gedcom_name);
}

void add_encoding(char *gedcom_n, char* charwidth, char *iconv_n)
{
  void **datum;
  struct node *nodeptr = (struct node *) malloc(sizeof *nodeptr);
  nodeptr->gedcom_name = (char *) malloc(strlen(gedcom_n)
					 + strlen(charwidth) + 3);
  nodeptr->iconv_name  = (char *) malloc(strlen(iconv_n) + 1);
  /* sprintf is safe here (malloc'ed before) */
  sprintf(nodeptr->gedcom_name, "%s(%s)", gedcom_n, charwidth);
  strcpy(nodeptr->iconv_name, iconv_n);
  datum = tsearch(nodeptr, &encoding_mapping, node_compare);
  if ((datum == NULL) || (*datum != nodeptr)) {
    gedcom_warning("Duplicate entry found for encoding '%s', ignoring",
		   gedcom_n);
  }
}

char* get_encoding(char* gedcom_n, ENCODING enc)
{
  void **datum;
  struct node search_node;
  char *buffer;
  buffer = (char*)malloc(strlen(gedcom_n) + strlen(charwidth_string[enc]) + 3);
  /* sprintf is safe here (malloc'ed before) */
  sprintf(buffer, "%s(%s)", gedcom_n, charwidth_string[enc]);
  search_node.gedcom_name = buffer;
  datum = tfind(&search_node, &encoding_mapping, node_compare);
  free(buffer);
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
    char charwidth[MAXBUF + 1];
    char iconv_n[MAXBUF + 1];
    char *gconv_path;

    /* Add gedcom data directory to gconv search path */
    gconv_path = getenv(GCONV_SEARCH_PATH);
    if (gconv_path == NULL || strstr(gconv_path, PKGDATADIR) == NULL) {
      char *new_gconv_path;
      if (gconv_path == NULL) {
	new_gconv_path = (char *)malloc(strlen(GCONV_SEARCH_PATH)
					+ strlen(PKGDATADIR)
					+ 2);
	sprintf(new_gconv_path, "%s=%s", GCONV_SEARCH_PATH, PKGDATADIR);
      }
      else {
	new_gconv_path = (char *)malloc(strlen(GCONV_SEARCH_PATH)
					+ strlen(gconv_path)
					+ strlen(PKGDATADIR)
					+ 3);
	sprintf(new_gconv_path, "%s=%s:%s",
		GCONV_SEARCH_PATH, gconv_path, PKGDATADIR);
      }
      if (putenv(new_gconv_path) != 0) {
	gedcom_warning("Failed updating environment variable %s",
		       GCONV_SEARCH_PATH);
      }
    }
    
    /* Open gedcom configuration file and read */
    in = fopen(ENCODING_CONF_FILE, "r");
    if (in == NULL) {
      char path[PATH_MAX];
      sprintf(path, "%s/%s", PKGDATADIR, ENCODING_CONF_FILE);
      in = fopen(path, "r");
    }
    if (in == NULL) {
      gedcom_warning("Could not open encoding configuration file '%s'",
		     ENCODING_CONF_FILE);
    }
    else {
      while (fgets(buffer, sizeof(buffer), in) != NULL) {
	if (buffer[strlen(buffer) - 1] != '\n') {
	  gedcom_error("Line too long in encoding configuration file '%s'",
		       ENCODING_CONF_FILE);
	  return;
	}
	else if ((buffer[0] != '#') && (strcmp(buffer, "\n") != 0)) {
	  if (sscanf(buffer, "%s %s %s", gedcom_n, charwidth, iconv_n) == 3) {
	    add_encoding(gedcom_n, charwidth, iconv_n);
	  }
	  else {
	    gedcom_error("Missing data in encoding configuration file '%s'",
			 ENCODING_CONF_FILE);
	    return;
	  }
	}
      }
      fclose(in);
    }
  }
}

void set_encoding_width(ENCODING enc)
{
  the_enc = enc;
}

static char conv_buf[MAXGEDCLINELEN * 2];
static size_t conv_buf_size;

int open_conv_to_internal(char* fromcode)
{
  char *encoding = get_encoding(fromcode, the_enc);
  if (cd_to_internal != (iconv_t) -1)
    iconv_close(cd_to_internal);
  if (encoding == NULL) {
    cd_to_internal = (iconv_t) -1;
  }
  else {
    memset(conv_buf, 0, sizeof(conv_buf));
    conv_buf_size = 0;
    cd_to_internal = iconv_open(INTERNAL_ENCODING, encoding);
    if (cd_to_internal == (iconv_t) -1) {
      gedcom_error("Error opening conversion context for encoding %s: %s",
		   encoding, strerror(errno));
    }
  }
  return (cd_to_internal != (iconv_t) -1);  
}

void close_conv_to_internal()
{
  iconv_close(cd_to_internal);
  cd_to_internal = (iconv_t) -1;
}

char* to_internal(char* str, size_t len,
		  char* output_buffer, size_t out_len)
{
  size_t outsize = out_len;
  char *wrptr = output_buffer;
  char *rdptr = conv_buf;
  /* set up input buffer (concatenate to what was left previous time) */
  /* can't use strcpy, because possible null bytes from unicode */
  memcpy(conv_buf + conv_buf_size, str, len);
  conv_buf_size += len;
  /* set up output buffer (empty it) */
  memset(output_buffer, 0, out_len);
  /* do the conversion */
  iconv(cd_to_internal, &rdptr, &conv_buf_size, &wrptr, &outsize);
  /* then shift what is left over to the head of the input buffer */
  memmove(conv_buf, rdptr, conv_buf_size);
  memset(conv_buf + conv_buf_size, 0, sizeof(conv_buf) - conv_buf_size);
  return output_buffer;
}
