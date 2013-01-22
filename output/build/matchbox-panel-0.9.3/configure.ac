AC_PREREQ(2.53)
AC_INIT([matchbox-panel], 0.9.3, [mallum@handhelds.org])
AC_CONFIG_SRCDIR([src/panel.c])

AM_INIT_AUTOMAKE()
AM_MAINTAINER_MODE
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_GNU_SOURCE
AC_PROG_CC
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([libintl.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([alarm getcwd gettimeofday memset mkdir select setenv setlocale strcasecmp strdup strpbrk strstr])

AC_ARG_ENABLE(dnotify,
  [  --enable-dnotify      enable dnotify support (Linux 2.4+ only).],
  enable_dnotify=$enableval, enable_dnotify=no)

AC_ARG_ENABLE(startup_notification,
  [  --enable-startup-notification   enable startup notification support],
     enable_startup_notification=$enableval, enable_startup_notification=no )

AC_ARG_ENABLE(nls,
  [  --enable-nls        enable Native Language Support ( gettext/libintl )],
   enable_nls=$enableval, enable_nls=no)

AC_ARG_ENABLE(small_icons,
  [  --enable-small-icons    enable small icons],
     enable_small_icons=$enableval, enable_small_icons=no )

AC_ARG_ENABLE(dnotify,
  [  --enable-dnotify      enable dnotify support (Linux 2.4+ only).],
  enable_dnotify=$enableval, enable_dnotify=no)

AC_ARG_ENABLE(debug,
  [  --enable-debug    enable debug ( verbose ) build],
     enable_debug=$enableval, enable_debug=no )

AC_ARG_ENABLE(acpi-linux,
  [  --enable-acpi-linux   Use Linux ACPI rather than APM for battery info],
     enable_acpi_linux=$enableval, enable_acpi_linux=no )

PKG_CHECK_MODULES(LIBMB, libmb >= 1.6,,
	         AC_MSG_ERROR([*** Required Matchbox Library (libmb) 1.6 not installed ***]))

dnl ------ Debug Build ------------------------------------------------------

if test x$enable_debug = xyes; then
      LIBMB_CFLAGS="$LIBMB_CFLAGS -DDEBUG"
fi

dnl ----- DNOTIFY ----------------------------------------------------------

if test x$enable_dnotify = xyes; then
## FIXME: need to actually check its present
  AC_DEFINE(USE_DNOTIFY, [1], [Has dnotify support])
fi

dnl ----- Startup Notification ---------------------------------------------


if test x$enable_startup_notification != xno; then
  PKG_CHECK_MODULES(SN, libstartup-notification-1.0, ,
			   AC_MSG_ERROR([*** Required Startup Notification Librays not installed ***]))

  AC_DEFINE(USE_LIBSN, [1], [Has StartupNotification Support])
fi

dnl ----- Linux ACPI -------------------------------------------------------

if test x$enable_acpi_linux = xyes; then
  AC_DEFINE(USE_ACPI_LINUX, [1], [Use /proc/acpi to get battery status])
fi

dnl ----- NLS Fun -----------------------------------------------------------

INTLIBS=""
MOFILES=""

if test x$enable_nls = xyes; then

  LINGUAS="en_GB es_ES fi_FI de cs fr_FR zh_TW"

  AC_CHECK_FUNC(gettext, [HAVEGETTEXT="yes"], 
	     AC_CHECK_LIB(intl, gettext, [INTLIBS="-lintl" HAVEGETTEXT="yes"],
		  	 INTLIBS="" ))

  AC_CHECK_PROG(XGETTEXT, xgettext, xgettext)
  AC_CHECK_PROG(MSGMERGE, msgmerge, msgmerge)
  AC_CHECK_PROG(MSGFMT, msgfmt, msgfmt)

  if test "$XGETTEXT" != ""; then 
    if $XGETTEXT --help 2>&1 | grep illegal >/dev/null ; then
        echo "xgettext isn't GNU version"
        XGETTEXT=""
    fi
  fi

  if test "$LINGUAS" != ""; then
    if test "$XGETTEXT" != "" -a "$HAVEGETTEXT" != ""; then
       PO=""
       if test "$LINGUAS" = ""; then
   	    ling=` (cd src/po; /bin/ls *.po) `
    	    for l in $ling; do
	        lcode=`basename $l .po`
	        LINGUAS="$LINGUAS $lcode"
           done
       fi
       AC_DEFINE(ENABLE_NLS, [1], [NLS Please])
       echo "xgettext and gettext() exist; will build i18n support for $LINGUAS"
    else
        LINGUAS=""
        PO=""
        echo "xgettext and libintl.a don't both exist; will not build i18n support"
    fi
  else
    INTLIBS=""
    MOFILES=""
    PO=""
  fi

  for lang in $LINGUAS; do
    MOFILES="$MOFILES $lang.mo"
  done

fi

AC_SUBST(INTLIBS)
AC_SUBST(MOFILES)

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


dnl ---- Png Support -------------------------------------------------------

if $PKG_CONFIG --libs libmb | grep png ; then
   found_png="yes"
else
   found_png="no"	
fi

AM_CONDITIONAL(WANT_PNGS, test x$found_png = xyes)  


dnl ---- Icons --------------------------------------------------------------

AM_CONDITIONAL(WANT_SMALL_ICONS, test x$enable_small_icons = xyes)  

dnl ----- DNOTIFY ----------------------------------------------------------

if test x$enable_dnotify = xyes; then
## FIXME: need to actually check its present
  AC_DEFINE(USE_DNOTIFY, [1], [Has dnotify support])
fi

AM_CONDITIONAL(WANT_SMALL_ICONS, test x$enable_small_icons = xyes)  

dnl ----- Specific Applet deps ----------------------------------------------

miniapm_enabled="no ( enable ACPI? )"

if test x$enable_acpi_linux = xyes; then
  miniapm_enabled="yes, with ACPI"
else
  AC_CHECK_HEADERS(apm.h, enable_linux_apm=yes, enable_linux_apm=no)
  if test x$enable_linux_apm = xyes; then
     miniapm_enabled="yes"
     AC_DEFINE(HAVE_APM_H, [1], [Have Linux APM headers])
  fi

  AC_CHECK_HEADERS(apmvar.h, enable_bsd_apm=yes, enable_bsd_apm=no)
  if test x$enable_bsd_apm = xyes; then
     AC_DEFINE(HAVE_APMVAR_H, [1], [Have BSD APM headers])
     miniapm_enabled="yes"
  fi
fi

AM_CONDITIONAL(WANT_APM, test x"$miniapm_enabled" = xyes)

AM_CONDITIONAL(WANT_ACPI, test x$enable_acpi_linux = xyes)


dnl ------ wireless checks --------------------------------------------------

wifi_enabled=no

AC_CHECK_HEADERS(iwlib.h, have_iwlib_h=yes, have_iwlib_h=no)
AC_CHECK_LIB(iw, iw_sockets_open, have_libiw=yes, have_libiw=yes)

if test x$have_iwlib_h = x"yes" && test x$have_libiw = x"yes"; then
	WIRELESS_LIBS="-liw"
	wifi_enabled=yes
fi
AC_SUBST(WIRELESS_LIBS)

AM_CONDITIONAL(WANT_WIFI, test x$wifi_enabled = x"yes")



dnl ------ GCC flags --------------------------------------------------------

if test "x$GCC" = "xyes"; then
        GCC_WARNINGS="-g -Wall -fno-strict-aliasing"
        LIBMB_CFLAGS="$GCC_WARNINGS $LIBMB_CFLAGS"
fi


dnl ------ Substitute in found libs, clags to Makefiles etc -----------------

AC_SUBST(LIBMB_CFLAGS)
AC_SUBST(LIBMB_LIBS)

AC_SUBST(SN_CFLAGS)
AC_SUBST(SN_LIBS)

AC_OUTPUT([
Makefile
src/Makefile  
applets/Makefile
applets/dotdesktop/Makefile
applets/icons/Makefile
applets/small-icons/Makefile
po/Makefile
])

dnl ==========================================================================
echo "
                    Matchbox-panel $VERSION
                  =========================

        prefix:                 ${prefix}
        source code location:   ${srcdir}
        compiler:               ${CC} 

        Building with Debug:                ${enable_debug}

        Building with Startup-Notification: ${enable_startup_notification}
	Building with NLS:                  ${enable_nls}
        Building with small icons:          ${enable_small_icons}
	Building with DNOTIFY:		    ${enable_dnotify}

        Building mb-applet-battery:         ${miniapm_enabled}
        Building mb-applet-wireless:        ${wifi_enabled}
"
