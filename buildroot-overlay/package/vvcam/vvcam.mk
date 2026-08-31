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
VVCAM_SUPPORTS_IN_SOURCE_BUILD = NO

VVCAM_MODULE_MAKE_OPTS += BR2_PACKAGE_VVCAM_DEF_SENSOR=$(BR2_PACKAGE_VVCAM_DEF_SENSOR)

$(eval $(kernel-module))



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
