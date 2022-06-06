/* Common lexer code.
   Copyright (C) 2001, 2002 The Genes Development Team
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

#if LEX_SECTION == 1

#include "gedcom_internal.h"
#include "multilex.h"
#include "encoding.h"
#include "encoding_state.h"
#include "gedcom.h"
#include "gedcom.tabgen.h"
#include "compat.h"

static size_t encoding_width;
static int current_level = -1;
static int level_diff = MAXGEDCLEVEL;
static size_t line_len = 0;
static int tab_space = 0;
static int current_tag = -1;

static struct conv_buffer* ptr_buffer = NULL;
static struct conv_buffer* tag_buffer = NULL;
static struct conv_buffer* str_buffer = NULL;

#define INITIAL_PTR_BUFFER_LEN MAXGEDCPTRLEN * UTF_FACTOR + 1
#define INITIAL_TAG_BUFFER_LEN MAXGEDCTAGLEN * UTF_FACTOR + 1
#define INITIAL_STR_BUFFER_LEN MAXGEDCLINELEN * UTF_FACTOR + 1

#ifdef LEXER_TEST 
YYSTYPE gedcom_lval;
int line_no = 1;

int gedcom_lex();

void message_handler(Gedcom_msg_type type, char *msg)
{
  fprintf(stderr, "(%d) %s\n", type, msg);
}

int test_loop(ENCODING enc, const char* code)
{
  int tok, res;
  init_encodings();
  set_encoding_width(enc);
  gedcom_set_message_handler(message_handler);
  res = open_conv_to_internal(code);
  if (!res) {
    gedcom_error("Unable to open conversion context: %s",
		 strerror(errno));
    return 1;
  }
  tok = gedcom_lex();
  while (tok) {
    switch(tok) {
      case BADTOKEN: printf("BADTOKEN "); break;
      case OPEN: printf("OPEN(%d) ", gedcom_lval.number); break;
      case CLOSE: printf("CLOSE "); break;
      case ESCAPE: printf("ESCAPE(%s) ", gedcom_lval.string); break;
      case DELIM: printf("DELIM "); break;
      case ANYCHAR: printf("%s ", gedcom_lval.string); break;
      case POINTER: printf("POINTER(%s) ", gedcom_lval.string); break;
      case USERTAG: printf("USERTAG(%s) ", gedcom_lval.tag.string); break;
      default: printf("TAG(%s) ", gedcom_lval.tag.string); break;
    }
    tok = gedcom_lex();
  }
  printf("\n");
  close_conv_to_internal();
  return 0;  
}
 
#endif /* of #ifdef LEXER_TEST */

/* These are defined as functions here, because xgettext has trouble
   extracting the strings out of long pre-processor defined */

static void error_line_too_long()
{
  gedcom_error(_("Line too long, max %d characters allowed"),
	       MAXGEDCLINELEN); 
}

static void error_level_leading_zero()
{
  gedcom_error (_("Level number with leading zero not allowed"));
}

static void error_level_out_of_range()
{
  gedcom_error (_("Level number out of range [0..%d]"), MAXGEDCLEVEL); 
}

static void error_level_too_high(int level_diff)
{
  gedcom_error (_("GEDCOM level number is %d higher than previous"),
		level_diff); 
}

static void error_tag_too_long(const char *tag)
{
  gedcom_error(_("Tag '%s' too long, max %d characters allowed"),
	       tag, MAXGEDCTAGLEN); 
}

static void error_invalid_character(const char *str, char ch)
{
  gedcom_error(_("Invalid character for encoding: '%s' (0x%02x)"), str, ch); 
}

static void error_pointer_too_long(const char *ptr)
{
  gedcom_error(_("Pointer '%s' too long, max %d characters allowed"),
	       ptr, MAXGEDCPTRLEN);
}

static void error_at_character()
{
  gedcom_error(_("'@' character should be written as '@@' in values"));
}

static void error_tab_character()
{
  gedcom_error(_("Tab character is not allowed in values"));
}

static void error_unexpected_character(const char* str, char ch)
{
  gedcom_error(_("Unexpected character: '%s' (0x%02x)"), str, ch);
}

/* This is to bypass the iconv conversion (if the input is UTF-8 coming
   from the program) */
static int dummy_conv = 0;

#elif LEX_SECTION == 2

#define TO_INTERNAL(STR,OUTBUF) \
  (dummy_conv ? STR : to_internal(STR, yyleng, OUTBUF))

#define INIT_LINE_LEN \
  line_len = 0;

#define CHECK_LINE_LEN                                                        \
  { if (line_len != (size_t)-1) {                                             \
      line_len += strlen(yytext);                                             \
      if (line_len > MAXGEDCLINELEN * encoding_width                          \
	  && ! compat_long_line(current_level, current_tag)) {                \
        error_line_too_long();                                                \
        line_len = (size_t)-1;                                                \
        return BADTOKEN;                                                      \
      }                                                                       \
    }                                                                         \
  }

