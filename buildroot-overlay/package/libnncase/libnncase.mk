LIBNNCASE_SITE = $(realpath $(TOPDIR))"/package/libnncase"
LIBNNCASE_SITE_METHOD = local

define LIBNNCASE_BUILD_CMDS
wget https://ai.b-bug.org/~zhaozhongxiang/runtime_lib.tar.bz2
endef

define LIBNNCASE_INSTALL_TARGET_CMDS
tar xvf runtime_lib.tar.bz2 -C $(@D)
endef

$(eval $(generic-package))