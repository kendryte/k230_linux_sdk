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

VVCAM_MODULE_MAKE_OPTS += BR2_PACKAGE_VVCAM_DEF_SENSOR=$(BR2_PACKAGE_VVCAM_DEF_SENSOR)

$(eval $(kernel-module))
$(eval $(cmake-package))
