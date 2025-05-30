FACE_DETECT_SITE = $(realpath $(TOPDIR))"/package/face_detect"
FACE_DETECT_SITE_METHOD = local
FACE_DETECT_DEPENDENCIES += libmmz libnncase gsl-lite

ifeq ($(BR2_PACKAGE_OPENCV4),y)
FACE_DETECT_DEPENDENCIES += opencv4 display vvcam
endif



define FACE_DETECT_BUILD_DEB
	$(call COPYFILE ,$(TARGET_DIR)/root/app/face_detect,$(@D)/deb/app/)
	dpkg -b  $(@D)/deb  $(BINARIES_DIR)/deb/$(call LOWERCASE, k230-$(PKG)).deb
endef

FACE_DETECT_POST_INSTALL_TARGET_HOOKS += FACE_DETECT_BUILD_DEB

$(eval $(cmake-package))
