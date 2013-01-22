INCLUDES = \
	-I$(top_srcdir)/include	\
	@DIRECTFB_CFLAGS@

lib_LTLIBRARIES = libdivine.la

libdivine_la_SOURCES = \
	divine.c

libdivine_la_LDFLAGS = \
        -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)	\
        -release $(LT_RELEASE)
