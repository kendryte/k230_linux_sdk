################################################################################
#
# bcmdhd
#
################################################################################

AIC8800_SITE = $(realpath $(TOPDIR))"/package/aic8800"
AIC8800_SITE_METHOD = local
AIC8800_INSTALL_STAGING = YES
AIC8800_INSTALL_TARGET = YES
AIC8800_SUPPORTS_IN_SOURCE_BUILD = NO

# AIC8800_MODULE_MAKE_OPTS += CONFIG_AIC8800=m
# AIC8800_MODULE_MAKE_OPTS += CONFIG_AIC8800_FW_PATH="/etc/firmware/fw_bcmdhd.bin"
# AIC8800_MODULE_MAKE_OPTS += CONFIG_AIC8800_NVRAM_PATH="/etc/firmware/nvram.txt"
# AIC8800_MODULE_MAKE_OPTS += CONFIG_AIC8800_SDIO=y
# AIC8800_MODULE_MAKE_OPTS += CONFIG_AIC8800_SDIO_IRQ=y


define AIC8800_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/firmware/
	cp -rf $(@D)/fw/* $(TARGET_DIR)/lib/firmware/
endef

$(eval $(kernel-module))
$(eval $(generic-package))
