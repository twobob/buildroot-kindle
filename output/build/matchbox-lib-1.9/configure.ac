AC_PREREQ(2.53)
AC_INIT([libmatchbox], 1.9, [mallum@handhelds.org])
AC_CONFIG_SRCDIR([libmb/mbtray.c])

AC_CONFIG_AUX_DIR(.)
AM_INIT_AUTOMAKE()
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_GNU_SOURCE
AC_PROG_CC
AC_PROG_LIBTOOL

SUPPORTS_PNG=0
SUPPORTS_JPEG=0
SUPPORTS_XFT=0
SUPPORTS_PANGO=0
SUPPORTS_XSETTINGS=0

dnl ----- Args -------------------------------------------------------------

AC_ARG_ENABLE(xft,
  [  --disable-xft           disable xft (anti-aliased) support, [default=no]],
  enable_xft=$enableval, enable_xft=yes)

AC_ARG_ENABLE(pango,
  [  --enable-pango          enable Pango support.],
  enable_pango=$enableval, enable_pango=no)

AC_ARG_ENABLE(jpeg,
  [  --enable-jpeg           enable jpeg image support.],
  enable_jpeg=$enableval, enable_jpeg=no)

AC_ARG_ENABLE(png,
  [  --disable-png           disable png support [default=no]],
     enable_png=$enableval, enable_png=yes )

AC_ARG_ENABLE(doxygen-docs,     
  [  --enable-doxygen-docs   build DOXYGEN API documentation (requires Doxygen)], 
     enable_doxygen_docs=$enableval,enable_doxygen_docs=no)

AC_ARG_ENABLE(xsettings,
  [  --enable-xsettings      enable xsettings client support],
     enable_xsettings=$enableval, enable_xsettings=no )

AC_ARG_WITH(xsettings-includes,    
  [  --with-xsettings-includes=DIR Use xsettings-client includes in DIR],
 
	   xsettings_includes=$withval, xsettings_includes=yes)

AC_ARG_WITH(xsettings-lib,         
  [  --with-xsettings-lib=DIR      Use xsettings-client library in DIR], 
	   xsettings_lib=$withval, xsettings_lib=yes)

AC_ARG_ENABLE(debug,
  [  --enable-debug          enable debug ( verbose ) build],
     enable_debug=$enableval, enable_debug=no )

AC_ARG_ENABLE(gcov,
  [  --enable-gcov           enable gcov coverage ( testing only ) build],
     enable_gcov=$enableval, enable_gcov=no )

AC_ARG_ENABLE(gprof,
  [  --enable-gprof          enable gprof profiler ( testing only ) build],
     enable_gprof=$enableval, enable_gprof=no )

AC_ARG_ENABLE(unit-tests,
  [  --enable-unit-tests          enable unit tests ( Requires check )],
     enable_unit_tests=$enableval, enable_unit_tests=no )



if test x$enable_pango = xyes; then
   enable_xft=no
fi

dnl ------ Check for X Stuff ------------------------------------------------

PKG_CHECK_MODULES(X11, x11, [have_libx11pc="yes"], [have_libx11pc="no"])

if test $have_libx11pc = yes; then
   xft_pkg=
   if test x$enable_xft != xno; then
      xft_pkg=xft
      SUPPORTS_XFT=1
      AC_DEFINE(USE_XFT, [1], [Use Xft])	
   fi
# XXX : xau is missing from x11.pc - workaround is too add here 
   PKG_CHECK_MODULES(XLIBS, x11 xext  $xft_pkg)
   XLIBS_REQUIRED="x11 xext"
else

AC_PATH_XTRA
if test x"$no_x" = x"yes"; then
    AC_MSG_ERROR([*** Required X11 Headers and libraries not found.***])
fi

XFT_LIBS=
XFT_CFLAGS=
 
# FIXME: Should check for shm 

if test x$enable_xft != xno; then
  AC_MSG_CHECKING([for xft])
  if test x$PKG_CONFIG != xno && $PKG_CONFIG --exists xft; then 
    XFT_CFLAGS="`pkg-config --cflags xft`"
    XFT_LIBS="`pkg-config --libs xft`"
    AC_DEFINE(USE_XFT, [1], [Use Xft])
    SUPPORTS_XFT=1
    AC_MSG_RESULT(yes)
  else

    AC_PATH_PROG(XFT_CONFIG, xft-config, no)
    
    if test x$XFT_CONFIG = xno; then
      AC_MSG_RESULT(no)
      AC_MSG_WARN([*** Cannot find Xft librarys, disabling support])
      enable_xft = no
    else
      XFT_CFLAGS="`xft-config --cflags`"
      XFT_LIBS="`xft-config --libs`"
      AC_DEFINE(USE_XFT, [1], [Use Xft])
      SUPPORTS_XFT=1    
      AC_MSG_RESULT(yes)
    fi
  fi
