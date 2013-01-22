SUBDIRS=fakekey src tests doc 

EXTRA_DIST = libfakekey.pc.in 

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA = libfakekey.pc

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`



