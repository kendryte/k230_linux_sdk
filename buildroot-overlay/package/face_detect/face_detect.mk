FACE_DETECT_SITE = $(realpath $(TOPDIR))"/package/face_detect"
FACE_DETECT_SITE_METHOD = local
FACE_DETECT_DEPENDENCIES += libmmz libnncase


define FACE_DETECT_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CXX)" -C $(@D)
endef

define FACE_DETECT_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/app/face_detect
	$(INSTALL) -m 0755 $(@D)/face_detect $(TARGET_DIR)/app/face_detect
	$(INSTALL) -m 0755 $(@D)/ai2d_input.bin $(TARGET_DIR)/app/face_detect
	$(INSTALL) -m 0755 $(@D)/face_detection_320.kmodel $(TARGET_DIR)/app/face_detect
endef

$(eval $(generic-package))
