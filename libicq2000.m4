# Configure paths for libicq2000
# Erik Andersen	30 May 1998
# Modified by Tero Pulkkinen (added the compiler checks... I hope they work..)
# Modified by Thomas Langen 16 Jan 2000 (corrected CXXFLAGS)
# Borrowed for libicq2000, Barnaby Gray Dec 2001

dnl Test for LIBICQ2000, and define:
dnl  LIBICQ2000_CFLAGS
dnl  LIBICQ2000_LIBS
dnl  LIBICQ2000_LIBFLAGS (just -R/-L)
dnl  LIBICQ2000_LIBADD   (just -l)
dnl   to be used as follows:
dnl AM_PATH_LIBICQ2000([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
AC_DEFUN(AM_PATH_LIBICQ2000,
[dnl 
dnl Get the cflags and libraries from the libicq2000-config script
dnl
AC_ARG_WITH(libicq2000-prefix,[  --with-libicq2000-prefix=PREFIX
                          Prefix where libicq2000 is installed (optional)],
            libicq2000_config_prefix="$withval", libicq2000_config_prefix="")
AC_ARG_WITH(libicq2000-exec-prefix,[  --with-libicq2000-exec-prefix=PREFIX
                          Exec prefix where libicq2000 is installed (optional)],
            libicq2000_config_exec_prefix="$withval", libicq2000_config_exec_prefix="")
AC_ARG_ENABLE(libicq2000test, [  --disable-libicq2000test     Do not try to compile and run a test libicq2000 program],
		    , enable_libicq2000test=yes)

  if test x$libicq2000_config_exec_prefix != x ; then
     if test x${LIBICQ2000_CONFIG+set} != xset ; then
        LIBICQ2000_CONFIG=$libicq2000_config_exec_prefix/bin/libicq2000-config
     fi
  fi
  if test x$libicq2000_config_prefix != x ; then
     if test x${LIBICQ2000_CONFIG+set} != xset ; then
        LIBICQ2000_CONFIG=$libicq2000_config_prefix/bin/libicq2000-config
     fi
  fi

  AC_PATH_PROG(LIBICQ2000_CONFIG, libicq2000-config, no)
  min_libicq2000_version=ifelse([$1], ,0.3.0,$1)

  AC_MSG_CHECKING(for libicq2000 - version >= $min_libicq2000_version)
  AC_LANG_SAVE
  no_libicq2000=""
  if test "$LIBICQ2000_CONFIG" = "no" ; then
    no_libicq2000=yes
  else
    AC_LANG_CPLUSPLUS

    LIBICQ2000_CFLAGS=`$LIBICQ2000_CONFIG --cflags`
    LIBICQ2000_LIBS=`$LIBICQ2000_CONFIG --libs`
    LIBICQ2000_LIBFLAGS=`$LIBICQ2000_CONFIG --libs-only-L`
    LIBICQ2000_LIBADD=`$LIBICQ2000_CONFIG --libs-only-l`
    libicq2000_config_major_version=`$LIBICQ2000_CONFIG --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    libicq2000_config_minor_version=`$LIBICQ2000_CONFIG --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    libicq2000_config_micro_version=`$LIBICQ2000_CONFIG --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_libicq2000test" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $LIBICQ2000_CFLAGS"
      LIBS="$LIBS $LIBICQ2000_LIBS"
dnl
dnl Now check if the installed libicq2000 is sufficiently new. (Also sanity
dnl checks the results of libicq2000-config to some extent
dnl
      rm -f conf.libicq2000test
      AC_TRY_RUN([
#include <libicq2000/version.h>
#include <libicq2000/Client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.libicq2000test");

  /* HP/UX 0 (%@#!) writes to sscanf strings */
  tmp_version = strdup("$min_libicq2000_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_libicq2000_version");
     exit(1);
   }

  if ((libicq2000_major_version != $libicq2000_config_major_version) ||
      (libicq2000_minor_version != $libicq2000_config_minor_version) ||
      (libicq2000_micro_version != $libicq2000_config_micro_version))
    {
      printf("\n*** 'libicq2000-config --version' returned %d.%d.%d, but libicq2000 (%d.%d.%d)\n", 
             $libicq2000_config_major_version, $libicq2000_config_minor_version, $libicq2000_config_micro_version,
             libicq2000_major_version, libicq2000_minor_version, libicq2000_micro_version);
      printf ("*** was found! If libicq2000-config was correct, then it is best\n");
      printf ("*** to remove the old version of libicq2000. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If libicq2000-config was wrong, set the environment variable LIBICQ2000_CONFIG\n");
      printf("*** to point to the correct copy of libicq2000-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
  else if ((libicq2000_major_version != LIBICQ2000_MAJOR_VERSION) ||
	   (libicq2000_minor_version != LIBICQ2000_MINOR_VERSION) ||
           (libicq2000_micro_version != LIBICQ2000_MICRO_VERSION))
    {
      printf("*** libicq2000 header files (version %d.%d.%d) do not match\n",
	     LIBICQ2000_MAJOR_VERSION, LIBICQ2000_MINOR_VERSION, LIBICQ2000_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     libicq2000_major_version, libicq2000_minor_version, libicq2000_micro_version);
    }
  else
    {
      if ((libicq2000_major_version > major) ||
        ((libicq2000_major_version == major) && (libicq2000_minor_version > minor)) ||
        ((libicq2000_major_version == major) && (libicq2000_minor_version == minor) && (libicq2000_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of libicq2000 (%d.%d.%d) was found.\n",
               libicq2000_major_version, libicq2000_minor_version, libicq2000_micro_version);
        printf("*** You need a version of libicq2000 newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the libicq2000-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of libicq2000.\n");
      }
    }
  return 1;
}
],, no_libicq2000=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_libicq2000" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])
  else
     AC_MSG_RESULT(no)
     if test "$LIBICQ2000_CONFIG" = "no" ; then
       echo "*** The libicq2000-config script is installed but libicq2000 could not be found"
       echo "*** If libicq2000 was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the LIBICQ2000_CONFIG environment variable to the"
       echo "*** full path to libicq2000-config."
     else
       if test -f conf.libicq2000test ; then
        :
       else
          echo "*** Could not run libicq2000 test program, checking why..."
          CXXFLAGS="$CXXFLAGS $LIBICQ2000_CFLAGS"
          LIBS="$LIBS $LIBICQ2000_LIBS"
          AC_TRY_LINK([
#include <libicq2000/version.h>
#include <stdio.h>
],      [ return ((libicq2000_major_version) || (libicq2000_minor_version) || (libicq2000_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding libicq2000 or finding the"
          echo "*** wrong version of libicq2000. If it is not finding libicq2000,"
          echo "*** you'll need to set your LD_LIBRARY_PATH environment variable, or"
          echo "*** edit /etc/ld.so.conf to point to the installed location.  Also,"
          echo "*** make sure you have run ldconfig if that is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH" ],
        [ echo "*** The test program failed to compile or link. See the file config.log"
          echo "*** for the exact error that occured. This usually means libicq2000 was"
          echo "*** incorrectly installed or that you have moved libicq2000 since it"
          echo "*** was installed. In the latter case, you may want to edit the"
	  echo "*** libicq2000-config script: $LIBICQ2000_CONFIG" ])
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     LIBICQ2000_CFLAGS=""
     LIBICQ2000_LIBS=""
     LIBICQ2000_LIBFLAGS=""
     LIBICQ2000_LIBADD=""
     ifelse([$3], , :, [$3])
  fi
  AC_LANG_RESTORE
  AC_SUBST(LIBICQ2000_CFLAGS)
  AC_SUBST(LIBICQ2000_LIBS)
  AC_SUBST(LIBICQ2000_LIBFLAGS)
  AC_SUBST(LIBICQ2000_LIBADD)
  rm -f conf.libicq2000test
])

