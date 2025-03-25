CAMERA_RTSP_DEMO_SITE = $(realpath $(TOPDIR))"/package/camera_rtsp_demo/src"
CAMERA_RTSP_DEMO_SITE_METHOD = local
CAMERA_RTSP_DEMO_INSTALL_STAGING = YES

CAMERA_RTSP_DEMO_DEPENDENCIES += librtsp_server ffmpeg

define CAMERA_RTSP_DEMO_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" AR="$(TARGET_AR)" -C $(@D)
endef

define CAMERA_RTSP_DEMO_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/camera_rtsp_demo $(TARGET_DIR)/usr/bin/camera_rtsp_demo
endef

$(eval $(generic-package))
