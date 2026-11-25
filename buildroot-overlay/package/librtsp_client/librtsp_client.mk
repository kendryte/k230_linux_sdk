LIBRTSP_CLIENT_SITE = $(realpath $(TOPDIR))"/package/librtsp_client/src"
LIBRTSP_CLIENT_SITE_METHOD = local
LIBRTSP_CLIENT_INSTALL_STAGING = YES

define LIBRTSP_CLIENT_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CPP="$(TARGET_CXX)" CFLAGS="$(TARGET_CXXFLAGS)" -C $(@D)
endef

LIBRTSP_CLIENT_DEPENDENCIES += live555


define LIBRTSP_CLIENT_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0644 $(@D)/include/rtsp_client.h $(STAGING_DIR)/usr/include/rtsp_client.h
	$(INSTALL) -m 0644 $(@D)/librtsp_client.so $(STAGING_DIR)/usr/lib/librtsp_client.so
endef

define LIBRTSP_CLIENT_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0644 $(@D)/librtsp_client.so $(TARGET_DIR)/usr/lib/librtsp_client.so
endef

$(eval $(generic-package))
