AC_PREREQ(2.53)
AC_INIT([matchbox-desktop], 0.9.1, [mallum@handhelds.org])
AC_CONFIG_SRCDIR([src/mbdesktop.c])
AC_CONFIG_AUX_DIR(.)

AM_INIT_AUTOMAKE()
AM_MAINTAINER_MODE
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_GNU_SOURCE
AC_PROG_CC
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_VOLATILE

# Checks for library functions.
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([alarm memset mkdir strchr strdup])

AC_ARG_ENABLE(startup_notification,
  [  --enable-startup-notification   enable startup notification support],
     enable_startup_notification=$enableval, enable_startup_notification=no )

AC_ARG_ENABLE(dnotify,
  [  --enable-dnotify      enable dnotify support (Linux 2.4+ only).],
  enable_dnotify=$enableval, enable_dnotify=no)

AC_ARG_ENABLE(debug,
  [  --enable-debug    enable debug ( verbose ) build],
     enable_debug=$enableval, enable_debug=no )

PKG_CHECK_MODULES(LIBMB, libmb >= 1.5,,
	         AC_MSG_ERROR([*** Required Matchbox Library (libmb) not installed ***]))

dnl ------ Crappy check for module loader -----------------------------------

# FIXME: Improve this for BSD's etc

AC_CHECK_HEADERS(dlfcn.h, have_dlstuff=yes, have_dlstuff=no)

if test x$havedlstuff = xno; then
   AC_MSG_ERROR([*** Required dlfcn.h not found])
fi


dnl ------ Debug Build ------------------------------------------------------

if test x$enable_debug = xyes; then
      LIBMB_CFLAGS="$LIBMB_CFLAGS -DDEBUG"
fi

dnl ----- DNOTIFY ----------------------------------------------------------

if test x$enable_dnotify = xyes; then
## FIXME: need to actually check its present
  AC_DEFINE(USE_DNOTIFY, [1], [Has dnotify support])
fi

dnl ----- Xsettings ---------------------------------------------------------

AC_MSG_CHECKING([for libmb xsettings support])

if $PKG_CONFIG --libs libmb | grep -i xsettings ; then
     mb_have_xsettings="yes"
     AC_DEFINE(USE_XSETTINGS, [1], [Use XSettings])
     AC_MSG_RESULT([yes])
else
     mb_have_xsettings="no"	
     AC_MSG_RESULT([no])
fi

dnl ----- Startup Notification ---------------------------------------------


if test x$enable_startup_notification != xno; then
  PKG_CHECK_MODULES(SN, libstartup-notification-1.0, ,
			   AC_MSG_ERROR([*** Required Startup Notification Librays not installed ***]))

  AC_DEFINE(USE_LIBSN, [1], [Has StartupNotification Support])
fi


dnl ------ GCC flags --------------------------------------------------------

if test "x$GCC" = "xyes"; then
        GCC_WARNINGS="-g -Wall -fno-strict-aliasing"
fi


dnl ------ Substitute in found libs, clags to Makefiles etc -----------------

if test "x$prefix" = xNONE; then
   prefix="${ac_default_prefix}"
fi
if test "x$exec_prefix" = xNONE; then
   exec_prefix='${prefix}'
fi

MBDESKTOP_PLUGIN_DIR=$libdir/matchbox/desktop
eval MBDESKTOP_PLUGIN_DIR=`eval echo "$MBDESKTOP_PLUGIN_DIR"`

AC_SUBST(MBDESKTOP_PLUGIN_DIR)
AC_SUBST(LIBMB_CFLAGS)
AC_SUBST(LIBMB_LIBS)
AC_SUBST(GCC_WARNINGS)

AC_SUBST(SN_CFLAGS)
AC_SUBST(SN_LIBS)

AC_OUTPUT([
Makefile
matchbox-desktop.pc
src/Makefile
modules/Makefile
data/Makefile
data/mbdesktop_modules
])

dnl ==========================================================================
echo "
                    Matchbox-desktop $VERSION
                  =========================

        Prefix:                 ${prefix}
	Plug-in location        ${MBDESKTOP_PLUGIN_DIR}
        Source code location:   ${srcdir}
        Compiler:               ${CC} 


        Building with Debug:                ${enable_debug}

        Building with Startup-Notification: ${enable_startup_notification}
	Building with DNOTIFY:		    ${enable_dnotify}

"