fi

XLIBS_CFLAGS="$XLIBS_CLAGS $XFT_CFLAGS"
XLIBS_LIBS="$X_LIBS $XFT_LIBS -lX11 -lXext"

MB_EXTRA_LIBS="$MB_EXTRA_LIBS $XLIBS_LIBS"

fi

# do this here for freetype include
MB_EXTRA_CFLAGS="$MB_EXTRA_CFLAGS $XLIBS_CFLAGS"


dnl ------ Check for Pango ---------------------------------------------------

if test x$enable_pango != xno; then
  PKG_CHECK_MODULES(PANGO, pango pangoxft, [have_pango="yes"], [have_pango="no"])
  if test x$have_pango=xyes; then
    AC_DEFINE(USE_PANGO, [1], [Use Pango])
    SUPPORTS_PANGO=1
    PANGO_REQUIRED="pango pangoxft"
  else
    AC_MSG_WARN([*** Cannot find pango, disabling support])
    enable_pango=no
  fi
fi


dnl ------ Check for PNG ---------------------------------------------------

if test x$enable_png != xno; then
  AC_MSG_CHECKING(for libpng12)
  if test x$PKG_CONFIG != xno && $PKG_CONFIG --exists libpng12; then 
        AC_MSG_RESULT(yes)
        PNG_LIBS=`$PKG_CONFIG --libs libpng12`
	PNG_CFLAGS=`$PKG_CONFIG --cflags libpng12`
        AC_DEFINE(USE_PNG, [1], [Use Png])
	SUPPORTS_PNG=1
	PNG_REQUIRED="libpng12"
  else
	AC_MSG_RESULT(no)
        # AC_CHECK_HEADERS(png.h, [ have_png_h="yes" ], [ have_png_h="no" ] )
        AC_CHECK_LIB([png], [png_create_read_struct], [have_png="yes"], [have_png="no"])

        if test x$have_png=xyes && test x$have_png_h=xyes; then 
            AC_DEFINE(USE_PNG, [1], [Use Png])
	    SUPPORTS_PNG=1
            PNG_LIBS="-lpng -lz"
            MB_EXTRA_LIBS="$MB_EXTRA_LIBS $XLIBS_LIBS $PNG_LIBS"
        else
	    AC_MSG_WARN([*** Cannot find PNG, disabling support])
            enable_png=no
	fi
  fi
fi


dnl ------ Check for JPEG ---------------------------------------------------

if test x$enable_jpeg != xno; then
  # AC_CHECK_HEADERS(jpeglib.h, [ have_jpg_h="yes" ], [ have_jpg_h="no" ] )
  AC_CHECK_LIB([jpeg], [jpeg_read_header], [have_jpg="yes"], [have_jpg="no"])

  if test x$have_jpg=xyes && test x$have_jpg_h=xyes; then 
        AC_DEFINE(USE_JPG, [1], [Use JPEG])
	SUPPORTS_JPEG=1
	JPEG_LIBS="-ljpeg"
        MB_EXTRA_LIBS="$MB_EXTRA_LIBS -ljpeg"
  else
	AC_MSG_WARN([*** Cannot find libjpeg, disabling support])
        enable_jpeg=no
  fi
fi


dnl ------ Check for XSettings ----------------------------------------------

if test x$enable_xsettings != xno; then

  AC_MSG_CHECKING([for xsettings])

  case "$xsettings_includes" in
    yes|no)
	XSET_CFLAGS=""
	;;
    *)
	XSET_CFLAGS="-I$xsettings_includes"
	;;
  esac
	
  case "$xsettings_lib" in
    yes)
	XSET_LIBS="-lXsettings-client"
	;;
    *)
	XSET_LIBS="-L$xsettings_lib -lXsettings-client"
	;;
   esac

   xsetsaved_CPPFLAGS="$CPPFLAGS"
   CPPFLAGS="$CPPFLAGS $XSET_CFLAGS $XLIBS_CFLAGS"
   xsetsaved_LIBS="$LIBS"
   LIBS="$LIBS $XSET_LIBS $XLIBS_LIBS"

   AC_CHECK_HEADER(xsettings-client.h, [ have_xset_h="yes" ], [ have_xset_h="no" ] )

   if test x$have_xset_h = xno; then
      AC_MSG_ERROR([cannot find Xsettings-client headers])
   fi

   AC_CHECK_LIB([Xsettings-client], [xsettings_client_new], 
                [have_xset_libs="yes"], [ have_xset_libs="no"])

   if test x$have_xset_libs = x"no"; then	
      AC_MSG_ERROR([cannot find Xsettings-client headers])
   fi
   	
   CPPFLAGS="$saved_CPPFLAGS"
   LIBS="$saved_LIBS"

   MB_EXTRA_LIBS="$MB_EXTRA_LIBS $XSET_LIBS"	
   MB_EXTRA_CFLAGS="$MB_EXTRA_CFLAGS $XSET_CFLAGS"	

   AC_DEFINE(USE_XSETTINGS, [1], [Use XSettings Client])

   AC_MSG_RESULT([yes])
