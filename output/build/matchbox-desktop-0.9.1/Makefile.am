SUBDIRS = src modules data

EXTRA_DIST = matchbox-desktop.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = matchbox-desktop.pc

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`
