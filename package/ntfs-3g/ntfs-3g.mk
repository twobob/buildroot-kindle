#############################################################
#
# ntfs-3g
#
#############################################################

NTFS_3G_VERSION = 2012.1.15
NTFS_3G_SOURCE = ntfs-3g_ntfsprogs-$(NTFS_3G_VERSION).tgz
NTFS_3G_SITE = http://tuxera.com/opensource
NTFS_3G_CONF_OPT = --disable-ldconfig
NTFS_3G_INSTALL_STAGING = YES
NTFS_3G_DEPENDENCIES = host-pkg-config

ifeq ($(BR2_PACKAGE_LIBFUSE),y)
	NTFS_3G_CONF_OPT += --with-fuse=external
	NTFS_3G_DEPENDENCIES += libfuse
endif

ifeq ($(BR2_PACKAGE_UTIL_LINUX_LIBUUID),y)
	NTFS_3G_DEPENDENCIES += util-linux
endif

ifeq ($(BR2_PACKAGE_NTFS_3G_ENCRYPTED),y)
	NTFS_3G_CONF_ENV += LIBGCRYPT_CONFIG=$(STAGING_DIR)/usr/bin/libgcrypt-config
	NTFS_3G_CONF_OPT += --enable-crypto
	NTFS_3G_DEPENDENCIES += gnutls
endif

ifneq ($(BR2_PACKAGE_NTFS_3G_NTFSPROGS),y)
	NTFS_3G_CONF_OPT += --disable-ntfsprogs
endif

$(eval $(autotools-package))
