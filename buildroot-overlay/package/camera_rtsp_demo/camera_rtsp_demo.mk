CAMERA_RTSP_DEMO_SITE = $(realpath $(TOPDIR))"/package/camera_rtsp_demo/src"
CAMERA_RTSP_DEMO_SITE_METHOD = local
CAMERA_RTSP_DEMO_INSTALL_STAGING = YES

CAMERA_RTSP_DEMO_DEPENDENCIES += librtsp_server ffmpeg  libevent

define CAMERA_RTSP_DEMO_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CPP="$(TARGET_CXX)" CFLAGS="$(TARGET_CXXFLAGS)" -C $(@D)
endef

define CAMERA_RTSP_DEMO_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/camera_rtsp_demo $(TARGET_DIR)/usr/bin/camera_rtsp_demo
	cp $(CAMERA_RTSP_DEMO_SITE)/libsmolrtsp_server.so $(TARGET_DIR)/usr/lib/libsmolrtsp_server.so
endef

$(eval $(generic-package))
