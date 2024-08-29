################################################################################
#
# vvcam
#
################################################################################

VVCAM_DIR_NAME := vvcam
VVCAM_APP_NAME := vvcam
VVCAM_SITE = $(realpath $(TOPDIR))"/package/vvcam"
VVCAM_SITE_METHOD = local
VVCAM_INSTALL_STAGING = YES
VVCAM_INSTALL_TARGET = YES
VVCAM_DEPENDENCIES = mxml libmicrohttpd display
VVCAM_SUPPORTS_IN_SOURCE_BUILD = NO

$(eval $(kernel-module))
$(eval $(cmake-package))
