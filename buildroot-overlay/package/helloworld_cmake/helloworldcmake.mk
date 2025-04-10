################################################################################
#
# bcmdhd
#
################################################################################
#pkgdir = $(dir $(lastword $(MAKEFILE_LIST)))
HELLOWORLD_CMAKE_SITE = "$(realpath $(TOPDIR))/package/helloworld_cmake/src"
HELLOWORLD_CMAKE_SITE_METHOD = local
HELLOWORLD_CMAKE_INSTALL_STAGING = YES
HELLOWORLD_CMAKE_INSTALL_TARGET = YES
HELLOWORLD_CMAKE_SUPPORTS_IN_SOURCE_BUILD = NO



# $1 src $2 dst_dir
define COPYFILE
	mkdir -p $2 ;cp  -rf $1 $2;
endef

define HELLOWORLD_CMAKE_BUILD_DEB
	$(call COPYFILE ,$(TARGET_DIR)/root/helloworld,$(@D)/deb/root/)
	dpkg -b  $(@D)/deb  $(BINARIES_DIR)/deb/$(call LOWERCASE, $(PKG)).deb
endef

HELLOWORLD_CMAKE_POST_INSTALL_TARGET_HOOKS += HELLOWORLD_CMAKE_BUILD_DEB


$(eval $(cmake-package))
