################################################################################
#
# python-adafruit-blinka
#
################################################################################

PYTHON_ADAFRUIT_BLINKA_VERSION = 9.1.0
PYTHON_ADAFRUIT_BLINKA_SITE = $(call github,adafruit,Adafruit_Blinka,$(PYTHON_ADAFRUIT_BLINKA_VERSION))
PYTHON_ADAFRUIT_BLINKA_SETUP_TYPE = setuptools
PYTHON_ADAFRUIT_BLINKA_LICENSE = MIT
PYTHON_ADAFRUIT_BLINKA_LICENSE_FILES = LICENSE

# Remove pre-compiled binaries for other architectures
define PYTHON_ADAFRUIT_BLINKA_REMOVE_NON_RISCV_BINARIES
	rm -f $(TARGET_DIR)/usr/lib/python3.13/site-packages/adafruit_blinka/microcontroller/amlogic/meson_g12_common/pulseio/libgpiod_pulsein
	rm -f $(TARGET_DIR)/usr/lib/python3.13/site-packages/adafruit_blinka/microcontroller/bcm283x/pulseio/libgpiod_pulsein
	rm -f $(TARGET_DIR)/usr/lib/python3.13/site-packages/adafruit_blinka/microcontroller/bcm283x/pulseio/libgpiod_pulsein64
endef

PYTHON_ADAFRUIT_BLINKA_POST_INSTALL_TARGET_HOOKS += PYTHON_ADAFRUIT_BLINKA_REMOVE_NON_RISCV_BINARIES

$(eval $(python-package))
