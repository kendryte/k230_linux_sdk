USAGE_AI2D_SITE = $(realpath $(TOPDIR))"/package/usage_ai2d"
USAGE_AI2D_SITE_METHOD = local
USAGE_AI2D_DEPENDENCIES += libmmz libnncase gsl-lite

ifeq ($(BR2_PACKAGE_OPENCV4),y)
USAGE_AI2D_DEPENDENCIES += opencv4
endif

define USAGE_AI2D_BUILD_DEB
	# 创建目录
	mkdir -p $(@D)/deb/DEBIAN
	mkdir -p $(@D)/deb/app/

	# 拷贝应用
	$(call COPYFILE,$(TARGET_DIR)/root/app/usage_ai2d,$(@D)/deb/root/app/)

	# 写 control 文件
	echo "Package: k230-usage-ai2d"        >  $(@D)/deb/DEBIAN/control
	echo "Version: 1.0"              			>> $(@D)/deb/DEBIAN/control
	echo "Section: base"             			>> $(@D)/deb/DEBIAN/control
	echo "Priority: optional"        			>> $(@D)/deb/DEBIAN/control
	echo "Architecture: riscv64"     			>> $(@D)/deb/DEBIAN/control
	echo "Maintainer: K230 Dev <dev@example.com>" >> $(@D)/deb/DEBIAN/control
	echo "Description: usage_ai2d application for K230" >> $(@D)/deb/DEBIAN/control

	# 打包
	mkdir -p $(BINARIES_DIR)/deb
	dpkg -b $(@D)/deb $(BINARIES_DIR)/deb/$(call LOWERCASE,k230-$(PKG)).deb
endef

USAGE_AI2D_POST_INSTALL_TARGET_HOOKS += USAGE_AI2D_BUILD_DEB

$(eval $(cmake-package))
