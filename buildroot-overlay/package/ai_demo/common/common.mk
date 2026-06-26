COMMON_INSTALL_STAGING = YES
COMMON_INSTALL_TARGET = NO


COMMON_SITE := https://download.kendryte.com/k230/downloads/kmodel

COMMON_SOURCE := kmodel_v2.10.0.tgz
COMMON_DEPENDENCIES += libmmz libnncase gsl-lite


ifeq ($(OS), common)
	COMMON_DEPENDENCIES += common
endif

ifeq ($(BR2_PACKAGE_OPENCV4), y)
	COMMON_DEPENDENCIES += opencv4 display vvcam
endif


COMMON_LOCAL_SRC := $(call qstrip,$(pkgdir))



# Generate a tarball if one does not already exist.
define COMMON_DOWNLOAD_KMODE_FILE
	rsync -au --chmod=u=rwX,go=rX $($(PKG)_OVERRIDE_SRCDIR_RSYNC_EXCLUSIONS) $(RSYNC_VCS_EXCLUSIONS) $(COMMON_LOCAL_SRC)/ $(@D)
	mkdir -p $(@D)/shell;rsync -au --chmod=u=rwX,go=rX $($(PKG)_OVERRIDE_SRCDIR_RSYNC_EXCLUSIONS) $(RSYNC_VCS_EXCLUSIONS) package/ai_demo/shell $(@D)/
endef
COMMON_POST_DOWNLOAD_HOOKS += COMMON_DOWNLOAD_KMODE_FILE


$(eval $(cmake-package))
