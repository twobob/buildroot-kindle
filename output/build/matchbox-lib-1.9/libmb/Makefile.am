source_h = mb.h           \
           mbmenu.h       \
           mbtray.h       \
           mbdotdesktop.h \
           mbpixbuf.h     \
           mbutil.h       \
           mbexp.h        \
           mbconfig.h     \
           hash.h 

source_c = mbmenu.c       \
           mbtray.c       \
           mbdotdesktop.c \
           mbpixbuf.c     \
           mbutil.c       \
	   mbexp.c        \
           hash.c

DATADIR=$(datadir)

AM_CFLAGS = @GCC_WARNINGS@ @XLIBS_CFLAGS@ @PANGO_CFLAGS@ @PNG_CFLAGS@ @XSET_LIBS@ -DDATADIR=\"$(datadir)\"

lib_LTLIBRARIES      = libmb.la
libmb_la_SOURCES     = $(source_c) $(source_h)
libmb_la_LIBADD      = @XLIBS_LIBS@ @PANGO_LIBS@ @JPEG_LIBS@ @PNG_LIBS@ @XSET_CFLAGS@

# http://sources.redhat.com/autobook/autobook/autobook_91.html#SEC91
# current : revision : age

libmb_la_LDFLAGS = -version-info 1:9:0

libmbheadersdir = $(includedir)/libmb
libmbheaders_DATA = $(source_h)

clean-local:
	/bin/rm *.bb *.bbg *.da *.gcov .libs/*.da .libs/*.bbg || true
