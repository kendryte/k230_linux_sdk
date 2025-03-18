################################################################################
#
# bcmdhd
#
################################################################################

K230_NOC_SITE = $(realpath $(TOPDIR))"/package/k230_noc/src"
K230_NOC_SITE_METHOD = local
K230_NOC_INSTALL_STAGING = YES
K230_NOC_INSTALL_TARGET = YES
K230_NOC_SUPPORTS_IN_SOURCE_BUILD = NO


# define K230_NOC_INSTALL_TARGET_CMDS
# 	mkdir -p $(TARGET_DIR)/lib/firmware/
# 	mkdir -p $(TARGET_DIR)/etc/init.d/
# 	cp -rf $(@D)/fw/* $(TARGET_DIR)/lib/firmware/
# 	cp -rf $(@D)/S100aic8800 $(TARGET_DIR)/etc/init.d/
# endef

# define K230_NOC_LINUX_CONFIG_FIXUPS
# 	$(call KCONFIG_ENABLE_OPT,CONFIG_WIRELESS)
# 	$(call KCONFIG_ENABLE_OPT,CONFIG_CFG80211)
# 	$(call KCONFIG_ENABLE_OPT,CONFIG_MAC80211)
# 	$(call KCONFIG_ENABLE_OPT,CONFIG_MMC)
# 	$(call KCONFIG_ENABLE_OPT,CONFIG_BT_HCIUART)
# 	$(call KCONFIG_ENABLE_OPT,CONFIG_BT_HCIUART_H4)

# endef

$(eval $(kernel-module))
$(eval $(generic-package))
