################################################################################
#
# vvcam
#
################################################################################

VVCAM_DIR_NAME := vvcam
VVCAM_APP_NAME := vvcam
VVCAM_SITE = $(realpath $(TOPDIR))"/package/vvcam"
VVCAM_SITE_METHOD = local
VVCAM_INSTALL_STAGING = YES
VVCAM_INSTALL_TARGET = YES
VVCAM_DEPENDENCIES = mxml libmicrohttpd display umtprd android-tools isp-media-server
ifeq ($(BR2_PACKAGE_LIBCAMERA),y)
VVCAM_DEPENDENCIES += libcamera
endif
VVCAM_SUPPORTS_IN_SOURCE_BUILD = NO

VVCAM_MODULE_MAKE_OPTS += BR2_PACKAGE_VVCAM_DEF_SENSOR=$(BR2_PACKAGE_VVCAM_DEF_SENSOR)

$(eval $(kernel-module))



ifeq ($(BR2_PACKAGE_LIBCAMERA),y)
define VVCAM_BUILD_DEB_LIBCAMERA
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/vvcam-libcamera-drm,$(@D)/deb/usr/bin/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/vvcam-libcamera-hold,$(@D)/deb/usr/bin/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/vvcam-libcamera-grab,$(@D)/deb/usr/bin/)
endef
VVCAM_POST_INSTALL_TARGET_HOOKS += VVCAM_BUILD_DEB_LIBCAMERA
endif

define VVCAM_BUILD_DEB
	$(call COPYFILE ,$(TARGET_DIR)/etc/vvcam,$(@D)/deb/etc/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libvvcam.so,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libv4l2-drm.so,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libv4l2-drm++.so,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libdisplay.so,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/isp_media_server,$(@D)/deb/usr/bin/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/v4l2-drm           ,$(@D)/deb/usr/bin/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/v4l2-drm-three-camera ,$(@D)/deb/usr/bin/)
	$(call COPYFILE ,$(TARGET_DIR)/lib/modules/6.6.36/updates/,$(@D)/deb/usr/lib/modules/6.6.36/)
	$(call COPYFILE ,$(BR2_ROOTFS_OVERLAY)/etc/init.d/S41adb_mtp ,$(@D)/deb/etc/vvcam/)
	$(call COPYFILE ,$(BR2_ROOTFS_OVERLAY)/etc/umtprd,$(@D)/deb/etc/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/sbin/umtprd,$(@D)/deb/usr/sbin/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/adbd ,$(@D)/deb/usr/bin/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libcrypt.so.2 ,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	dpkg -b  $(@D)/deb  $(BINARIES_DIR)/deb/$(call LOWERCASE, k230-$(PKG)).deb
endef

VVCAM_POST_INSTALL_TARGET_HOOKS += VVCAM_BUILD_DEB

$(eval $(cmake-package))
include $(sort $(wildcard package/vvcam/isp-media-server/*.mk))

# make vvcam-rebuild 会先整包重编 libcamera，再编 vvcam。
# pipeline 改动在 libcamera 的 patch 里，漏掉这一步时 vvcam 仍链着旧的 libcamera.so。
# 只改 v4l2-drm 时也会走这条，代价是 libcamera 全量重编。这是有意的。
ifeq ($(BR2_PACKAGE_LIBCAMERA),y)
vvcam-clean-for-rebuild: libcamera-rebuild
endif
