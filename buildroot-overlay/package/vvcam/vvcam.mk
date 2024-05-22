################################################################################
#
# vvcam
#
################################################################################

VVCAM_DIR_NAME := vvcam
VVCAM_APP_NAME := vvcam
VVCAM_SITE = $(realpath $(TOPDIR))"/package/vvcam"
VVCAM_SITE_METHOD = local

define VVCAM_INSTALL_TARGET_CMDS
	cp package/vvcam/isp_media_server $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
