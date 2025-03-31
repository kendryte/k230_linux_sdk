include $(WSCFG_KCONFIG_CONFIG)
STRIP := $(WSCFG_CROSS_COMPILE)strip
$(warning defconfig: ================ARCH=$(WSCFG_ARCH_NAME) CROSS_COMPILE=$(WSCFG_CROSS_COMPILE)=======================)
all:
	mkdir -p build && cd build && cmake ../ -DWSCFG_CROSS_COMPILE=${WSCFG_CROSS_COMPILE} && make -j4
	$(STRIP) --strip-unneeded -s build/bp_channel
clean:
	rm -rf build
