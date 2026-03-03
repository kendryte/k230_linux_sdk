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
UVC_GADGET_DEPENDENCIES = libv4l host-pkgconf  libcamera

# v0.3.0 默认可能需要一些配置选项
UVC_GADGET_CONF_OPTS = -Dwerror=false


define UVC_GADGET_LINUX_CONFIG_FIXUPS
	$(call KCONFIG_SET_OPT,CONFIG_USB_CONFIGFS_F_UVC,y)
endef

$(eval $(meson-package))
