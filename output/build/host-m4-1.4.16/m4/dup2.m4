#serial 12
dnl Copyright (C) 2002, 2005, 2007, 2009-2011 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_DUP2],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_CHECK_FUNCS_ONCE([dup2 fcntl])
  if test $ac_cv_func_dup2 = no; then
    HAVE_DUP2=0
    AC_LIBOBJ([dup2])
  else
    AC_CACHE_CHECK([whether dup2 works], [gl_cv_func_dup2_works],
      [AC_RUN_IFELSE([
         AC_LANG_PROGRAM([[#include <unistd.h>
#include <fcntl.h>
#include <errno.h>]],
           [int result = 0;
#if HAVE_FCNTL
            if (fcntl (1, F_SETFD, FD_CLOEXEC) == -1)
              result |= 1;
#endif HAVE_FCNTL
            if (dup2 (1, 1) == 0)
              result |= 2;
#if HAVE_FCNTL
            if (fcntl (1, F_GETFD) != FD_CLOEXEC)
              result |= 4;
#endif
            close (0);
            if (dup2 (0, 0) != -1)
              result |= 8;
            /* Many gnulib modules require POSIX conformance of EBADF.  */
            if (dup2 (2, 1000000) == -1 && errno != EBADF)
              result |= 16;
            return result;
           ])
        ],
        [gl_cv_func_dup2_works=yes], [gl_cv_func_dup2_works=no],
        [case "$host_os" in
           mingw*) # on this platform, dup2 always returns 0 for success
             gl_cv_func_dup2_works=no;;
           cygwin*) # on cygwin 1.5.x, dup2(1,1) returns 0
             gl_cv_func_dup2_works=no;;
           linux*) # On linux between 2008-07-27 and 2009-05-11, dup2 of a
                   # closed fd may yield -EBADF instead of -1 / errno=EBADF.
             gl_cv_func_dup2_works=no;;
           freebsd*) # on FreeBSD 6.1, dup2(1,1000000) gives EMFILE, not EBADF.
             gl_cv_func_dup2_works=no;;
           haiku*) # on Haiku alpha 2, dup2(1, 1) resets FD_CLOEXEC.
             gl_cv_func_dup2_works=no;;
           *) gl_cv_func_dup2_works=yes;;
         esac])
      ])
    if test "$gl_cv_func_dup2_works" = no; then
      gl_REPLACE_DUP2
    fi
  fi
])

AC_DEFUN([gl_REPLACE_DUP2],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  if test $ac_cv_func_dup2 = yes; then
    REPLACE_DUP2=1
  fi
  AC_LIBOBJ([dup2])
])
