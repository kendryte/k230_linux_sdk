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
PYTHON_K230_DEPENDENCIES = display python-numpy python-pybind opencv4

PYTHON_K230_ENV = \
    CFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/drm -I$(STAGING_DIR)/usr/lib/python3.13/site-packages/numpy/core/include -I$(STAGING_DIR)/usr/lib/python3.13/site-packages/pybind11/include -I$(STAGING_DIR)/usr/include/opencv4" \
    CXXFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/drm -I$(STAGING_DIR)/usr/lib/python3.13/site-packages/numpy/core/include -I$(STAGING_DIR)/usr/lib/python3.13/site-packages/pybind11/include -I$(STAGING_DIR)/usr/include/opencv4 -I$(STAGING_DIR)/usr/include"

define PYTHON_K230_INSTALL_PY_DEMO
	cp -rf  $(@D)/py_demo  $(TARGET_DIR)/root/
endef

PYTHON_K230_POST_INSTALL_TARGET_HOOKS += PYTHON_K230_INSTALL_PY_DEMO

$(eval $(python-package))
