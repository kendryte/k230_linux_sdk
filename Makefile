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


.PHONY: all buildroot  debian ubuntu openouler
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

.PHONY:sync
sync:
	make -f tools/sync.mk sync

this-makefile := $(lastword $(MAKEFILE_LIST))  all dl help  savedefconfig  sync
$(filter-out $(this-makefile) , $(MAKECMDGOALS)):	$(BRW_BUILD_DIR)/.config
	[ -d $(BRW_BUILD_DIR) ] && make -C $(BRW_BUILD_DIR) $@

%_defconfig:  sync
	echo CONF=$@ >.last_conf

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

