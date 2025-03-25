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


define HELLOWORLD_CMAKE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/buildroot-build/helloWorld_cmake $(TARGET_DIR)/usr/bin/
endef

$(eval $(cmake-package))
