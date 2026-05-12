################################################################################
#
# python-adafruit-pureio
#
################################################################################

PYTHON_ADAFRUIT_PUREIO_VERSION = 1.1.11
PYTHON_ADAFRUIT_PUREIO_SITE = $(call github,adafruit,Adafruit_Python_PureIO,$(PYTHON_ADAFRUIT_PUREIO_VERSION))
PYTHON_ADAFRUIT_PUREIO_SETUP_TYPE = setuptools
PYTHON_ADAFRUIT_PUREIO_LICENSE = MIT
PYTHON_ADAFRUIT_PUREIO_LICENSE_FILES = LICENSE

$(eval $(python-package))
