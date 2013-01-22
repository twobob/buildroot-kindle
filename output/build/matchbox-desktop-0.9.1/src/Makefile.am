PREFIXDIR  = $(prefix)
PKGDATADIR = $(pkgdatadir)
DATADIR    = $(datadir)
MBCONFDIR  = $(sysconfdir)/matchbox

INCLUDES = $(GCC_WARNINGS) -DDATADIR=\"$(DATADIR)\" -DPKGDATADIR=\"$(PKGDATADIR)\" -DPREFIX=\"$(PREFIXDIR)\" -DMBCONFDIR=\"$(MBCONFDIR)\" $(LIBMB_CFLAGS) 

bin_PROGRAMS = matchbox-desktop

matchbox_desktop_LDADD = $(LIBMB_LIBS)  $(LIBADD_DL) -lz
matchbox_desktop_LDFLAGS = -export-dynamic

matchbox_desktop_SOURCES = \
 mbdesktop.c mbdesktop_item.c mbdesktop_view.c mbdesktop_module.c md5.c md5.h \
 mbdesktop.h mbdesktop_item.h mbdesktop_view.h mbdesktop_module.h \
 mbdesktop_win_plugin.c mbdesktop_win_plugin.h


mbdesktopheadersdir = $(includedir)/matchbox-desktop
mbdesktopheaders_DATA = \
  mbdesktop.h mbdesktop_item.h mbdesktop_view.h mbdesktop_module.h
