################################################################################
#
# smart_ipc
#
################################################################################

SMART_IPC_VERSION = 1.0.0
SMART_IPC_SITE = $(realpath $(TOPDIR))"/package/smart_ipc/src"
SMART_IPC_SITE_METHOD = local
SMART_IPC_INSTALL_TARGET = YES
SMART_IPC_SUPPORTS_IN_SOURCE_BUILD = NO
SMART_IPC_LICENSE = Apache-2.0

# All of them are required, not optional. Config.in selects every one, and
# the CMakeLists stops with a message if any is missing. There used to be
# ifeq blocks here making the AI and WebRTC parts conditional; the build
# they were meant to allow was never produced, and the #ifdefs they implied
# in smart_ipc.c were a place for bugs to hide.
SMART_IPC_DEPENDENCIES = libvcodec display libdrm librtsp_server live555 \
	libnncase gsl-lite libmmz opencv4 webrtc cjson nonai2d

define SMART_IPC_BUILD_DEB
	mkdir -p $(@D)/deb/DEBIAN
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/smart_ipc,$(@D)/deb/usr/bin/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/share/smart_ipc/face_detection_320.kmodel,$(@D)/deb/usr/share/smart_ipc/)

	echo "Package: k230-smart-ipc"                  >  $(@D)/deb/DEBIAN/control
	echo "Version: $(SMART_IPC_VERSION)"            >> $(@D)/deb/DEBIAN/control
	echo "Section: base"                            >> $(@D)/deb/DEBIAN/control
	echo "Priority: optional"                       >> $(@D)/deb/DEBIAN/control
	echo "Architecture: riscv64"                    >> $(@D)/deb/DEBIAN/control
	echo "Depends: k230-libvcodec"                  >> $(@D)/deb/DEBIAN/control
	echo "Maintainer: K230 Dev <dev@example.com>"   >> $(@D)/deb/DEBIAN/control
	echo "Description: K230 network camera: screen + stream + face detection" >> $(@D)/deb/DEBIAN/control

	mkdir -p $(BINARIES_DIR)/deb
	dpkg -b $(@D)/deb $(BINARIES_DIR)/deb/$(call LOWERCASE,k230-$(PKG)).deb
endef

SMART_IPC_POST_INSTALL_TARGET_HOOKS += SMART_IPC_BUILD_DEB

$(eval $(cmake-package))
