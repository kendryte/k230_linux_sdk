################################################################################
#
# LVGL
#
################################################################################
LVGL_VERSION = $(call qstrip,$(BR2_PACKAGE_LVGL_CUSTOM_VERSION))

LVGL_DEPENDENCIES += libdrm vg_lite

LVGL_CFLAG = -I$(STAGING_DIR)/usr/include/libdrm

ifeq ($(BR2_RISCV_32), y)
LVGL_CFLAG += -march=rv32gcv_xtheadc
LVGL_LDFLAG += -march=rv32gcv_xtheadc
endif

ifeq ($(LVGL_VERSION), 8.3.7)
LVGL_SITE = $(call github,lvgl,lvgl,v$(LVGL_VERSION))
define LVGL_EXTRACT_CMDS
	tar zxf $(LVGL_DL_DIR)/$(LVGL_SOURCE) -C $(@D)
	mv $(@D)/lvgl-* $(@D)/lvgl
	rsync -au --chmod=u=rwX,go=rX --exclude='*.patch'  $(RSYNC_VCS_EXCLUSIONS) $(LVGL_PKGDIR)/$(LVGL_VERSION)/ $(@D)
endef

define LVGL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" CFLAG="$(LVGL_CFLAG)" LDFLAG="$(LVGL_LDFLAG)" -C $(@D)
endef

define LVGL_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR="$(TARGET_DIR)" -C $(@D) install
endef

$(eval $(generic-package))

endif




ifeq ($(LVGL_VERSION), 59dc7e436ae97a25e32656739ea6a943f9f11b6a)
LVGL_SITE = $(call github,lvgl,lvgl,$(LVGL_VERSION))
LVGL_SUPPORTS_IN_SOURCE_BUILD = NO

LVGL_DEPENDENCIES += libevdev
LVGL_EXTRA_DOWNLOADS = $(call github,lvgl,lv_port_linux,b492d738b00cb7733c2ea442e2a2d5070f74d663.tar.gz)


define LVGL_EXTRACT_CMDS
	tar zxf $(LVGL_DL_DIR)/$(LVGL_SOURCE) -C $(@D)
	mv $(@D)/lvgl-* $(@D)/lvgl
	tar zxf $(LVGL_DL_DIR)/b492d738b00cb7733c2ea442e2a2d5070f74d663.tar.gz --strip-components=1  -C $(@D)
	rsync -au --chmod=u=rwX,go=rX --exclude='*.patch'  $(RSYNC_VCS_EXCLUSIONS) $(LVGL_PKGDIR)/$(LVGL_VERSION)/ $(@D)
endef

# define LVGL_INSTALL_TARGET_CMDS
# 	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR="$(TARGET_DIR)" -C $(@D) install
# endef


$(eval $(cmake-package))

endif



ifeq ($(LVGL_VERSION), 97a99a8affe966617f048830a007bbf8bea63da6)
LVGL_SITE = $(call github,lvgl,lvgl,$(LVGL_VERSION))
LVGL_SUPPORTS_IN_SOURCE_BUILD = NO
LVGL_INSTALL_STAGING = YES

LVGL_DEPENDENCIES += libevdev
LVGL_EXTRA_DOWNLOADS = $(call github,lvgl,lv_port_linux,0a57deb47bada6916da2a1d103f31458ce933a6c.tar.gz)


define LVGL_EXTRACT_CMDS
	tar zxf $(LVGL_DL_DIR)/$(LVGL_SOURCE) -C $(@D)
	mv $(@D)/lvgl-* $(@D)/lvgl
	tar zxf $(LVGL_DL_DIR)/0a57deb47bada6916da2a1d103f31458ce933a6c.tar.gz --strip-components=1  -C $(@D)
	#rsync -au --chmod=u=rwX,go=rX --exclude='*.patch'  $(RSYNC_VCS_EXCLUSIONS) $(LVGL_PKGDIR)/$(LVGL_VERSION)/ $(@D)
endef

define LVGL_INSTALL_LV_CONF
	$(INSTALL) -D -m 0644 $(@D)/buildroot-build/lv_conf.h $(STAGING_DIR)/usr/include/lvgl/lv_conf.h
endef
LVGL_POST_INSTALL_STAGING_HOOKS += LVGL_INSTALL_LV_CONF

# define LVGL_INSTALL_TARGET_CMDS
# 	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR="$(TARGET_DIR)" -C $(@D) install
# endef


define LVGL_BUILD_DEB
		# 创建目录
	mkdir -p $(@D)/deb/DEBIAN
	mkdir -p $(@D)/deb/root/app/

	# 写 control 文件
	echo "Package: k230-lvgl"        >  $(@D)/deb/DEBIAN/control
	echo "Version: 1.0"              			>> $(@D)/deb/DEBIAN/control
	echo "Section: base"             			>> $(@D)/deb/DEBIAN/control
	echo "Priority: optional"        			>> $(@D)/deb/DEBIAN/control
	echo "Architecture: riscv64"     			>> $(@D)/deb/DEBIAN/control
	echo "Maintainer: K230 Dev <dev@example.com>" >> $(@D)/deb/DEBIAN/control
	echo "Description: k230 lvgl" >> $(@D)/deb/DEBIAN/control


	# LVGL libraries
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/liblvgl.so,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/liblvgl_linux.so,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/liblvgl_demos.so,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/liblvgl_examples.so,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/liblvgl_thorvg.so,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/liblvgl_thorvg.so.9,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/liblvgl_thorvg.so.9.6.0,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	# LVGL applications
	$(call COPYFILE ,$(TARGET_DIR)/root/app/lvglsim,$(@D)/deb/root/app/)
	$(call COPYFILE ,$(TARGET_DIR)/root/app/lvglsimk230,$(@D)/deb/root/app/)
	# LVGL Python demos and assets
	$(call COPYFILE ,$(TARGET_DIR)/root/py_demo/lvgl,$(@D)/deb/root/py_demo/)
	# LVGL font files
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/fonts/SourceHanSansSC-Normal-Min.ttf,$(@D)/deb/usr/lib/fonts/)

	# 打包
	mkdir -p $(BINARIES_DIR)/deb
	dpkg -b  $(@D)/deb  $(BINARIES_DIR)/deb/$(call LOWERCASE, k230-$(PKG)).deb
endef

LVGL_POST_INSTALL_TARGET_HOOKS += LVGL_BUILD_DEB


$(eval $(cmake-package))


endif
