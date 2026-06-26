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

# LVGL v9 CMake configure requires pcpp (C preprocessor).
# Buildroot's host Python lacks SSL so pip can't install pcpp.
# Copy system pcpp into host/bin so shutil.which("pcpp") finds it.
define LVGL_ENSURE_PCPP
	if ! which pcpp > /dev/null 2>&1; then \
		pip3 install pcpp; \
	fi
	$(INSTALL) -D -m 0755 $$(which pcpp) $(HOST_DIR)/bin/pcpp
endef
LVGL_PRE_CONFIGURE_HOOKS += LVGL_ENSURE_PCPP


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


$(eval $(cmake-package))

endif
