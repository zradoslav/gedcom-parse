/*  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *

 (C) 2001 by The Genes Development Team
 Original author: Peter Verthez (Peter.Verthez@advalvas.be)
*/

/* $Id$ */
/* $Name$ */

char string_buf[MAXGEDCLINELEN+1];
 
#define TO_INTERNAL(str) to_internal(str, yyleng) 

#define MKTAGACTION(the_tag) \
  { gedcom_lval.tag = TO_INTERNAL(yytext); \
    BEGIN(NORMAL); \
    return TAG_##the_tag; }


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
       gedcom_lval.level = current_level;                                     \
       return OPEN;                                                           \
     }                                                                        \
     else {                                                                   \
       /* out of brackets... */                                               \
     }                                                                        \
   } 


#define ACTION_0_DIGITS                                                       \
   { gedcom_error ("Level number with leading zero");                         \
     return BADTOKEN;                                                         \
   } 


#define ACTION_DIGITS                                                         \
   { int level = atoi(TO_INTERNAL(yytext));                                   \
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
       gedcom_lval.level = current_level;                                     \
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
   { if (strlen(yytext) > MAXGEDCTAGLEN) {                                    \
       gedcom_error("Tag '%s' too long, max %d chars");                       \
       return BADTOKEN;                                                       \
     }                                                                        \
     strncpy(string_buf, yytext, MAXGEDCTAGLEN+1);                            \
     gedcom_lval.tag = TO_INTERNAL(string_buf);                               \
     BEGIN(NORMAL);                                                           \
     return USERTAG;                                                          \
   }


#define ACTION_DELIM                                                          \
  { gedcom_lval.string = TO_INTERNAL(yytext);                                 \
    return DELIM;                                                             \
  }


#define ACTION_ANY                                                            \
  { gedcom_lval.string = TO_INTERNAL(yytext);                                 \
    /* Due to character conversions, it is possible                           \
       that the current character will be combined with                       \
       the next, and so now we don't have a character yet...                  \
       In principle, this is only applicable to the 1byte case (e.g. ANSEL),  \
       but it doesn't harm the unicode case.                                  \
    */                                                                        \
    if (strlen(gedcom_lval.string) > 0)                                       \
      return ANYCHAR;                                                         \
  }


#define ACTION_ESCAPE                                                         \
  { gedcom_lval.string = TO_INTERNAL(yytext);                                 \
    return ESCAPE;                                                            \
  }


#define ACTION_POINTER                                                        \
  { gedcom_lval.pointer = TO_INTERNAL(yytext);                                \
    return POINTER;                                                           \
  }


/* Due to the conversion of level numbers into brackets, the
   terminator is not important, so no token is returned here.
   Although not strictly according to the GEDCOM spec, we'll ignore
   whitespace just before the terminator.
*/

#define ACTION_TERMINATOR                                                     \
  { line_no++;                                                                \
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
