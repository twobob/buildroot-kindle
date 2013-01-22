#!/bin/sh

# This script does all the magic calls to automake/autoconf and
# friends that are needed to configure a cvs checkout. You need a
# couple of extra development tools to run this script successfully.
#
# If you are compiling from a released tarball you don't need these
# tools and you shouldn't use this script.  Just call ./configure
# directly.


PROJECT="DirectFB"
TEST_TYPE=-f
FILE=include/directfb.h

LIBTOOL_REQUIRED_VERSION=1.3.4
AUTOCONF_REQUIRED_VERSION=2.13
AUTOMAKE_REQUIRED_VERSION=1.4


srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.
ORIGDIR=`pwd`
cd $srcdir

make_version ()
{
    major=`echo $1 | cut -d '.' -f 1`
    minor=`echo $1 | cut -d '.' -f 2`
    micro=`echo $1 | cut -d '.' -f 3`

    expr $major \* 65536 + $minor \* 256 + $micro
}

check_version ()
{
    ver=`make_version $1.0.0`
    req=`make_version $2.0.0`

    if test $ver -ge $req; then
	echo "yes (version $1)"
    else
	echo "Too old (found version $1)!"
	DIE=1
    fi
}

echo
echo "I am testing that you have the required versions of libtool, autoconf," 
echo "and automake."
echo

DIE=0

echo -n "checking for libtool >= $LIBTOOL_REQUIRED_VERSION ... "
if (libtoolize --version) < /dev/null > /dev/null 2>&1; then
    VER=`libtoolize --version \
         | grep libtool | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $LIBTOOL_REQUIRED_VERSION
else
    echo
    echo "  You must have libtool installed to compile $PROJECT."
    echo "  Install the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1;
fi

echo -n "checking for autoconf >= $AUTOCONF_REQUIRED_VERSION ... "
if (autoconf --version) < /dev/null > /dev/null 2>&1; then
    VER=`autoconf --version \
         | head -n1 | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOCONF_REQUIRED_VERSION
else
    echo
    echo "  You must have autoconf installed to compile $PROJECT."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1;
fi

echo -n "checking for automake >= $AUTOMAKE_REQUIRED_VERSION ... "
if (automake --version) < /dev/null > /dev/null 2>&1; then
    VER=`automake --version \
         | grep automake | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOMAKE_REQUIRED_VERSION
else
    echo
    echo "  You must have automake installed to compile $PROJECT."
    echo "  Get ftp://ftp.cygnus.com/pub/home/tromey/automake-1.4p1.tar.gz"
    echo "  (or a newer version if it is available)"
    DIE=1
fi

if test "$DIE" -eq 1; then
    echo
    echo "Please install/upgrade the missing tools and call me again."
    echo	
    exit 1
fi


test $TEST_TYPE $FILE || {
    echo
    echo "You must run this script in the top-level $PROJECT directory."
    echo
    exit 1
}


if test -z "$*"; then
    echo
    echo "I am going to run ./configure with no arguments - if you wish "
    echo "to pass any to it, please specify them on the $0 command line."
    echo
fi


case $CC in
    *xlc | *xlc\ * | *lcc | *lcc\ *)
	am_opt=--include-deps
    ;;
esac

echo
echo
echo Running aclocal ...
aclocal -I m4 $ACLOCAL_FLAGS

echo Running libtoolize ...
libtoolize --automake

echo Running autoconf ...
autoconf

# optionally feature autoheader

(autoheader --version)  < /dev/null > /dev/null 2>&1 && echo Running autoheader... && autoheader

echo Running automake ...
automake -Wno-portability --add-missing $am_opt

cd $ORIGDIR

echo Running configure --enable-maintainer-mode "$@" ...
$srcdir/configure --enable-maintainer-mode "$@" || exit 1

echo "Now type 'make' to compile $PROJECT."
