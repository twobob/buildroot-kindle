source_c = libfakekey.c

AM_CFLAGS = @FAKEKEY_CFLAGS@

libfakekey_la_SOURCES     = $(source_c)

lib_LTLIBRARIES = libfakekey.la

libfakekey_la_LDFLAGS = -version-info @LT_VERSION_INFO@
libfakekey_la_LIBADD = $(FAKEKEY_LIBS)
