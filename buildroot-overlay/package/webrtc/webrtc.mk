################################################################################
#
# webrtc (libpeer)
#
################################################################################

WEBRTC_SITE = $(realpath $(TOPDIR))"/package/webrtc/src"
WEBRTC_SITE_METHOD = local
WEBRTC_INSTALL_STAGING = YES
WEBRTC_INSTALL_TARGET = YES
WEBRTC_SUPPORTS_IN_SOURCE_BUILD = NO

WEBRTC_DEPENDENCIES += host-cmake cjson

define WEBRTC_CONFIGURE_CMDS
	mkdir -p $($(PKG)_BUILDDIR)
	(cd $($(PKG)_BUILDDIR) && \
		$(TARGET_CONFIGURE_OPTS) \
		$(HOST_DIR)/bin/cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DCMAKE_C_COMPILER="$(TARGET_CC)" \
		-DCMAKE_C_FLAGS="$(TARGET_CFLAGS) -fPIC" \
		-DWEBRTC_STAGING_DIR="$(STAGING_DIR)" \
		-DBUILD_SHARED_LIBS=OFF \
		-DLINUX_BUILD=1 \
		$($(PKG)_SRCDIR))
endef

define WEBRTC_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $($(PKG)_BUILDDIR)
endef

define WEBRTC_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $($(PKG)_BUILDDIR) DESTDIR=$(STAGING_DIR) install
endef

define WEBRTC_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $($(PKG)_BUILDDIR) DESTDIR=$(TARGET_DIR) install
endef

$(eval $(cmake-package))
