LIBNNCASE_VERSION =
NNCASE_VERSION_NUM=2.11.0
NNCASE_VERSION = v$(NNCASE_VERSION_NUM)

LIBNNCASE_SOURCE = nncase_k230_$(NNCASE_VERSION)_runtime_linux.tgz
LIBNNCASE_SITE = https://github.com/kendryte/nncase/releases/download/$(NNCASE_VERSION)

LIBNNCASE_RISCV_WHL = nncaseruntime_k230-$(NNCASE_VERSION_NUM)-py3-none-linux_riscv64.whl

LIBNNCASE_EXTRA_DOWNLOADS := $(LIBNNCASE_SITE)/$(LIBNNCASE_RISCV_WHL)



define LIBNNCASE_EXTRACT_CMDS
	tar zxf $(LIBNNCASE_DL_DIR)/$(LIBNNCASE_SOURCE) -C $(@D)
	mv $(@D)/nncase_k230_$(NNCASE_VERSION)_runtime_linux $(@D)/nncase
endef

define LIBNNCASE_INSTALL_TARGET_CMDS
	cp -r $(@D)/nncase/* $(STAGING_DIR)/usr/
	cp $(LIBNNCASE_DL_DIR)/$(LIBNNCASE_RISCV_WHL)   $(TARGET_DIR)/root/
endef



$(eval $(generic-package))
