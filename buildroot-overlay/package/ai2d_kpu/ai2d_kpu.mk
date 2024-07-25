AI2D_KPU_SITE = $(realpath $(TOPDIR))"/package/ai2d_kpu"
AI2D_KPU_SITE_METHOD = local
AI2D_KPU_DEPENDENCIES += libmmz libnncase


define AI2D_KPU_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CXX)" -C $(@D)
endef

define AI2D_KPU_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/app/ai2d_kpu
	$(INSTALL) -m 0755 $(@D)/ai2d_kpu.elf $(TARGET_DIR)/app/ai2d_kpu
	$(INSTALL) -m 0755 $(@D)/test.kmodel $(TARGET_DIR)/app/ai2d_kpu
	$(INSTALL) -m 0755 $(@D)/ai2d_input.bin $(TARGET_DIR)/app/ai2d_kpu
	$(INSTALL) -m 0755 $(@D)/input.bin $(TARGET_DIR)/app/ai2d_kpu
	$(INSTALL) -m 0755 $(@D)/result.bin $(TARGET_DIR)/app/ai2d_kpu
	$(INSTALL) -m 0755 $(@D)/run.sh $(TARGET_DIR)/app/ai2d_kpu
endef

$(eval $(generic-package))
