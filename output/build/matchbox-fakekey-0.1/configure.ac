>AC_PREREQ(2.53)
AC_INIT([libfakekey], 0.1, [mallum@openedhand.com])
AC_CONFIG_SRCDIR([src/libfakekey.c])

AC_CONFIG_AUX_DIR(.)
AM_INIT_AUTOMAKE()
AM_CONFIG_HEADER(config.h)


# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_HEADER_STDC

dnl ------ libtool versioning -----------------------------------------------

LT_CURRENT=0
LT_REVISION=1
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
LT_AGE=0

LT_VERSION_INFO="$LT_CURRENT:$LT_REVISION:$LT_AGE"
AC_SUBST(LT_VERSION_INFO)

LT_CURRENT_MINUS_AGE=`expr $LT_CURRENT - $LT_AGE`
AC_SUBST(LT_CURRENT_MINUS_AGE)

dnl ------ Check for X Stuff ------------------------------------------------

PKG_CHECK_MODULES(X11, x11, [have_libx11pc="yes"], [have_libx11pc="no"])

if test $have_libx11pc = yes; then
   PKG_CHECK_MODULES(XLIBS, x11 xtst)

   FAKEKEY_LIBS="$XLIBS_LIBS"
   FAKEKEY_CFLAGS="$XLIBS_CFLAGS"

else

AC_PATH_XTRA

ALL_X_LIBS="$X_LIBS -lX11"

AC_CHECK_LIB(Xtst, XTestQueryExtension, XTEST_LIBS=-lXtst have_xtest="yes" , have_xtest="no", $ALL_X_LIBS)

if test "x$have_xtest" = "xno"; then
   AC_MSG_ERROR([Cannot find XTest extension library])
   exit 1
fi

FAKEKEY_CFLAGS="$XLIBS_CLAGS"
FAKEKEY_LIBS="$ALL_X_LIBS $XTEST_LIBS"

fi

dnl ------ Debug -----------------------------------------------------------

AC_ARG_ENABLE(debug,
  [  --enable-debug          enable debug ( verbose ) build],
     enable_debug=$enableval, enable_debug=no )

if test x$enable_debug != xno; then
   AC_DEFINE_UNQUOTED(WANT_DEBUG, 1, [Make a debug (Verbose) Build])
fi

dnl ------ Doxygen docs ----------------------------------------------------

AC_ARG_ENABLE(doxygen-docs,     
  [  --enable-doxygen-docs   build DOXYGEN API documentation (requires Doxygen)], 
     enable_doxygen_docs=$enableval,enable_doxygen_docs=no)

if test x$enable_doxygen_docs = xyes ; then

  AC_PATH_PROG(DOXYGEN, doxygen, no)

  AC_MSG_CHECKING([whether to build Doxygen documentation])

  if test x$DOXYGEN = xno ; then
    have_doxygen=no
  else
    have_doxygen=yes
  fi

  if test x$have_doxygen = xno; then
    AC_MSG_ERROR([Building Doxygen docs explicitly required, but Doxygen not found])
  fi

  AC_MSG_RESULT(yes)

fi

AM_CONDITIONAL(HAVE_DOXYGEN, test x$enable_doxygen_docs = xyes)	


dnl ------ GCC flags --------------------------------------------------------

if test "x$GCC" = "xyes"; then
        GCC_WARNINGS="-g -Wall -fno-strict-aliasing"
        FAKEKEY_CFLAGS="$GCC_WARNINGS $FAKEKEY_CFLAGS"
fi

dnl -------------------------------------------------------------------------

AC_SUBST(FAKEKEY_CFLAGS)
AC_SUBST(FAKEKEY_LIBS)


AC_OUTPUT([
Makefile
libfakekey.pc
fakekey/Makefile
src/Makefile
doc/Doxyfile
doc/Makefile
tests/Makefile

])

dnl ==========================================================================
echo "
                    LibFakeKey $VERSION
                  ======================

        prefix:                 ${prefix}
        source code location:   ${srcdir}

        Building with Debug:                ${enable_debug}
        Building with API Documentation:    ${enable_doxygen_docs}

"
