dnl Autoconf/automake macros for the Gedcom parser library
dnl Peter Verthez
dnl 
dnl $Id$
dnl $Name$

dnl AM_LIB_GEDCOM_PARSER([MINIMUM-MAJOR-VERSION, [MINIMUM-MINOR-VERSION]])
dnl Test for the availability of the Gedcom parser library, and check whether
dnl the required minimum version is available (parameters default to 0 if
dnl not given
AC_DEFUN(AM_LIB_GEDCOM_PARSER, [
  AC_CHECK_LIB(gedcom, gedcom_parse_file,,
     AC_MSG_ERROR([Cannot find libgedcom: Please install gedcom-parse]))
  major_version=ifelse([$1], ,0,$1)
  minor_version=ifelse([$2], ,0,$2)
  AC_MSG_CHECKING(for libgedcom version >= $major_version.$minor_version)
  AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <gedcom.h>
int
main()
{
if (GEDCOM_PARSE_VERSION >= $major_version * 1000 + $minor_version) exit(0);
exit(1);
}],
ac_gedcom_version_ok='yes',
ac_gedcom_version_ok='no',
ac_gedcom_version_ok='no')
if test "$ac_gedcom_version_ok" = 'yes' ; then
  AC_MSG_RESULT(ok)
else
  AC_MSG_RESULT(not ok)
  AC_MSG_ERROR(You need at least version $major_version.$minor_version of gedcom-parse)
fi
])
