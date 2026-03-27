################################################################################
#
# nonai2d
#
################################################################################

NONAI2D_SITE = $(realpath $(TOPDIR))"/package/nonai2d/src"
NONAI2D_SITE_METHOD = local
NONAI2D_INSTALL_STAGING = YES
NONAI2D_INSTALL_TARGET = YES
NONAI2D_SUPPORTS_IN_SOURCE_BUILD = NO

define NONAI2D_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0644 $(@D)/nonai2d-v4l2-controls.h $(STAGING_DIR)/usr/include/nonai2d-v4l2-controls.h
endef

$(eval $(kernel-module))
$(eval $(generic-package))
