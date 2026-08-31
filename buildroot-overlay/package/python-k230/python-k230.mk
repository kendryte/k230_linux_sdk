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
PYTHON_K230_DEPENDENCIES = display vvcam python-numpy python-pybind opencv4 lvgl libnncase libmmz gsl-lite json-for-modern-cpp

# Derive short version (e.g. "3.13") from buildroot's PYTHON3_VERSION ("3.13.x")
PYTHON3_VERSION_SHORT = $(basename $(PYTHON3_VERSION))
PYTHON3_SITEPACKAGES = $(STAGING_DIR)/usr/lib/python$(PYTHON3_VERSION_SHORT)/site-packages

PYTHON_K230_ENV = \
    CFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/drm -I$(STAGING_DIR)/usr/include/lvgl -I$(STAGING_DIR)/usr/include/lvgl_private -I$(PYTHON3_SITEPACKAGES)/numpy/core/include -I$(PYTHON3_SITEPACKAGES)/pybind11/include -I$(STAGING_DIR)/usr/include/opencv4 -I$(STAGING_DIR)/usr/include/libv4l2-drm" \
    CXXFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/drm -I$(STAGING_DIR)/usr/include/lvgl -I$(STAGING_DIR)/usr/include/lvgl_private -I$(PYTHON3_SITEPACKAGES)/numpy/core/include -I$(PYTHON3_SITEPACKAGES)/pybind11/include -I$(STAGING_DIR)/usr/include/opencv4 -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/libv4l2-drm" \
    STAGING_DIR="$(STAGING_DIR)" \
    LDFLAGS="-L$(STAGING_DIR)/usr/lib"

define PYTHON_K230_INSTALL_PY_DEMO
	cp -rf  $(@D)/py_demo  $(TARGET_DIR)/root/
endef

PYTHON_K230_POST_INSTALL_TARGET_HOOKS += PYTHON_K230_INSTALL_PY_DEMO

$(eval $(python-package))
