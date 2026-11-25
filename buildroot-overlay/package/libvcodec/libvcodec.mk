################################################################################
#
# libvcodec
#
################################################################################

LIBVCODEC_VERSION = 1.0.0
LIBVCODEC_SITE = $(realpath $(TOPDIR))"/package/libvcodec/src"
LIBVCODEC_SITE_METHOD = local
LIBVCODEC_INSTALL_STAGING = YES
LIBVCODEC_INSTALL_TARGET = YES
LIBVCODEC_SUPPORTS_IN_SOURCE_BUILD = NO
LIBVCODEC_LICENSE = Apache-2.0

define LIBVCODEC_BUILD_DEB
	mkdir -p $(@D)/deb/DEBIAN
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libvcodec.so*,$(@D)/deb/usr/lib/riscv64-linux-gnu/)

	echo "Package: k230-libvcodec"             >  $(@D)/deb/DEBIAN/control
	echo "Version: $(LIBVCODEC_VERSION)"       >> $(@D)/deb/DEBIAN/control
	echo "Section: libs"                           >> $(@D)/deb/DEBIAN/control
	echo "Priority: optional"                      >> $(@D)/deb/DEBIAN/control
	echo "Architecture: riscv64"                   >> $(@D)/deb/DEBIAN/control
	echo "Maintainer: K230 Dev <dev@example.com>"  >> $(@D)/deb/DEBIAN/control
	echo "Description: K230 VPU video codec wrapper library" >> $(@D)/deb/DEBIAN/control

	mkdir -p $(BINARIES_DIR)/deb
	dpkg -b $(@D)/deb $(BINARIES_DIR)/deb/$(call LOWERCASE,k230-$(PKG)).deb
endef

LIBVCODEC_POST_INSTALL_TARGET_HOOKS += LIBVCODEC_BUILD_DEB

$(eval $(cmake-package))
