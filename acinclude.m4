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

dnl Own functions
dnl gedcom_GLIBC_ICONV()
dnl Checks whether iconv is coming from glibc, defines USE_GLIBC_ICONV if so
dnl The variable $is_glibc_iconv contains yes or no
AC_DEFUN(gedcom_GLIBC22_ICONV, [
  AC_CACHE_CHECK(for the GNU C Library 2.2 iconv implementation, is_glibc22_iconv, [
    AC_EGREP_CPP(yes,
    [
#include <iconv.h>
#ifdef __GLIBC__
 #if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2) || (__GLIBC__ > 2)
yes
 #endif
#endif
    ], is_glibc22_iconv=yes, is_glibc22_iconv=no)
  ])
  if test "$is_glibc22_iconv" = yes; then
    AC_DEFINE(USE_GLIBC_ICONV,1,
      [Define if the GNU implementation of iconv in glibc is used])
  fi
])

dnl gedcom_ICONV_HAS_CONV()
dnl Checks whether iconv has support to convert $1 to $2
dnl The variable $iconv_has_conv contains yes or no afterwards
dnl (overwritten on subsequent calls)
AC_DEFUN(gedcom_ICONV_HAS_CONV, [
  my_save_LIBS="$LIBS"
  LIBS="$LIBS $LIBICONV"
  AC_TRY_RUN([
#include <iconv.h>
int main() {
  iconv_t cd = iconv_open("$2","$1");
  return (cd == (iconv_t)-1);
}
    ],
    iconv_has_conv=yes,
    iconv_has_conv=no,
    iconv_has_conv=no)
    LIBS="$my_save_LIBS"
  ])
])

dnl gedcom_SANE_ICONV()
dnl Checks whether the iconv implementation has the basic functionality
dnl that we need
dnl The variable $is_iconv_sane contains yes or no
AC_DEFUN(gedcom_SANE_ICONV, [
  AC_CACHE_CHECK(whether iconv has the needed functionality, is_iconv_sane, [
    is_iconv_sane=yes
    gedcom_ICONV_HAS_CONV(ASCII, UTF-8)
    if test "$iconv_has_conv" = "no"; then
      is_iconv_sane=no
    else
      gedcom_ICONV_HAS_CONV(UCS-2LE, UTF-8)
      if test "$iconv_has_conv" = "no"; then
        is_iconv_sane=no
      else
        gedcom_ICONV_HAS_CONV(UCS-2BE, UTF-8)
        if test "$iconv_has_conv" = "no"; then
          is_iconv_sane=no
        fi
      fi
    fi
  ])
])

dnl gedcom_LIBICONV_HAS_ANSEL()
dnl Checks whether libiconv has ANSEL support
dnl The variable $is_ansel_supported contains yes or no
AC_DEFUN(gedcom_LIBICONV_HAS_ANSEL, [
  AC_CACHE_CHECK(for ANSEL support in libiconv, is_ansel_supported, [
    is_ansel_supported=no
    gedcom_ICONV_HAS_CONV(ANSEL, UTF-8)
    if test "$iconv_has_conv" = yes; then
      is_ansel_supported=yes
    fi
  ])
])

dnl gedcom_SYS_NEWLINE()
dnl Checks how newline is written on the system
dnl SYS_NEWLINE is set to one of the following:
dnl END_CR, END_LF, END_CR_LF, END_LF_CR
AC_DEFUN(gedcom_SYS_NEWLINE, [
  AC_CACHE_CHECK(how to represent newline, ac_cv_system_newline, [
    echo > newlinetest
    AC_TRY_RUN([
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
int main() {
  char buffer[11];
  int i, fd;
  FILE* f;
  for (i=0; i<10; i++) { buffer[i] = '\0'; }
  fd = open("newlinetest", O_RDONLY);
  if (fd == -1) return 1;
  read(fd, buffer, 10);
  close(fd);
  f = fopen("newlinetest", "w");
  if (!f) return 1;
  i = 0;
  while (buffer[i] != '\0') { fprintf(f, "%02x", buffer[i++]); }
  fclose(f);
  return 0;
}
    ],
    [system_newline_output=`cat newlinetest`
     case "$system_newline_output" in
       0a0d) ac_cv_system_newline="\"\x0A\x0D\"" ;;
       0d0a) ac_cv_system_newline="\"\x0D\x0A\"" ;;
       0a)   ac_cv_system_newline="\"\x0A\"" ;;
       0d)   ac_cv_system_newline="\"\x0D\"" ;;
       *)    ac_cv_system_newline="\"\x0A\"" ;;
     esac],
    ac_cv_system_newline="\"\x0A\"",
    ac_cv_system_newline="\"\x0A\"")
    rm -f newlinetest
  ])
  AC_DEFINE_UNQUOTED(SYS_NEWLINE,$ac_cv_system_newline,
    [The representation of newline in text files in the system])
])