#define GENERATE_TAB_SPACE                                                    \
  { gedcom_lval.string = " ";                                                 \
    tab_space--;                                                              \
    return DELIM;                                                             \
  }

#define MKTAGACTION(THETAG)                                                  \
  { CHECK_LINE_LEN;                                                          \
    gedcom_lval.tag.string = TO_INTERNAL(yytext, tag_buffer);                \
    current_tag            = TAG_##THETAG;                                   \
    gedcom_lval.tag.value  = current_tag;                                    \
    BEGIN(NORMAL);                                                           \
    line_no++;                                                               \
    return current_tag;                                                      \
  }

/* The GEDCOM level number is converted into a sequence of opening
   and closing brackets.  Simply put, the following GEDCOM fragment:
   
   0 HEAD
   1 SOUR genes
   2 VERS 1.6
   2 NAME Genes
   1 DATE 07 OCT 2001
   ...
   0 TRLR
   
   is converted into:
   
   { HEAD                     (initial)  
   { SOUR genes               (1 higher: no closing brackets)
   { VERS 1.6                 (1 higher: no closing brackets)
   } { NAME Genes             (same level: 1 closing bracket)
   } } { DATE 07 OCT 2001     (1 lower: 2 closing brackets)
   ...
   } { TRLR }
   
   or more clearly:
   
   { HEAD
     { SOUR genes
       { VERS 1.6 }
       { NAME Genes } }
     { DATE 07 OCT 2001
     ... }
   { TRLR }

   But because this means that one token is converted into a series
   of tokens, there is some initial code following immediately here
   that returns "pending" tokens.

   Also, for compatibility tabs are converted into spaces, which is
   also handled here */

#define ACTION_BEFORE_REGEXPS                                                 \
   { if (compat_mode(C_TAB_CHARACTER) && tab_space-- > 0) {                   \
       GENERATE_TAB_SPACE;                                                    \
     }                                                                        \
     else if (level_diff < 1) {                                               \
       level_diff++;                                                          \
       return CLOSE;                                                          \
     }                                                                        \
     else if (level_diff == 1) {                                              \
       level_diff++;                                                          \
       gedcom_lval.number = current_level;                                    \
       return OPEN;                                                           \
     }                                                                        \
     else {                                                                   \
       /* out of brackets... */                                               \
     }                                                                        \
   }


#define ACTION_INITIAL_WHITESPACE                                             \
  { CHECK_LINE_LEN;                                                           \
    /* ignore initial whitespace further */                                   \
  }


#define ACTION_0_DIGITS                                                       \
   { error_level_leading_zero();                                              \
     return BADTOKEN;                                                         \
   } 


#define ACTION_DIGITS                                                         \
   { int level = atoi(TO_INTERNAL(yytext, str_buffer));                       \
     CHECK_LINE_LEN;                                                          \
     if ((level < 0) || (level > MAXGEDCLEVEL)) {                             \
       error_level_out_of_range();                                            \
       line_no++;                                                             \
       return BADTOKEN;                                                       \
     }                                                                        \
     level_diff = level - current_level;                                      \
     BEGIN(EXPECT_TAG);                                                       \
     current_level = level;                                                   \
     if (level_diff < 1) {                                                    \
       level_diff++;                                                          \
       return CLOSE;                                                          \
     }                                                                        \
     else if (level_diff == 1) {                                              \
       level_diff++;                                                          \
       gedcom_lval.number = current_level;                                    \
       return OPEN;                                                           \
     }                                                                        \
     else {                                                                   \
       /* should never happen (error to GEDCOM spec) */                       \
       error_level_too_high(level_diff);                                      \
       line_no++;                                                             \
       return BADTOKEN;                                                       \
     }                                                                        \
   } 


#define ACTION_ALPHANUM                                                       \
   { if (strlen(yytext) > MAXGEDCTAGLEN * encoding_width) {                   \
       error_tag_too_long(yytext);                                            \
       line_no++;                                                             \
       return BADTOKEN;                                                       \
     }                                                                        \
     CHECK_LINE_LEN;                                                          \
     gedcom_lval.tag.string = TO_INTERNAL(yytext, tag_buffer);                \
     gedcom_lval.tag.value  = USERTAG;                                        \
     BEGIN(NORMAL);                                                           \
     line_no++;                                                               \
     return USERTAG;                                                          \
   }


#define ACTION_DELIM                                                          \
  { CHECK_LINE_LEN;                                                           \
    gedcom_lval.string = TO_INTERNAL(yytext, str_buffer);                     \
    return DELIM;                                                             \
  }


