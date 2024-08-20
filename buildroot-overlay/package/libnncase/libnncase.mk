LIBNNCASE_VERSION =
NNCASE_VERSION = v2.9.0
LIBNNCASE_SOURCE = nncase_k230_$(NNCASE_VERSION)_runtime_linux.tgz
LIBNNCASE_SITE = https://github.com/kendryte/nncase/releases/download/$(NNCASE_VERSION)

define LIBNNCASE_EXTRACT_CMDS
	tar zxf $(LIBNNCASE_DL_DIR)/$(LIBNNCASE_SOURCE) -C $(@D)
	mv $(@D)/nncase_k230_$(NNCASE_VERSION)_runtime_linux $(@D)/nncase
endef

define LIBNNCASE_INSTALL_TARGET_CMDS
	cp -r $(@D)/nncase/* $(STAGING_DIR)/usr/
endef

$(eval $(generic-package))