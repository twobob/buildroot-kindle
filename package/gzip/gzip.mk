#############################################################
#
# gzip
#
#############################################################

GZIP_VERSION = 1.5
GZIP_SITE = $(BR2_GNU_MIRROR)/gzip

$(eval $(autotools-package))
