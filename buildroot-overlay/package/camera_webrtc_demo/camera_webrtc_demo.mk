CAMERA_WEBRTC_DEMO_SITE = $(realpath $(TOPDIR))"/package/camera_webrtc_demo/src"
CAMERA_WEBRTC_DEMO_SITE_METHOD = local
CAMERA_WEBRTC_DEMO_INSTALL_STAGING = YES

CAMERA_WEBRTC_DEMO_DEPENDENCIES += webrtc ffmpeg libevent cjson

define CAMERA_WEBRTC_DEMO_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CPP="$(TARGET_CXX)" CC="$(TARGET_CC)" CFLAGS="$(TARGET_CXXFLAGS)" -C $(@D)
endef

define CAMERA_WEBRTC_DEMO_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/camera_webrtc_demo $(TARGET_DIR)/root/app/camera_webrtc_demo
endef

$(eval $(generic-package))
