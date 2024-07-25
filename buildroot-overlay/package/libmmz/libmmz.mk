LIBMMZ_SITE = $(realpath $(TOPDIR))"/package/libmmz"
LIBMMZ_SITE_METHOD = local

define LIBMMZ_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" AR="$(TARGET_AR)" -C $(@D)
endef

define LIBMMZ_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/libmmz.so $(TARGET_DIR)/usr/lib/
endef

$(eval $(generic-package))
