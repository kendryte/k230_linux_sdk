CAMERA_RTSP_DEMO_SITE = $(realpath $(TOPDIR))"/package/camera_rtsp_demo/src"
CAMERA_RTSP_DEMO_SITE_METHOD = local
CAMERA_RTSP_DEMO_INSTALL_STAGING = YES

CAMERA_RTSP_DEMO_DEPENDENCIES += librtsp_server ffmpeg  libevent

define CAMERA_RTSP_DEMO_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CPP="$(TARGET_CXX)" CFLAGS="$(TARGET_CXXFLAGS)" -C $(@D)
endef

define CAMERA_RTSP_DEMO_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/camera_rtsp_demo $(TARGET_DIR)/root/app/camera_rtsp_demo
endef




define CAMERA_RTSP_DEMO_BUILD_DEB
	# 创建目录
	mkdir -p $(@D)/deb/DEBIAN
	mkdir -p $(@D)/deb/root/app/

	# 拷贝应用
	$(call COPYFILE,$(TARGET_DIR)/root/app/camera_rtsp_demo, $(@D)/deb/root/app/)


	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/librtsp_server.so ,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libUsageEnvironment.so.3 ,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libBasicUsageEnvironment.so.1 ,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	#$(call COPYFILE ,$(TARGET_DIR)/usr/lib/librtsp_server ,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libgroupsock.so.30 ,$(@D)/deb/usr/lib/riscv64-linux-gnu/)
	$(call COPYFILE ,$(TARGET_DIR)/usr/lib/libliveMedia.so.94 ,$(@D)/deb/usr/lib/riscv64-linux-gnu/)


	# 写 control 文件
	echo "Package: k230-camera-rtsp"        >  $(@D)/deb/DEBIAN/control
	echo "Version: 1.0"              			>> $(@D)/deb/DEBIAN/control
	echo "Section: base"             			>> $(@D)/deb/DEBIAN/control
	echo "Priority: optional"        			>> $(@D)/deb/DEBIAN/control
	echo "Architecture: riscv64"     			>> $(@D)/deb/DEBIAN/control
	echo "Maintainer: K230 Dev <dev@example.com>" >> $(@D)/deb/DEBIAN/control
	echo "Description: camera rtsp application for K230" >> $(@D)/deb/DEBIAN/control

	# 打包
	mkdir -p $(BINARIES_DIR)/deb
	dpkg -b $(@D)/deb $(BINARIES_DIR)/deb/$(call LOWERCASE,k230-$(PKG)).deb
endef


CAMERA_RTSP_DEMO_POST_INSTALL_TARGET_HOOKS +=   CAMERA_RTSP_DEMO_BUILD_DEB

$(eval $(generic-package))
