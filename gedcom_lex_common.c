/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

#ifndef IN_LEX

#include "gedcom.tab.h"
#include "gedcom.h"
#include "multilex.h"
#include "encoding.h"

#define YY_NO_UNPUT

static size_t encoding_width;
static int current_level = -1;
static int level_diff=MAXGEDCLEVEL;
static size_t line_len = 0;

static char ptr_buf[MAXGEDCPTRLEN * UTF_FACTOR + 1];
static char tag_buf[MAXGEDCTAGLEN * UTF_FACTOR + 1];
static char str_buf[MAXGEDCLINELEN * UTF_FACTOR + 1];

#ifdef LEXER_TEST 
YYSTYPE gedcom_lval;
int line_no = 1;

int gedcom_lex();

int test_loop(ENCODING enc, char* code)
{
  int tok, res;
  init_encodings();
  set_encoding_width(enc);
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
      case USERTAG: printf("USERTAG(%s) ", gedcom_lval.string); break;
      default: printf("TAG(%s) ", gedcom_lval.string); break;
    }
    tok = gedcom_lex();
  }
  printf("\n");
  close_conv_to_internal();
  return 0;  
}
 
#endif /* of #ifdef LEXER_TEST */

#else  /* of #ifndef IN_LEX */

#define TO_INTERNAL(STR,OUTBUF) \
  to_internal(STR, yyleng, OUTBUF, sizeof(OUTBUF))

#define INIT_LINE_LEN \
  line_len = 0;

#define CHECK_LINE_LEN                                                        \
  { if (line_len != (size_t)-1) {                                             \
      line_len += strlen(yytext);                                             \
      if (line_len > MAXGEDCLINELEN * encoding_width) {                       \
        gedcom_error("Line too long, max %d characters",                      \
		     MAXGEDCLINELEN);                                         \
        line_len = (size_t)-1;                                                \
        return BADTOKEN;                                                      \
      }                                                                       \
    }                                                                         \
  }

#define MKTAGACTION(THETAG)                                                  \
  { CHECK_LINE_LEN;                                                          \
    gedcom_lval.string = TO_INTERNAL(yytext, tag_buf);                       \
    BEGIN(NORMAL);                                                           \
    return TAG_##THETAG;                                                     \
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
   that returns "pending" tokens. */

#define ACTION_BEFORE_REGEXPS                                                 \
   { if (level_diff < 1) {                                                    \
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
   { gedcom_error ("Level number with leading zero");                         \
     return BADTOKEN;                                                         \
   } 


#define ACTION_DIGITS                                                         \
   { int level = atoi(TO_INTERNAL(yytext, str_buf));                          \
     CHECK_LINE_LEN;                                                          \
     if ((level < 0) || (level > MAXGEDCLEVEL)) {                             \
       gedcom_error ("Level number out of range [0..%d]",                     \
		     MAXGEDCLEVEL);                                           \
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
       gedcom_error ("GEDCOM level number is %d higher than "                 \
		     "previous",                                              \
		     level_diff);                                             \
       return BADTOKEN;                                                       \
     }                                                                        \
   } 


#define ACTION_ALPHANUM                                                       \
   { if (strlen(yytext) > MAXGEDCTAGLEN * encoding_width) {                   \
       gedcom_error("Tag '%s' too long, max %d characters",                   \
		    yytext, MAXGEDCTAGLEN);                                   \
       return BADTOKEN;                                                       \
     }                                                                        \
     CHECK_LINE_LEN;                                                          \
     gedcom_lval.string = TO_INTERNAL(yytext, tag_buf);                       \
     BEGIN(NORMAL);                                                           \
     return USERTAG;                                                          \
   }


#define ACTION_DELIM                                                          \
  { CHECK_LINE_LEN;                                                           \
    gedcom_lval.string = TO_INTERNAL(yytext, str_buf);                        \
    return DELIM;                                                             \
  }


#define ACTION_ANY                                                            \
  { CHECK_LINE_LEN;                                                           \
    gedcom_lval.string = TO_INTERNAL(yytext, str_buf);                        \
    /* Due to character conversions, it is possible that the current          \
       character will be combined with the next, and so now we don't have a   \
       character yet...                                                       \
       In principle, this is only applicable to the 1byte case (e.g. ANSEL),  \
       but it doesn't harm the unicode case.                                  \
    */                                                                        \
    if (strlen(gedcom_lval.string) > 0)                                       \
      return ANYCHAR;                                                         \
  }


#define ACTION_ESCAPE                                                         \
  { CHECK_LINE_LEN;                                                           \
    gedcom_lval.string = TO_INTERNAL(yytext, str_buf);                        \
    return ESCAPE;                                                            \
  }


#define ACTION_POINTER                                                        \
  { CHECK_LINE_LEN;                                                           \
    if (strlen(yytext) > MAXGEDCPTRLEN * encoding_width) {                    \
      gedcom_error("Pointer '%s' too long, max %d characters",                \
		   yytext, MAXGEDCPTRLEN);                                    \
      return BADTOKEN;                                                        \
    }                                                                         \
    gedcom_lval.string = TO_INTERNAL(yytext, ptr_buf);                        \
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
    line_no++;                                                                \
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
      yyterminate();                                                          \
    }                                                                         \
  } 


#define ACTION_UNEXPECTED                                                     \
  { gedcom_error("Unexpected character: '%s' (0x%02x)",                       \
		 yytext, yytext[0]);                                          \
    return BADTOKEN;                                                          \
  }

#endif /* IN_LEX */