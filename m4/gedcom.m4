dnl Autoconf/automake macros for the Gedcom parser library
dnl Peter Verthez
dnl partly stolen from Manish Singh
dnl stolen back from Frank Belew
dnl stolen from Manish Singh
dnl Shamelessly stolen from Owen Taylor
dnl 
dnl $Id$
dnl $Name$

dnl AM_PATH_GEDCOM_PARSER([MINIMUM-VERSION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for gedcom-parse, and define GEDCOM_CFLAGS and GEDCOM_LIBS
dnl
AC_DEFUN(AM_PATH_GEDCOM_PARSER, [
AC_ARG_WITH(gedcom-prefix,[  --with-gedcom-prefix=PFX   Prefix where the Gedcom parser library is installed (optional)],
            gedcom_config_prefix="$withval", gedcom_config_prefix="")
AC_ARG_WITH(gedcom-exec-prefix,[  --with-gedcom-exec-prefix=PFX Exec prefix where the Gedcom parser library is installed (optional)],
            gedcom_config_exec_prefix="$withval", gedcom_config_exec_prefix="")
AC_ARG_ENABLE(gedcomtest, [  --disable-gedcomtest       Do not try to compile and run a test program with the Gedcom parser library],
		    , enable_gedcomtest=yes)

  for module in . $4
  do
      case "$module" in
         gom) 
             gedcom_config_args="$gedcom_config_args gom"
         ;;
      esac
  done

  if test x$gedcom_config_exec_prefix != x ; then
     gedcom_config_args="$gedcom_config_args --exec-prefix=$gedcom_config_exec_prefix"
     if test x${GEDCOM_CONFIG+set} != xset ; then
        GEDCOM_CONFIG=$gedcom_config_exec_prefix/bin/gedcom-config
     fi
  fi
  if test x$gedcom_config_prefix != x ; then
     gedcom_config_args="$gedcom_config_args --prefix=$gedcom_config_prefix"
     if test x${GEDCOM_CONFIG+set} != xset ; then
        GEDCOM_CONFIG=$gedcom_config_prefix/bin/gedcom-config
     fi
  fi

  AC_PATH_PROG(GEDCOM_CONFIG, gedcom-config, no)
  min_gedcom_version=ifelse([$1], ,0.20.0,$1)
  AC_MSG_CHECKING(for Gedcom parser library - version >= $min_gedcom_version)
  no_gedcom=""
  if test "$GEDCOM_CONFIG" = "no" ; then
    no_gedcom=yes
  else
    GEDCOM_CFLAGS=`$GEDCOM_CONFIG $gedcom_config_args --cflags`
    GEDCOM_LIBS=`$GEDCOM_CONFIG $gedcom_config_args --libs`
    gedcom_config_major_vers=`$GEDCOM_CONFIG $gedcom_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gedcom_config_minor_vers=`$GEDCOM_CONFIG $gedcom_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gedcom_config_patch_vers=`$GEDCOM_CONFIG $gedcom_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gedcomtest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GEDCOM_CFLAGS"
      LIBS="$GEDCOM_LIBS $LIBS"
dnl
dnl Now check if the installed Gedcom parser is sufficiently new. (Also sanity
dnl checks the results of gedcom-config to some extent
dnl
      rm -f conf.gedcomtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gedcom.h"

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gedcomtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_gedcom_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gedcom_version");
     exit(1);
   }

  if (0) {
  }
#ifdef GEDCOM_PARSE_VERSION_PATCH
  else if ((GEDCOM_PARSE_VERSION_MAJOR != $gedcom_config_major_vers) ||
           (GEDCOM_PARSE_VERSION_MINOR != $gedcom_config_minor_vers) ||
           (GEDCOM_PARSE_VERSION_PATCH != $gedcom_config_patch_vers))
    {
      printf("\n*** 'gedcom-config --version' returned %d.%d.%d, but gedcom-parse (%d.%d.%d)\n", 
             $gedcom_config_major_vers, $gedcom_config_minor_vers, $gedcom_config_patch_vers,
             GEDCOM_PARSE_VERSION_MAJOR, GEDCOM_PARSE_VERSION_MINOR,
             GEDCOM_PARSE_VERSION_PATCH);
      printf ("*** was found! If gedcom-config was correct, then it is best to remove the\n");
      printf ("*** old version of gedcom-parse. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If gedcom-config was wrong, set the environment variable GEDCOM_CONFIG\n");
      printf("*** to point to the correct copy of gedcom-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
      return 1;
    }

  else if (gedcom_check_version(major, minor, micro)) {
    return 0;
  }
#else
  else if (GEDCOM_PARSE_VERSION >= major * 1000 + minor) {
    return 0;
  }
#endif

  else
      {
        printf("\n*** An old version of gedcom-parse (%d.%d.%d) was found.\n",
               GEDCOM_PARSE_VERSION_MAJOR, GEDCOM_PARSE_VERSION_MINOR,
               GEDCOM_PARSE_VERSION_PATCH);
        printf("*** You need a version of gedcom-parse newer than %d.%d.%d. The latest version\n",
	       major, minor, micro);
        printf("*** of gedcom-parse is always available from the following location:\n");
        printf("*** https://sourceforge.net/projects/gedcom-parse\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the gedcom-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of gedcom-parse, but you can also set the GEDCOM_CONFIG environment to\n");
        printf("*** point to the correct copy of gedcom-config. (In this case, you will have\n");
        printf("*** to modify your LD_LIBRARY_PATH environment var., or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
        return 1;
      }
}
],, no_gedcom=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gedcom" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GEDCOM_CONFIG" = "no" ; then
       echo "*** The gedcom-config script installed by gedcom-parse could not be found"
       echo "*** If gedcom-parse was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GEDCOM_CONFIG environment variable to the"
       echo "*** full path to gedcom-config."
       echo "*** It could also be that your version of gedcom-parse is too old."
       echo "*** You need at least version $min_gedcom_version.  The latest version"
       echo "*** of gedcom-parse can always be found at this location:"
       echo "*** https://sourceforge.net/projects/gedcom-parse"
     else
       if test -f conf.gedcomtest ; then
        :
       else
          echo "*** Could not run gedcom-parse test program, checking why..."
          CFLAGS="$CFLAGS $GEDCOM_CFLAGS"
          LIBS="$LIBS $GEDCOM_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include "gedcom.h"
],      [ return ((GEDCOM_PARSE_VERSION_MAJOR) || (GEDCOM_PARSE_VERSION_MINOR) || (GEDCOM_PARSE_VERSION_PATCH)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding gedcom-parse or finding the wrong"
          echo "*** version of gedcom-parse. If it is not finding gedcom-parse, you'll need"
          echo "*** to set your LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf"
          echo "*** to point to the installed location  Also, make sure you have run ldconfig"
          echo "*** if that is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means gedcom-parse was incorrectly"
          echo "*** installed or that you have moved gedcom-parse since it was installed."
          echo "*** In the latter case, you may want to edit the gedcom-config script: $GEDCOM_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GEDCOM_CFLAGS=""
     GEDCOM_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GEDCOM_CFLAGS)
  AC_SUBST(GEDCOM_LIBS)
  rm -f conf.gedcomtest
])