#define ACTION_ANY                                                            \
  { char* tmp;                                                                \
    CHECK_LINE_LEN;                                                           \
    tmp = TO_INTERNAL(yytext, str_buffer);                                    \
    if (!tmp) {                                                               \
      /* Something went wrong during conversion... */                         \
          error_invalid_character(yytext, yytext[0]);                         \
          return BADTOKEN;                                                    \
    }                                                                         \
    else {                                                                    \
      gedcom_lval.string = tmp;                                               \
      /* Due to character conversions, it is possible that the current        \
         character will be combined with the next, and so now we don't have a \
         character yet...                                                     \
         In principle, this is only applicable to the 1byte case (e.g. ANSEL),\
         but it doesn't harm the unicode case.                                \
      */                                                                      \
      if (strlen(gedcom_lval.string) > 0)                                     \
        return ANYCHAR;                                                       \
    }                                                                         \
  }


#define ACTION_ESCAPE                                                         \
  { CHECK_LINE_LEN;                                                           \
    gedcom_lval.string = TO_INTERNAL(yytext, str_buffer);                     \
    return ESCAPE;                                                            \
  }


#define ACTION_POINTER                                                        \
  { CHECK_LINE_LEN;                                                           \
    if (strlen(yytext) > MAXGEDCPTRLEN * encoding_width) {                    \
      error_pointer_too_long(yytext);                                         \
      return BADTOKEN;                                                        \
    }                                                                         \
    gedcom_lval.string = TO_INTERNAL(yytext, ptr_buffer);                     \
    return POINTER;                                                           \
  }


/* Due to the conversion of level numbers into brackets, the
   terminator is not important, so no token is returned here.
   Although not strictly according to the GEDCOM spec, we'll ignore
   whitespace just before the terminator.
*/

#define ACTION_TERMINATOR                                                     \
  { CHECK_LINE_LEN;                                                           \
    INIT_LINE_LEN;                                                            \
    if (line_no == 1)                                                         \
      set_read_encoding_terminator(TO_INTERNAL(yytext, str_buffer));          \
    BEGIN(INITIAL);                                                           \
  }


/* Eventually we have to return 1 closing bracket (for the trailer).
   We can detect whether we have sent the closing bracket using the
   level_diff (at eof, first it is 2, then we increment it ourselves)
*/

#define ACTION_EOF                                                            \
  { if (level_diff == 2) {                                                    \
      level_diff++;                                                           \
      return CLOSE;                                                           \
    }                                                                         \
    else {                                                                    \
      char* ptr; int size;                                                    \
      /* ... terminate lex */                                                 \
      yyterminate();                                                          \
      /* Get rid of f*cking compiler warning from lex generated code */       \
      /* yyterminate does return(), so program will never come here  */       \
      yy_flex_realloc(ptr, size);                                             \
    }                                                                         \
  } 

#define ACTION_NORMAL_AT                                                      \
  { if (compat_mode(C_NO_DOUBLE_AT)) {                                        \
      int i, j;                                                               \
      char *yycopy = strdup(yytext);                                          \
      if (yycopy) {                                                           \
        for (i = 0; i < 2; i++)                                               \
          for (j = yyleng - 1; j >= 0; --j)                                   \
            unput(yycopy[j]);                                                 \
        free(yycopy);                                                         \
      }                                                                       \
      else {                                                                  \
        MEMORY_ERROR;                                                         \
      }                                                                       \
    }                                                                         \
    else {                                                                    \
      error_at_character();                                                   \
      return BADTOKEN;                                                        \
    }                                                                         \
  }

#define ACTION_TAB                                                            \
  { if (compat_mode(C_TAB_CHARACTER)) {                                       \
      tab_space = 8;                                                          \
      GENERATE_TAB_SPACE;                                                     \
    }                                                                         \
    else {                                                                    \
      error_tab_character();                                                  \
      return BADTOKEN;                                                        \
    }                                                                         \
  }

#define ACTION_UNEXPECTED                                                     \
  { error_unexpected_character(yytext, yytext[0]);                            \
    return BADTOKEN;                                                          \
  }

#elif LEX_SECTION == 3

int yywrap()
{
  return 1;
}

static void free_conv_buffers()
{
  free_conv_buffer(ptr_buffer);
  free_conv_buffer(tag_buffer);
  free_conv_buffer(str_buffer);
}

static void yylex_cleanup()
{
  /* fix memory leak in lex */
#ifdef FIXME
  yy_delete_buffer(yy_current_buffer);
  yy_current_buffer = NULL;
#endif
  free_conv_buffers();
}

static void init_conv_buffers()
{
  if (!ptr_buffer) {
    ptr_buffer = create_conv_buffer(INITIAL_PTR_BUFFER_LEN);
    tag_buffer = create_conv_buffer(INITIAL_TAG_BUFFER_LEN);
    str_buffer = create_conv_buffer(INITIAL_STR_BUFFER_LEN);
  }
}

static int exitfuncregistered = 0;

void yymyinit(FILE *f)
{
  if (! exitfuncregistered && atexit(yylex_cleanup) == 0)
    exitfuncregistered = 1;
  init_conv_buffers();
  yyin = f;
  yyrestart(f);
  /* Reset our state */
  current_level = -1;
  level_diff = MAXGEDCLEVEL;
  BEGIN(INITIAL);
}

#endif
