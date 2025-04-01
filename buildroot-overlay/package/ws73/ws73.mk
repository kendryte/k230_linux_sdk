################################################################################
#
# ws73
#
################################################################################

WS73_SITE = $(realpath $(TOPDIR))"/package/ws73"
WS73_SITE_METHOD = local
WS73_INSTALL_STAGING = YES
WS73_INSTALL_TARGET = YES
WS73_SUPPORTS_IN_SOURCE_BUILD = NO

define WS73_PRE_CONFIGURE_SCRIPT
	$(Q)chmod +x $(@D)/build.sh
	$(Q)$(@D)/build.sh "$(@D)"
endef

WS73_PRE_CONFIGURE_HOOKS += WS73_PRE_CONFIGURE_SCRIPT

define WS73_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/ws73
	cp -rf $(@D)/firmware/us/* $(TARGET_DIR)/etc/ws73
	cp -rf $(@D)/output/bin/ws73_cfg.ini $(TARGET_DIR)/etc/
	mkdir -p $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/extra
	cp $(@D)//output/bin/*.ko $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/extra/
	echo "update_config=1" >> $(TARGET_DIR)/wpa_supplicant.conf
endef

$(eval $(kernel-module))
$(eval $(generic-package))
