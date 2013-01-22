PREFIXDIR  = $(prefix)
PKGDATADIR = $(datadir)/matchbox
DATADIR    = $(datadir)

INCLUDES = -DDATADIR=\"$(DATADIR)\" -DPKGDATADIR=\"$(PKGDATADIR)\" -DPREFIX=\"$(PREFIXDIR)\" $(LIBMB_CFLAGS)

bin_PROGRAMS = matchbox-panel

matchbox_panel_LDADD = $(LIBMB_LIBS)

matchbox_panel_SOURCES =                                             \
  panel.c  panel_app.c  panel_menu.c  panel_util.c  session.c msg.c  \
  panel.h  panel_app.h  panel_menu.h  panel_util.h  session.h msg.h  


