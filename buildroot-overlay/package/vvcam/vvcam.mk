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
VVCAM_DEPENDENCIES = mxml libmicrohttpd display
VVCAM_SUPPORTS_IN_SOURCE_BUILD = NO

VVCAM_MODULE_MAKE_OPTS += BR2_PACKAGE_VVCAM_DEF_SENSOR=$(BR2_PACKAGE_VVCAM_DEF_SENSOR)

$(eval $(kernel-module))

# $1 src $2 dst_dir
define COPYFILE
	mkdir -p $2 ;cp  -rpf $1 $2;
endef

define VVCAM_BUILD_DEB
	$(call COPYFILE ,$(TARGET_DIR)/etc/vvcam,$(@D)/deb/etc/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libvvcam.so,$(@D)/deb/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libv4l2-drm.so,$(@D)/deb/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libdisplay.so,$(@D)/deb/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/isp_media_server,$(@D)/deb/usr/bin/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/v4l2-drm           ,$(@D)/deb/usr/bin/)
	$(call COPYFILE ,$(TARGET_DIR)/lib/modules/6.6.36/updates/,$(@D)/deb/lib/modules/6.6.36/)
	dpkg -b  $(@D)/deb  $(BINARIES_DIR)/deb/$(call LOWERCASE, $(PKG)).deb
endef

VVCAM_POST_INSTALL_TARGET_HOOKS += VVCAM_BUILD_DEB

$(eval $(cmake-package))
