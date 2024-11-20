AUDIO_DEMO_SITE = $(realpath $(TOPDIR))"/package/audio_demo"
AUDIO_DEMO_SITE_METHOD = local
AUDIO_DEMO_INSTALL_STAGING = YES

define AUDIO_DEMO_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" AR="$(TARGET_AR)" -C $(@D)
endef

define AUDIO_DEMO_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0755 $(@D)/audio_demo $(TARGET_DIR)/usr/bin/audio_demo
endef

$(eval $(generic-package))