fi


dnl ------ Debug -----------------------------------------------------------

if test x$enable_debug != xno; then
  XLIBS_CFLAGS="-DDEBUG $XLIBS_CFLAGS"	
fi


dnl ------ Doxygen docs ----------------------------------------------------

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

   if test x$enable_gprof = xyes; then
         GCC_WARNINGS="$GCC_WARNINGS -pg"
	 AC_MSG_RESULT([Enabling gprof Support.])	
   fi	 

   if test x$enable_gcov = xyes; then
         GCC_WARNINGS="$GCC_WARNINGS -fprofile-arcs -ftest-coverage"
	 AC_MSG_RESULT([Enabling gcov Support.])	
   fi	 

   GCC_WARNINGS="-g -Wall -fno-strict-aliasing $GCC_WARNINGS"

else
   if test x$enable_gprof = xyes; then
      AC_MSG_WARN([*** Need gcc compiler for gprof support])
   fi

   if test x$enable_gcov = xyes; then
      AC_MSG_WARN([*** Need gcc compiler for gcov support])
   fi
fi

dnl ------ 'check' for unit tests -------------------------------------------

have_check=no

if test x$enable_unit_tests = xyes; then
  AM_PATH_CHECK(,have_check=yes, have_check=no)
fi

AM_CONDITIONAL(HAVE_CHECK, test x$have_check = xyes)

dnl ------ Substitute in found libs, clags to Makefiles etc -----------------

AC_SUBST(XLIBS_CFLAGS)
AC_SUBST(XLIBS_LIBS)
AC_SUBST(PANGO_CFLAGS)
AC_SUBST(PANGO_LIBS)
AC_SUBST(PNG_CFLAGS)
AC_SUBST(PNG_LIBS)
AC_SUBST(JPEG_LIBS)
AC_SUBST(GCC_WARNINGS)

AC_SUBST(XSET_LIBS)
AC_SUBST(XSET_CFLAGS)

AC_SUBST(MB_EXTRA_LIBS)
AC_SUBST(MB_EXTRA_CFLAGS)
AC_SUBST(XLIBS_REQUIRED)
AC_SUBST(PANGO_REQUIRED)
AC_SUBST(PNG_REQUIRED)

dnl ------ Below used for mbconfig.h ----------------------------------------

AC_SUBST(SUPPORTS_PNG)
AC_SUBST(SUPPORTS_JPEG)
AC_SUBST(SUPPORTS_XFT)
AC_SUBST(SUPPORTS_PANGO)

dnl ------ 'Built in' checks ------------------------------------------------ 

AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([langinfo.h locale.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_STRCOLL
AC_CHECK_FUNCS([memset select setlocale strcasecmp strchr strdup strncasecmp strstr])

AC_C_BIGENDIAN

AC_OUTPUT([
Makefile
libmb.pc
libmb/Makefile
libmb/mbconfig.h
doc/Doxyfile
doc/Makefile
tests/Makefile
tests/menu/Makefile
])

dnl ==========================================================================
echo "
                    LibMatchbox $VERSION
                  ======================

        prefix:                 ${prefix}
        source code location:   ${srcdir}

        Building with Debug:                ${enable_debug}
        Building with gcov:                 ${enable_gcov}
        Building with gprof:                ${enable_gprof}
	Building unit tests:                ${have_check}

        Building with Xft:                  ${enable_xft}
        Building with Pango:                ${enable_pango}
        Building with PNG:                  ${enable_png}
        Building with JPG:                  ${enable_jpeg}
        Building with XSettings:            ${enable_xsettings}
        Building with API Documentation:    ${enable_doxygen_docs}

"
