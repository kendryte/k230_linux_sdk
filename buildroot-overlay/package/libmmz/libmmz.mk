LIBMMZ_SITE = $(realpath $(TOPDIR))"/package/libmmz"
LIBMMZ_SITE_METHOD = local
LIBMMZ_INSTALL_STAGING = YES

define LIBMMZ_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" AR="$(TARGET_AR)" -C $(@D)
endef

define LIBMMZ_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0644 $(@D)/mmz.h $(STAGING_DIR)/usr/include/mmz.h
	$(INSTALL) -m 0755 $(@D)/libmmz.a $(STAGING_DIR)/usr/lib/libmmz.a
endef

$(eval $(generic-package))
