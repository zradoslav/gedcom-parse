dnl $Id$
dnl $Name$

dnl From codeset.m4 (in libcharset):
#serial 2

dnl From Bruno Haible.

AC_DEFUN(jm_LANGINFO_CODESET,
[
  AC_CHECK_HEADERS(langinfo.h)
  AC_CHECK_FUNCS(nl_langinfo)

  AC_CACHE_CHECK([for nl_langinfo and CODESET], jm_cv_langinfo_codeset,
    [AC_TRY_LINK([#include <langinfo.h>],
      [char* cs = nl_langinfo(CODESET);],
      jm_cv_langinfo_codeset=yes,
      jm_cv_langinfo_codeset=no)
    ])
  if test $jm_cv_langinfo_codeset = yes; then
    AC_DEFINE(HAVE_LANGINFO_CODESET, 1,
      [Define if you have <langinfo.h> and nl_langinfo(CODESET).])
  fi
])

dnl From glibc21.m4 (in libcharset):
#serial 2

# Test for the GNU C Library, version 2.1 or newer.
# From Bruno Haible.

AC_DEFUN(jm_GLIBC21,
  [
    AC_CACHE_CHECK(whether we are using the GNU C Library 2.1 or newer,
      ac_cv_gnu_library_2_1,
      [AC_EGREP_CPP([Lucky GNU user],
	[
#include <features.h>
#ifdef __GNU_LIBRARY__
 #if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1) || (__GLIBC__ > 2)
  Lucky GNU user
 #endif
#endif
	],
	ac_cv_gnu_library_2_1=yes,
	ac_cv_gnu_library_2_1=no)
      ]
    )
    AC_SUBST(GLIBC21)
    GLIBC21="$ac_cv_gnu_library_2_1"
  ]
)

dnl From iconv.m4:
#serial AM2

dnl From Bruno Haible.

AC_DEFUN([AM_ICONV],
[
  dnl Some systems have iconv in libc, some have it in libiconv (OSF/1 and
  dnl those with the standalone portable GNU libiconv installed).

  AC_ARG_WITH([libiconv-prefix],
[  --with-libiconv-prefix=DIR  search for libiconv in DIR/include and DIR/lib], [
    # Addition Peter Verthez
    ICONV_PATH=`echo "$withval" | tr : '/bin'`
    ICONV_PATH="$ICONV_PATH/bin"
    AC_SUBST(ICONV_PATH)
    # end of addition

    for dir in `echo "$withval" | tr : ' '`; do
      if test -d $dir/include; then CPPFLAGS="$CPPFLAGS -I$dir/include"; fi
      if test -d $dir/lib; then LDFLAGS="$LDFLAGS -L$dir/lib"; fi
    done
   ])

  AC_CACHE_CHECK(for iconv, am_cv_func_iconv, [
    am_cv_func_iconv="no, consider installing GNU libiconv"
    am_cv_lib_iconv=no
    AC_TRY_LINK([#include <stdlib.h>
#include <iconv.h>],
      [iconv_t cd = iconv_open("","");
       iconv(cd,NULL,NULL,NULL,NULL);
       iconv_close(cd);],
      am_cv_func_iconv=yes)
    if test "$am_cv_func_iconv" != yes; then
      am_save_LIBS="$LIBS"
      LIBS="$LIBS -liconv"
      AC_TRY_LINK([#include <stdlib.h>
#include <iconv.h>],
        [iconv_t cd = iconv_open("","");
         iconv(cd,NULL,NULL,NULL,NULL);
         iconv_close(cd);],
        am_cv_lib_iconv=yes
        am_cv_func_iconv=yes)
      LIBS="$am_save_LIBS"
    fi
  ])
  if test "$am_cv_func_iconv" = yes; then
    AC_DEFINE(HAVE_ICONV, 1, [Define if you have the iconv() function.])
    AC_MSG_CHECKING([for iconv declaration])
    AC_CACHE_VAL(am_cv_proto_iconv, [
      AC_TRY_COMPILE([
#include <stdlib.h>
#include <iconv.h>
extern
#ifdef __cplusplus
"C"
#endif
#if defined(__STDC__) || defined(__cplusplus)
size_t iconv (iconv_t cd, char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);
#else
size_t iconv();
#endif
], [], am_cv_proto_iconv_arg1="", am_cv_proto_iconv_arg1="const")
      am_cv_proto_iconv="extern size_t iconv (iconv_t cd, $am_cv_proto_iconv_arg1 char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);"])
    am_cv_proto_iconv=`echo "[$]am_cv_proto_iconv" | tr -s ' ' | sed -e 's/( /(/'`
    AC_MSG_RESULT([$]{ac_t:-
         }[$]am_cv_proto_iconv)
    AC_DEFINE_UNQUOTED(ICONV_CONST, $am_cv_proto_iconv_arg1,
      [Define as const if the declaration of iconv() needs const.])
  fi
  LIBICONV=
  if test "$am_cv_lib_iconv" = yes; then
    LIBICONV="-liconv"
  fi
  AC_SUBST(LIBICONV)
])

dnl Own functions
dnl AC_GLIBC_ICONV()
dnl Checks whether iconv is coming from glibc, defines USE_GLIBC_ICONV if so
dnl The variable $is_glibc_iconv contains yes or no
AC_DEFUN(AC_GLIBC_ICONV, [
  AC_CACHE_CHECK(for the GNU C Library iconv implementation, is_glibc_iconv, [
    AC_EGREP_CPP(yes,
    [
#include <iconv.h>
#ifdef __GLIBC__
yes
#endif
    ], is_glibc_iconv=yes, is_glibc_iconv=no)
  ])
  if test "$is_glibc_iconv" = yes; then
    AC_DEFINE(USE_GLIBC_ICONV,1,
      [Define if the GNU implementation of iconv in glibc is used])
  fi
])

dnl AC_LIBICONV_HAS_ANSEL()
dnl Checks whether libiconv has ANSEL support
dnl The variable $is_ansel_supported contains yes or no
AC_DEFUN(AC_LIBICONV_HAS_ANSEL, [
  AC_CACHE_CHECK(for ANSEL support in libiconv, is_ansel_supported, [
    my_save_LIBS="$LIBS"
    LIBS="$LIBS $LIBICONV"
    AC_TRY_RUN([
#include <iconv.h>
int main() {
  iconv_t cd = iconv_open("UTF-8","ANSEL");
  return (cd == (iconv_t)-1);
}
    ],
    is_ansel_supported=yes,
    is_ansel_supported=no,
    is_ansel_supported=no)
    LIBS="$my_save_LIBS"
  ])
])
