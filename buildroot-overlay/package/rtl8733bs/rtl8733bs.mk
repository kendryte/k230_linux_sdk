################################################################################
#
# rtl8733bs
#
################################################################################
RTL8733BS_INSTALL_STAGING = YES
RTL8733BS_INSTALL_TARGET = YES
RTL8733BS_SUPPORTS_IN_SOURCE_BUILD = NO

RTL8733BS_SITE := https://download.kendryte.com/k230/downloads/dl/rtl8733bs


RTL8733BS_wifi := rtl8733BS_WiFi_linux_v5.15.17-113-g1924716b3.20250521_COEX20241204-390f
RTL8733BS_rtwpriv := rtwpriv_release_v5.8.0.20220302
RTL8733BS_bt := 20250813_LINUX_BT_DRIVER

RTL8733BS_SOURCE := $(RTL8733BS_wifi).tgz
RTL8733BS_EXTRA_DOWNLOADS :=  $(RTL8733BS_rtwpriv).tgz  $(RTL8733BS_bt).tgz


define RTL8733BS_LINUX_CONFIG_FIXUPS
	$(call KCONFIG_ENABLE_OPT,CONFIG_WIRELESS)
	$(call KCONFIG_ENABLE_OPT,CONFIG_CFG80211)
	$(call KCONFIG_ENABLE_OPT,CONFIG_MAC80211)
	$(call KCONFIG_ENABLE_OPT,CONFIG_MMC)
	$(call KCONFIG_ENABLE_OPT,CONFIG_BT_HCIUART)
	$(call KCONFIG_ENABLE_OPT,CONFIG_BT_HCIUART_H4)
endef



define RTL8733BS_EXTRACT_CMDS
	tar -xf $(RTL8733BS_DL_DIR)/$(RTL8733BS_SOURCE) -C  $(@D)
	tar -xf $(RTL8733BS_DL_DIR)/$(RTL8733BS_rtwpriv).tgz -C  $(@D)
	#rsync -au --chmod=u=rwX,go=rX $($(PKG)_OVERRIDE_SRCDIR_RSYNC_EXCLUSIONS) $(RSYNC_VCS_EXCLUSIONS) $(RTL8733BS_PKGDIR)/src/ $(@D)
endef


define RTL8733BS_INSTALL_TARGET_CMDS
	cp -rf $(@D)/rtwpriv_release_v5.8.0.20220302/rtwpriv $(TARGET_DIR)/usr/bin/

endef

define RTL8733BS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" AR="$(TARGET_AR)" -C $(@D)/$(RTL8733BS_rtwpriv)
endef



RTL8733BS_MODULE_SUBDIRS = $(RTL8733BS_wifi)
RTL8733BS_MODULE_MAKE_OPTS = USER_EXTRA_CFLAGS=-DCONFIG_CONCURRENT_MODE  CONFIG_RTL8733BS=m  CONFIG_RTW_LOG_LEVEL=3

$(eval $(kernel-module))
$(eval $(generic-package))
