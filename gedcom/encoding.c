/* Conversion between encodings.
   Copyright (C) 2001,2002 The Genes Development Team
   This file is part of the Gedcom parser library.
   Contributed by Peter Verthez <Peter.Verthez@advalvas.be>, 2001.

   The Gedcom parser library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The Gedcom parser library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Gedcom parser library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* $Id$ */
/* $Name$ */

#include <string.h>
#include <iconv.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "gedcom_internal.h"
#include "encoding.h"
#include "hash.h"

#define ENCODING_CONF_FILE "gedcom.enc"
#define GCONV_SEARCH_PATH "GCONV_PATH"
#define MAXBUF 255

static iconv_t cd_to_internal = (iconv_t) -1;
static ENCODING the_enc = ONE_BYTE;
static hash_t *encodings = NULL;

char* charwidth_string[] = { "1", "2_HILO", "2_LOHI" };

hnode_t *node_alloc(void *c __attribute__((unused)))
{
  return malloc(sizeof *node_alloc(NULL));
}

void node_free(hnode_t *n, void *c __attribute__((unused)))
{
  free((void*)hnode_getkey(n));
  free(hnode_get(n));
  free(n);
}

void add_encoding(char *gedcom_n, char* charwidth, char *iconv_n)
{
  char *key, *val;

  key = (char *) malloc(strlen(gedcom_n) + strlen(charwidth) + 3);
  val = (char *) malloc(strlen(iconv_n) + 1);

  /* sprintf is safe here (malloc'ed before) */
  sprintf(key, "%s(%s)", gedcom_n, charwidth);
  strcpy(val, iconv_n);

  if (hash_lookup(encodings, key)) {
    gedcom_warning(_("Duplicate entry found for encoding '%s', ignoring"),
		   gedcom_n);
    free(key);
    free(val);
  }
  else {
    hash_alloc_insert(encodings, key, val);
  }
}

char* get_encoding(char* gedcom_n, ENCODING enc)
{
  char *key;
  hnode_t *node;
  
  key = (char*)malloc(strlen(gedcom_n) + strlen(charwidth_string[enc]) + 3);
  /* sprintf is safe here (malloc'ed before) */
  sprintf(key, "%s(%s)", gedcom_n, charwidth_string[enc]);

  node = hash_lookup(encodings, key);
  free(key);
  if (node) {
    return hnode_get(node);
  }
  else {
    gedcom_error(_("No encoding defined for '%s'"), gedcom_n);
    return NULL;
  }
}

void cleanup_encodings()
{
  hash_free(encodings);
}

void init_encodings()
{
  if (encodings == NULL) {
    FILE *in;
    char buffer[MAXBUF + 1];
    char gedcom_n[MAXBUF + 1];
    char charwidth[MAXBUF + 1];
    char iconv_n[MAXBUF + 1];
    char *gconv_path;

    atexit(cleanup_encodings);
    
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
	gedcom_warning(_("Failed updating conversion module path"));
      }
    }

    encodings = hash_create(HASHCOUNT_T_MAX, NULL, NULL);
    hash_set_allocator(encodings, node_alloc, node_free, NULL);
    
    /* Open gedcom configuration file and read */
    in = fopen(ENCODING_CONF_FILE, "r");
    if (in == NULL) {
      char path[PATH_MAX];
      sprintf(path, "%s/%s", PKGDATADIR, ENCODING_CONF_FILE);
      in = fopen(path, "r");
    }
    if (in == NULL) {
      gedcom_warning(_("Could not open encoding configuration file '%s'"),
		     ENCODING_CONF_FILE);
    }
    else {
      line_no = 1;
      while (fgets(buffer, sizeof(buffer), in) != NULL) {
	if (buffer[strlen(buffer) - 1] != '\n') {
	  gedcom_error(_("Line too long in encoding configuration file '%s'"),
		       ENCODING_CONF_FILE);
	  return;
	}
	else if ((buffer[0] != '#') && (strcmp(buffer, "\n") != 0)) {
	  if (sscanf(buffer, "%s %s %s", gedcom_n, charwidth, iconv_n) == 3) {
	    add_encoding(gedcom_n, charwidth, iconv_n);
	  }
	  else {
	    gedcom_error(_("Missing data in encoding configuration file '%s'"),
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
      gedcom_error(_("Error opening conversion context for encoding %s: %s"),
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
