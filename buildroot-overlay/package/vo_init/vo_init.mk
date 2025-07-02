VO_INIT_SITE = $(realpath $(TOPDIR))"/package/vo_init/src"
VO_INIT_SITE_METHOD = local
VO_INIT_INSTALL_STAGING = YES


define VO_INIT_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CPP="$(TARGET_CXX)" CFLAGS="$(TARGET_CXXFLAGS)" -C $(@D)
endef

define VO_INIT_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/vo_init $(TARGET_DIR)/usr/bin/vo_init
endef

$(eval $(generic-package))
