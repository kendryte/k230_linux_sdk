################################################################################
#
# Linux kernel target
#
################################################################################
LINUX_ILP32_VERSION = $(call qstrip,$(BR2_LINUX_KERNEL_VERSION))
LINUX_ILP32_LICENSE = GPL-2.0


# Compute LINUX_SOURCE and LINUX_SITE from the configuration
LINUX_ILP32_SITE = $(call qstrip,$(BR2_LINUX_KERNEL_CUSTOM_REPO_URL))
LINUX_ILP32_SITE_METHOD = git

LINUX_ILP32_DEPENDENCIES += linux

# LINUX_ILP32_KCONFIG_OPTS =  $(LINUX_MAKE_FLAGS) HOSTCC="$(HOSTCC_NOCCACHE)"
#BR2_TOOLCHAIN_EXTERNAL_RUYI_NEW32_RV64ILP32_BIN_PATH
#BR2_TOOLCHAIN_EXTERNAL_RUYI_NEW32_RV64ILP32_PREFIX
RYSDI_NEW32_TOOLCHAIN = $(call qstrip,$(BR2_TOOLCHAIN_EXTERNAL_RUYI_NEW32_RV64ILP32_BIN_PATH))/bin/$(call qstrip,$(BR2_TOOLCHAIN_EXTERNAL_RUYI_NEW32_RV64ILP32_PREFIX))-

# LINUX_ILP32_KCONFIG_DEFCONFIG = $(LINUX_KCONFIG_DEFCONFIG)
# #linux/linux_ilp32/64ilp32.config
# LINUX_ILP32_KCONFIG_FRAGMENT_FILES = linux/linux_ilp32/64ilp32.config

# Compilation. We make sure the kernel gets rebuilt when the
# configuration has changed. We call the 'all' and
# '$(LINUX_TARGET_NAME)' targets separately because calling them in
# the same $(BR2_MAKE) invocation has shown to cause parallel build
# issues.
# The call to disable gcc-plugins is a stop-gap measure:
#   http://lists.busybox.net/pipermail/buildroot/2020-May/282727.html
define LINUX_ILP32_BUILD_CMDS
	CROSS_COMPILE=$(RYSDI_NEW32_TOOLCHAIN) ARCH=riscv $(BR2_MAKE)  -C $(@D)  $(LINUX_KCONFIG_DEFCONFIG)  64ilp32.config
	CROSS_COMPILE=$(RYSDI_NEW32_TOOLCHAIN) ARCH=riscv $(BR2_MAKE)  -C $(@D)  all $(LINUX_TARGET_NAME)	
endef

define LINUX_ILP32_INSTALL_TARGET_CMDS
endef 
LINUX_ILP32_INSTALL_IMAGES=YES
define LINUX_ILP32_INSTALL_IMAGES_CMDS
	cp $(@D)/arch/riscv/boot/Image $(BINARIES_DIR)/Image_ilp32
endef
#$(eval $(kconfig-package))
$(eval $(generic-package))
#$(eval $(virtual-package))

