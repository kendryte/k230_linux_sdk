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


.PHONY: all buildroot  debian ubuntu openouler  ruyi  debian_rootfs ubuntu_rootfs
all :  buildroot

debian ubuntu openouler debian_rootfs ubuntu_rootfs : sync
	@$(BR_SRC_DIR)/board/canaan/k230-soc/distribution.sh  $@  $(BRW_BUILD_DIR)

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
	@echo "often use command example:"
	@echo "    make uboot-rebuild  #rebuild uboot"
	@echo "    make uboot-dirclean #uboot clean"
	@echo "    make linux-rebuild  #rebuild linux"
	@echo "    make linux-dirclean #linux clean"
	@echo "    make list_def      #show support config,and current use config"
	@echo ""
	@echo "dcoker build and run example:"
	@echo "    docker  build   -f tools/docker/Dockerfile  -t wjx/d tools/docker "
	@echo '    docker run -it --rm  -h k230  -e uid=$$(id -u) -e gid=$$(id -g) -e user=$${USER} -v $$HOME:$$HOME  -v /opt/toolchain:/opt/toolchain -w $$(pwd) wjx/d:latest '
	@echo ""
	@echo "sdk build usage example:"
	@echo "    make CONF=k230_canmv_defconfig   BR2_PRIMARY_SITE=https://kendryte-download.canaan-creative.com/k230/downloads/dl/"
	@echo "          #note:k230_canmv_defconfig need replace, BR2_PRIMARY_SITE=xxxx is option"
	@echo ""
	@echo ""

.PHONY:list_def
list_def:
	@echo "current config:"
	@echo "	$$(cat .last_conf | cut -d = -f2)"
	@echo "Available all configs and board note:"
	@echo "	k230_canmv_defconfig                  --canmv 1.0/1.1 board"
	@echo "	k230_canmv_v3_defconfig               --canmv v3 board"
	@echo "	k230_canmv_01studio_defconfig         --01studio board"
	@echo "	k230_canmv_dongshanpi_defconfig       --dongshanpi board"
	@echo "	k230_canmv_lckfb_defconfig            --lushanpi ,jialichaung board"
	@echo "	BPI-CanMV-K230D-Zero_defconfig        --bananapi k230d"
	@echo "	k230d_canmv_ilp32_defconfig           --k230d canmv new32 board,plct use"
	@echo "	k230d_canmv_defconfig                 --k230d canmv board"
	@echo "	BPI-CanMV-K230D-Zero_ilp32_defconfig  --plct use,new 32 board,"
	@echo "	k230_evb_defconfig                    --k230 evb board"
	@echo ""









.PHONY:sync
sync:
	make -f tools/sync.mk sync   BR_SRC_DIR=$(BR_SRC_DIR)  BR_OVERLAY_DIR=$(BR_OVERLAY_DIR)  BR_NAME=$(BR_NAME)

this-makefile := $(lastword $(MAKEFILE_LIST))  all dl help  savedefconfig  sync  %_defconfig   debian ubuntu openouler  debian_rootfs ubuntu_rootfs list_def
$(filter-out $(this-makefile) , $(MAKECMDGOALS)):	$(BRW_BUILD_DIR)/.config
	[ -d $(BRW_BUILD_DIR) ] && make -C $(BRW_BUILD_DIR) $@
	@( if [ $@ = linux-savedefconfig ];then \
		lr="$$(make printvars VARS='LINUX_DIR' | grep LINUX_DIR  | cut -d= -f2 )";\
		cp $${lr}/defconfig $${lr}/arch/riscv/configs/k230_defconfig ; \
	fi )


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
