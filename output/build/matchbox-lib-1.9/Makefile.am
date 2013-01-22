SUBDIRS=libmb doc tests

EXTRA_DIST = libmb.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmb.pc

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`
