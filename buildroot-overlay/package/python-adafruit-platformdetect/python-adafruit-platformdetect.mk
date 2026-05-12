################################################################################
#
# python-adafruit-platformdetect
#
################################################################################

PYTHON_ADAFRUIT_PLATFORMDETECT_VERSION = 3.88.0
PYTHON_ADAFRUIT_PLATFORMDETECT_SITE = $(call github,adafruit,Adafruit_Python_PlatformDetect,$(PYTHON_ADAFRUIT_PLATFORMDETECT_VERSION))
PYTHON_ADAFRUIT_PLATFORMDETECT_SETUP_TYPE = setuptools
PYTHON_ADAFRUIT_PLATFORMDETECT_LICENSE = MIT
PYTHON_ADAFRUIT_PLATFORMDETECT_LICENSE_FILES = LICENSE

$(eval $(python-package))
