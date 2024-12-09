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


define AIC8800_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/firmware/
	cp -rf $(@D)/fw/* $(TARGET_DIR)/lib/firmware/
	cp $(@D)/S100aic8800   $(TARGET_DIR)/etc/init.d/
endef

$(eval $(kernel-module))
$(eval $(generic-package))
