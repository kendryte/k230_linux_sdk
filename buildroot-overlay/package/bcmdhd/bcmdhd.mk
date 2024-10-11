################################################################################
#
# bcmdhd
#
################################################################################

BCMDHD_SITE = $(realpath $(TOPDIR))"/package/bcmdhd"
BCMDHD_SITE_METHOD = local
BCMDHD_INSTALL_STAGING = YES
BCMDHD_INSTALL_TARGET = YES
BCMDHD_SUPPORTS_IN_SOURCE_BUILD = NO

BCMDHD_MODULE_MAKE_OPTS += CONFIG_BCMDHD=m
BCMDHD_MODULE_MAKE_OPTS += CONFIG_BCMDHD_FW_PATH="/etc/firmware/fw_bcmdhd.bin"
BCMDHD_MODULE_MAKE_OPTS += CONFIG_BCMDHD_NVRAM_PATH="/etc/firmware/nvram.txt"
BCMDHD_MODULE_MAKE_OPTS += CONFIG_BCMDHD_SDIO=y
BCMDHD_MODULE_MAKE_OPTS += CONFIG_BCMDHD_SDIO_IRQ=y

$(eval $(kernel-module))
$(eval $(generic-package))
