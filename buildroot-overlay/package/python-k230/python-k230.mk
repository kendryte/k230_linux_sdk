################################################################################
#
# python-k230
#
################################################################################

PYTHON_K230_VERSION = 1.0.0
PYTHON_K230_SITE = $(TOPDIR)/package/python-k230
PYTHON_K230_SITE_METHOD = local
PYTHON_K230_SETUP_TYPE = setuptools
PYTHON_K230_LICENSE = MIT
PYTHON_K230_LICENSE_FILES = setup.py

define PYTHON_K230_INSTALL_PY_DEMO
cp -rf  $(@D)/py_demo  $(TARGET_DIR)/root/
endef

PYTHON_K230_POST_INSTALL_TARGET_HOOKS += PYTHON_K230_INSTALL_PY_DEMO

$(eval $(python-package))
