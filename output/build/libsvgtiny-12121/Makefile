# Component settings
COMPONENT := svgtiny
COMPONENT_VERSION := 0.0.1
# Default to a static library
COMPONENT_TYPE ?= lib-static

# Setup the tooling
include build/makefiles/Makefile.tools

TESTRUNNER := $(ECHO)

# Toolchain flags
WARNFLAGS := -Wall -W -Wundef -Wpointer-arith -Wcast-align \
	-Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes \
	-Wmissing-declarations -Wnested-externs -pedantic
# BeOS/Haiku/AmigaOS4 standard library headers create warnings
ifneq ($(TARGET),beos)
  ifneq ($(TARGET),AmigaOS)
    WARNFLAGS := $(WARNFLAGS) -Werror
  endif
endif
CFLAGS := -D_BSD_SOURCE -I$(CURDIR)/include/ \
	-I$(CURDIR)/src $(WARNFLAGS) $(CFLAGS)
ifneq ($(GCCVER),2)
  CFLAGS := $(CFLAGS) -std=c99
else
  # __inline__ is a GCCism
  CFLAGS := $(CFLAGS) -Dinline="__inline__"
endif

# LibXML2
ifneq ($(PKGCONFIG),)
  CFLAGS := $(CFLAGS) \
		$(shell $(PKGCONFIG) $(PKGCONFIGFLAGS) --cflags libxml-2.0)
  LDFLAGS := $(LDFLAGS) \
		$(shell $(PKGCONFIG) $(PKGCONFIGFLAGS) --libs libxml-2.0)
else
  ifeq ($(TARGET),beos)
    CFLAGS := $(CFLAGS) -I/boot/home/config/include/libxml2
  endif
  CFLAGS := $(CFLAGS) -I$(PREFIX)/include/libxml2
  LDFLAGS := $(CFLAGS) -lxml2
endif

include build/makefiles/Makefile.top

# Extra installation rules
I := /include
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/svgtiny.h
INSTALL_ITEMS := $(INSTALL_ITEMS) /lib/pkgconfig:lib$(COMPONENT).pc.in
INSTALL_ITEMS := $(INSTALL_ITEMS) /lib:$(OUTPUT)
