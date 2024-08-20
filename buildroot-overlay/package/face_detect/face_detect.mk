FACE_DETECT_SITE = $(realpath $(TOPDIR))"/package/face_detect"
FACE_DETECT_SITE_METHOD = local
FACE_DETECT_DEPENDENCIES += libmmz libnncase

ifeq ($(BR2_PACKAGE_OPENCV4),y)
FACE_DETECT_DEPENDENCIES += opencv4
endif

$(eval $(cmake-package))
