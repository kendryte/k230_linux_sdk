################################################################################
#
# LVGL
#
################################################################################
LVGL_VERSION = v8.3.7
LVGL_SOURCE = $(LVGL_VERSION).tar.gz
LVGL_SITE = https://github.com/lvgl/lvgl/archive/refs/tags
LVGL_DEPENDENCIES += libdrm vg_lite

LVGL_CFLAG = -I$(STAGING_DIR)/usr/include/libdrm

ifeq ($(BR2_RISCV_32), y)
LVGL_CFLAG += -march=rv32gcv_xtheadc
LVGL_LDFLAG += -march=rv32gcv_xtheadc
endif

define LVGL_EXTRACT_CMDS
	tar zxf $(LVGL_DL_DIR)/$(LVGL_SOURCE) -C $(@D)
	mv $(@D)/lvgl-* $(@D)/lvgl
	cp -r $(TOPDIR)/package/lvgl/* $(@D)
endef

define LVGL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" CFLAG="$(LVGL_CFLAG)" LDFLAG="$(LVGL_LDFLAG)" -C $(@D)
endef

define LVGL_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR="$(TARGET_DIR)" -C $(@D) install
endef

$(eval $(generic-package))
