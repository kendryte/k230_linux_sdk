SHELL=/bin/bash
BR_NAME = buildroot-2024.02.1
BR_SRC_DIR = output/$(BR_NAME)
BR_OVERLAY_DIR = buildroot-overlay

ifeq ("$(origin CONF)", "command line")
$(shell echo CONF=$(CONF)>.last_conf;)
else
$(shell [ -f .last_conf ] || ( echo CONF=k230_canmv_defconfig>.last_conf;) )
endif
include .last_conf
BRW_BUILD_DIR = $(CURDIR)/output/$(CONF)


.PHONY: all buildroot  debian ubuntu openouler  ruyi
all :  buildroot

debian ubuntu openouler : buildroot
	@:

buildroot: $(BRW_BUILD_DIR)/.config
	make -C $(BRW_BUILD_DIR) all

.PHONY:dl
dl:   $(BRW_BUILD_DIR)/.config
	echo "download all source"
	make -C $(BRW_BUILD_DIR) source

.PHONY:help
help:sync
	@echo "sdk build usage:"
	@echo "   make CONF=k230_canmv_defconfig  -build k230 linux sdk user k230_canmv_defconfig config "
	@[ -d $(BR_SRC_DIR) ] && echo "buildroot usage:" || exit 0
	@[ -d $(BR_SRC_DIR) ] && make --no-print-directory -C $(BR_SRC_DIR)  help  || exit 0
	@echo "make uboot-rebuild #rebuild uboot"
	@echo "make uboot-dirclean #uboot clean"
	@echo "make linux-rebuild opensbi-rebuild #rebuild linux,rebuild opensbi"
	@echo "make linux-dirclean #linux clean"
	@echo "sdk build usage:"
	@echo "    make CONF=k230_canmv_defconfig    -build k230 linux sdk user k230_canmv_defconfig"
	@echo "                                      -CONF can be $$(ls $(BR_OVERLAY_DIR)/configs | tr '\n' '/')"
	@echo "dcoker build and run example:"
	@echo "		docker  build   -f tools/docker/Dockerfile  -t wjx/d tools/docker "
	@echo '		docker run -it --rm  -h k230  -e uid=$$(id -u) -e gid=$$(id -g) -e user=$${USER} -v $$HOME:$$HOME  -v /opt/toolchain:/opt/toolchain -w $$(pwd) wjx/d:latest '

.PHONY:sync
sync:
	make -f tools/sync.mk sync   BR_SRC_DIR=$(BR_SRC_DIR)  BR_OVERLAY_DIR=$(BR_OVERLAY_DIR)  BR_NAME=$(BR_NAME)

this-makefile := $(lastword $(MAKEFILE_LIST))  all dl help  savedefconfig  sync  %_defconfig
$(filter-out $(this-makefile) , $(MAKECMDGOALS)):	$(BRW_BUILD_DIR)/.config
	[ -d $(BRW_BUILD_DIR) ] && make -C $(BRW_BUILD_DIR) $@

%_defconfig:  sync
	echo CONF=$@ >.last_conf
	CONF=$@ make -C $(BR_SRC_DIR) $(CONF) O=$(BRW_BUILD_DIR)

savedefconfig:  $(BRW_BUILD_DIR)/.config
	make -C $(BRW_BUILD_DIR) $@
	cp $(BR_SRC_DIR)/configs/$(CONF) $(BR_OVERLAY_DIR)/configs/

$(BRW_BUILD_DIR)/.config: sync
ifeq ("$(origin CONF)", "command line")
	make -C $(BR_SRC_DIR) $(CONF) O=$(BRW_BUILD_DIR)
	touch $@
else
	[ -e "$(BRW_BUILD_DIR)" ] || ( make -C $(BR_SRC_DIR) $(CONF) O=$(BRW_BUILD_DIR) ;touch $@; )
endif

#echo LINUX_OVERRIDE_SRCDIR=/home/wangjianxin/t/linux-xuantie-kernel >output/k230d_canmv_64kernel_32rootfs_defconfig/local.mk
