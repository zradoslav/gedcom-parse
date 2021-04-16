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
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "gedcom_internal.h"
#include "gedcom.h"
#include "encoding.h"
#include "encoding_state.h"
#include "hash.h"
#include "utf8tools.h"

#define ENCODING_CONF_FILE "gedcom.enc"
#define MAXBUF 255

static hash_t *encodings = NULL;

const char* charwidth_string[] = { "1", "2_HILO", "2_LOHI" };

hnode_t *node_alloc(void *c UNUSED)
{
  return (hnode_t *)malloc(sizeof *node_alloc(NULL));
}

void node_free(hnode_t *n, void *c UNUSED)
{
  free((void*)hnode_getkey(n));
  free(hnode_get(n));
  free(n);
}

void add_encoding(const char *gedcom_n, const char* charwidth,
		  const char *iconv_n)
{
  char *key, *val;

  key = (char *) malloc(strlen(gedcom_n) + strlen(charwidth) + 3);
  val = (char *) malloc(strlen(iconv_n) + 1);

  if (key && val) {
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
  else
    MEMORY_ERROR;
}

char* get_encoding(const char* gedcom_n, Encoding enc)
{
  char *key;
  hnode_t *node;

  if (encodings == NULL) return NULL;
  
  key = (char*)malloc(strlen(gedcom_n) + strlen(charwidth_string[enc]) + 3);

  if (key) {
    char* sp_pos = NULL;
    while ((sp_pos = strchr(gedcom_n, ' ')) != NULL) {
      *sp_pos = '_';
    }
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
  else {
    MEMORY_ERROR;
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

    if (atexit(cleanup_encodings) != 0) {
      gedcom_warning(_("Could not register encoding cleanup function"));
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
      gedcom_warning(_("Could not open encoding configuration file '%s': %s"),
		     ENCODING_CONF_FILE, strerror(errno));
    }
    else {
      line_no = 1;
      while (fgets(buffer, sizeof(buffer), in) != NULL) {
	if (buffer[strlen(buffer) - 1] != '\n') {
	  gedcom_error(_("Line too long in encoding configuration file '%s'"),
		       ENCODING_CONF_FILE);
	  line_no = 0;
	  return;
	}
	else if ((buffer[0] != '#') && (strcmp(buffer, "\n") != 0)) {
	  if (sscanf(buffer, "%s %s %s", gedcom_n, charwidth, iconv_n) == 3) {
	    add_encoding(gedcom_n, charwidth, iconv_n);
	  }
	  else {
	    gedcom_error(_("Missing data in encoding configuration file '%s'"),
			 ENCODING_CONF_FILE);
	    line_no = 0;
	    return;
	  }
	}
      }
      line_no = 0;
      if (fclose(in) != 0) {
	gedcom_warning(_("Error closing file '%s': %s"),
		       ENCODING_CONF_FILE, strerror(errno));
      }
    }
  }
}

static convert_t to_int = NULL;
static char* error_value = "<error>";

int open_conv_to_internal(const char* fromcode)
{
  convert_t new_to_int = NULL;
  const char *encoding = get_encoding(fromcode, read_encoding.width);
  
  if (encoding != NULL) {
    new_to_int = initialize_utf8_conversion(encoding, 1);
    if (new_to_int == NULL) {
      gedcom_error(_("Error opening conversion context for encoding %s: %s"),
		   encoding, strerror(errno));
    }
  }

  if (new_to_int != NULL) {
    if (to_int != NULL)
      cleanup_utf8_conversion(to_int);
    to_int = new_to_int;
    set_read_encoding(fromcode, encoding);
  }

  return (new_to_int != NULL);
}

void close_conv_to_internal()
{
  if (to_int != NULL) {
    cleanup_utf8_conversion(to_int);
    to_int = NULL;
  }
}


char* to_internal(const char* str, size_t len, struct conv_buffer* output_buf)
{
  if (conversion_set_output_buffer(to_int, output_buf))
    return convert_to_utf8_incremental(to_int, str, len);
  else
    return error_value;
}
