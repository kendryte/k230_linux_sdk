# YOLO_SITE = $(realpath $(TOPDIR))"/package/yolo"
# YOLO_SITE_METHOD = local
# YOLO_DEPENDENCIES += libmmz libnncase gsl-lite

# ifeq ($(BR2_PACKAGE_OPENCV4),y)
# YOLO_DEPENDENCIES += opencv4 display vvcam
# endif

# define YOLO_BUILD_DEB
# 	$(call COPYFILE ,$(TARGET_DIR)/root/app/yolo,$(@D)/deb/app/)
# 	dpkg -b  $(@D)/deb  $(BINARIES_DIR)/deb/$(call LOWERCASE, k230-$(PKG)).deb
# endef

# YOLO_POST_INSTALL_TARGET_HOOKS += YOLO_BUILD_DEB

# $(eval $(cmake-package))
YOLO_SITE = $(realpath $(TOPDIR))"/package/yolo"
YOLO_SITE_METHOD = local
YOLO_DEPENDENCIES += libmmz libnncase gsl-lite

ifeq ($(BR2_PACKAGE_OPENCV4),y)
YOLO_DEPENDENCIES += opencv4 display vvcam
endif

define YOLO_BUILD_DEB
	# 创建目录
	mkdir -p $(@D)/deb/DEBIAN
	mkdir -p $(@D)/deb/app/

	# 拷贝应用
	$(call COPYFILE,$(TARGET_DIR)/root/app/yolo,$(@D)/deb/root/app/)

	# 写 control 文件
	echo "Package: k230-yolo"        >  $(@D)/deb/DEBIAN/control
	echo "Version: 1.0"              >> $(@D)/deb/DEBIAN/control
	echo "Section: base"             >> $(@D)/deb/DEBIAN/control
	echo "Priority: optional"        >> $(@D)/deb/DEBIAN/control
	echo "Architecture: riscv64"     >> $(@D)/deb/DEBIAN/control
	echo "Maintainer: K230 Dev <dev@example.com>" >> $(@D)/deb/DEBIAN/control
	echo "Description: YOLO application for K230" >> $(@D)/deb/DEBIAN/control

	# 打包
	mkdir -p $(BINARIES_DIR)/deb
	dpkg -b $(@D)/deb $(BINARIES_DIR)/deb/$(call LOWERCASE,k230-$(PKG)).deb
endef

YOLO_POST_INSTALL_TARGET_HOOKS += YOLO_BUILD_DEB

$(eval $(cmake-package))
