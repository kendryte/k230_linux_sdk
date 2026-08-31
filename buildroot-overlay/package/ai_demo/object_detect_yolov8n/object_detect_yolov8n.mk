$(eval $(call ai_mk_common,$(pkgname),$(call UPPERCASE,$(pkgname))))

define OBJECT_DETECT_YOLOV8N_BUILD_DEB
	# 创建目录
	mkdir -p $(@D)/deb/DEBIAN
	mkdir -p $(@D)/deb/root/app/object_detect_yolov8n/

	# 拷贝可执行程序（install(TARGETS ob_det.elf)）
	$(call COPYFILE,$(TARGET_DIR)/root/app/object_detect_yolov8n/ob_det.elf,$(@D)/deb/root/app/object_detect_yolov8n/)

	# 对应 CMakeLists 的 install(FILES/PROGRAMS ...)
	$(call COPYFILE,$(TARGET_DIR)/root/app/object_detect_yolov8n/yolov8n_320.kmodel,$(@D)/deb/root/app/object_detect_yolov8n/)
	$(call COPYFILE,$(TARGET_DIR)/root/app/object_detect_yolov8n/yolov8n_640.kmodel,$(@D)/deb/root/app/object_detect_yolov8n/)
	$(call COPYFILE,$(TARGET_DIR)/root/app/object_detect_yolov8n/bus.jpg,$(@D)/deb/root/app/object_detect_yolov8n/)
	$(call COPYFILE,$(TARGET_DIR)/root/app/object_detect_yolov8n/ob_detect_image_320.sh,$(@D)/deb/root/app/object_detect_yolov8n/)
	$(call COPYFILE,$(TARGET_DIR)/root/app/object_detect_yolov8n/ob_detect_image_640.sh,$(@D)/deb/root/app/object_detect_yolov8n/)
	$(call COPYFILE,$(TARGET_DIR)/root/app/object_detect_yolov8n/ob_detect_isp_320.sh,$(@D)/deb/root/app/object_detect_yolov8n/)
	$(call COPYFILE,$(TARGET_DIR)/root/app/object_detect_yolov8n/ob_detect_isp_640.sh,$(@D)/deb/root/app/object_detect_yolov8n/)

	# 写 control 文件
	echo "Package: k230-object-detect-yolov8n"  >  $(@D)/deb/DEBIAN/control
	echo "Version: 1.0"              			>> $(@D)/deb/DEBIAN/control
	echo "Section: base"             			>> $(@D)/deb/DEBIAN/control
	echo "Priority: optional"        			>> $(@D)/deb/DEBIAN/control
	echo "Architecture: riscv64"     			>> $(@D)/deb/DEBIAN/control
	echo "Maintainer: K230 Dev <dev@example.com>" >> $(@D)/deb/DEBIAN/control
	echo "Description: Object Detect YOLOv8n application for K230" >> $(@D)/deb/DEBIAN/control

	# 打包
	mkdir -p $(BINARIES_DIR)/deb
	dpkg -b $(@D)/deb $(BINARIES_DIR)/deb/$(call LOWERCASE,k230-$(PKG)).deb
endef

OBJECT_DETECT_YOLOV8N_POST_INSTALL_TARGET_HOOKS += OBJECT_DETECT_YOLOV8N_BUILD_DEB

$(eval $(cmake-package))
