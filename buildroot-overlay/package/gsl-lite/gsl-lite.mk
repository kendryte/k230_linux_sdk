################################################################################
#
# gsl-lite
#
################################################################################

GSL_LITE_VERSION = v0.41.0
GSL_LITE_SOURCE = $(GSL_LITE_VERSION).tar.gz
GSL_LITE_SITE = https://github.com/gsl-lite/gsl-lite/archive/refs/tags
GSL_LITE_INSTALL_STAGING = YES

$(eval $(cmake-package))
