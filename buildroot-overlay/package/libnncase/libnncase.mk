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

ifeq ($(BR2_PACKAGE_PYTHON3),y)
define LIBNNCASE_INSTALL_TARGET_CMDS
	cp -r $(@D)/nncase/* $(STAGING_DIR)/usr/
	mkdir -p $(TARGET_DIR)/usr/lib/python$(PYTHON3_VERSION_MAJOR)/site-packages
	unzip -o $(LIBNNCASE_DL_DIR)/$(LIBNNCASE_RISCV_WHL) -d $(TARGET_DIR)/usr/lib/python$(PYTHON3_VERSION_MAJOR)/site-packages
endef
else
define LIBNNCASE_INSTALL_TARGET_CMDS
	cp -r $(@D)/nncase/* $(STAGING_DIR)/usr/
endef
endif

$(eval $(generic-package))
