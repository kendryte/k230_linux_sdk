################################################################################
#
# bcmdhd
#
################################################################################

AIC8800_SDIO_SITE = $(realpath $(TOPDIR))"/package/aic8800_sdio/src"
AIC8800_SDIO_SITE_METHOD = local
AIC8800_SDIO_INSTALL_STAGING = YES
AIC8800_SDIO_INSTALL_TARGET = YES
AIC8800_SDIO_SUPPORTS_IN_SOURCE_BUILD = NO


define AIC8800_SDIO_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/firmware/
	mkdir -p $(TARGET_DIR)/etc/init.d/
	cp -rf $(@D)/fw/* $(TARGET_DIR)/lib/firmware/
	#cp -rf $(@D)/S100aic8800 $(TARGET_DIR)/etc/init.d/
endef

define AIC8800_SDIO_LINUX_CONFIG_FIXUPS
	$(call KCONFIG_ENABLE_OPT,CONFIG_WIRELESS)
	$(call KCONFIG_ENABLE_OPT,CONFIG_CFG80211)
	$(call KCONFIG_ENABLE_OPT,CONFIG_MAC80211)
	$(call KCONFIG_ENABLE_OPT,CONFIG_MMC)
	$(call KCONFIG_ENABLE_OPT,CONFIG_BT_HCIUART)
	$(call KCONFIG_ENABLE_OPT,CONFIG_BT_HCIUART_H4)
	$(call KCONFIG_ENABLE_OPT,CONFIG_BT_BNEP)
	$(call KCONFIG_ENABLE_OPT,CONFIG_BT_HIDP)
	$(call KCONFIG_ENABLE_OPT,CONFIG_BT_RFCOMM)
	$(call KCONFIG_ENABLE_OPT,CONFIG_BT_RFCOMM_TTY)
endef

$(eval $(kernel-module))
$(eval $(generic-package))
