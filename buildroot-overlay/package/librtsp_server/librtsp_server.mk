LIBRTSP_SERVER_SITE = $(realpath $(TOPDIR))"/package/librtsp_server/src"
LIBRTSP_SERVER_SITE_METHOD = local
LIBRTSP_SERVER_INSTALL_STAGING = YES

define LIBRTSP_SERVER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" AR="$(TARGET_AR)" -C $(@D)
endef

LIBRTSP_SERVER_DEPENDENCIES += live555


define LIBRTSP_SERVER_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0644 $(@D)/include/rtsp_server.h $(STAGING_DIR)/usr/include/rtsp_server.h
	$(INSTALL) -m 0644 $(@D)/librtsp_server.so $(STAGING_DIR)/usr/lib/librtsp_server.so
endef

define LIBRTSP_SERVER_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0644 $(@D)/librtsp_server.so $(TARGET_DIR)/usr/lib/librtsp_server.so
endef

$(eval $(generic-package))
