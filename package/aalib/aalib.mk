#############################################################
#
# aalib
#
#############################################################
AALIB_VERSION = 1.4rc4
AALIB_SOURCE = aalib-$(AALIB_VERSION).tar.gz
AALIB_SITE = http://prdownloads.sourceforge.net/aa-project
AALIB_INSTALL_STAGING = YES
AALIB_INSTALL_TARGET = YES
AALIB_AUTORECONF = YES

AALIB_CONF_OPT = --enable-shared --with-x11-driver=no --exec-prefix=/mnt/us --prefix=/mnt/us 

ifeq ($(BR2_PACKAGE_SLANG),y)
AALIB_CONF_OPT+=--with-slang-driver=yes
AALIB_DEPENDENCIES += slang
else
AALIB_CONF_OPT+=--with-slang-driver=no
endif

ifeq ($(BR2_PACKAGE_CURSES),y)
AALIB_CONF_OPT+=--with-curses-driver=yes
AALIB_DEPENDENCIES += curses
else
AALIB_CONF_OPT+=--with-slang-driver=no
endif

$(eval $(autotools-package))
