################################################################################
#
# uvc-gadget
#
################################################################################

UVC_GADGET_VERSION = 04c18aa6c4a7017957e2dfe88c3ff7ef34a1b3a0
UVC_GADGET_SITE = https://gitlab.freedesktop.org/camera/uvc-gadget.git
UVC_GADGET_SITE_METHOD = git
UVC_GADGET_LICENSE = GPL-2.0+
UVC_GADGET_LICENSE_FILES = LICENSE

# 依赖 libv4l2 库
UVC_GADGET_DEPENDENCIES = libv4l host-pkgconf

# v0.3.0 默认可能需要一些配置选项
UVC_GADGET_CONF_OPTS = -Dwerror=false


define UVC_GADGET_LINUX_CONFIG_FIXUPS
	$(call KCONFIG_SET_OPT,CONFIG_USB_CONFIGFS_F_UVC,y)
endef


define UVC_GADGET_BUILD_DEB
	# 创建目录
	mkdir -p $(@D)/deb/DEBIAN
	mkdir -p $(@D)/deb/root/app/

	# 拷贝应用
	$(call COPYFILE,$(UVC_GADGET_PKGDIR)/uvc-gadget.sh, $(TARGET_DIR)/root/app/uvc-gadget/)
	$(call COPYFILE,$(UVC_GADGET_PKGDIR)/uvc-gadget.sh,  $(@D)/deb/root/app/uvc-gadget/)
	$(call COPYFILE,$(UVC_GADGET_PKGDIR)/uvc-gadget-acm.sh, $(TARGET_DIR)/root/app/uvc-gadget/)
	$(call COPYFILE,$(UVC_GADGET_PKGDIR)/uvc-gadget-acm.sh,  $(@D)/deb/root/app/uvc-gadget/)

	$(call COPYFILE,$(TARGET_DIR)/usr/bin/uvc-gadget, $(@D)/deb/usr/bin/)


	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libuvcgadget.so.0 ,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libavdevice.so.58 ,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libavfilter.so.7 ,$(@D)/deb/usr/lib/riscv64-linux-gnu/)

	# 写 control 文件
	echo "Package: k230-uvc-gadtet"        >  $(@D)/deb/DEBIAN/control
	echo "Version: 1.0"              			>> $(@D)/deb/DEBIAN/control
	echo "Section: base"             			>> $(@D)/deb/DEBIAN/control
	echo "Priority: optional"        			>> $(@D)/deb/DEBIAN/control
	echo "Architecture: riscv64"     			>> $(@D)/deb/DEBIAN/control
	echo "Maintainer: K230 Dev <dev@example.com>" >> $(@D)/deb/DEBIAN/control
	echo "Description: uvc gadget application for K230" >> $(@D)/deb/DEBIAN/control

	# 打包
	mkdir -p $(BINARIES_DIR)/deb
	dpkg -b $(@D)/deb $(BINARIES_DIR)/deb/$(call LOWERCASE,k230-$(PKG)).deb
endef


UVC_GADGET_POST_INSTALL_TARGET_HOOKS +=   UVC_GADGET_BUILD_DEB


$(eval $(meson-package))
