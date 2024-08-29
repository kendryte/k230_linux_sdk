################################################################################
#
# display
#
################################################################################

DISPLAY_DIR_NAME := display
DISPLAY_APP_NAME := display
DISPLAY_SITE = $(realpath $(TOPDIR))"/package/display"
DISPLAY_SITE_METHOD = local
DISPLAY_INSTALL_STAGING = YES
DISPLAY_INSTALL_TARGET = YES
DISPLAY_DEPENDENCIES = libdrm

$(eval $(cmake-package))
