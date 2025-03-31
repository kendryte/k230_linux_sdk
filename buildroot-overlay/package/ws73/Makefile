CUR_MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_MK_DIR := $(dir $(CUR_MK_PATH))
CUR_MK_DIR := $(realpath $(CUR_MK_DIR))

# basic dir
SDK_DIR := $(CUR_MK_DIR)
OUTPUT_DIR := $(SDK_DIR)/output
OUTPUT_BIN_DIR := $(OUTPUT_DIR)/bin
OUTPUT_BUILD_DIR := $(OUTPUT_DIR)/build
OPEN_SROUCE_DIR := $(SDK_DIR)/open_source
BSLE_SRC_DIR := $(SDK_DIR)/driver/bsle
WIFI_SRC_DIR := $(SDK_DIR)/driver/wifi
PLATFORM_SRC_DIR := $(SDK_DIR)/driver/platform
SCRIPT_DIR := $(SDK_DIR)/build/scripts
CONFIG_DIR := $(SDK_DIR)/build/config
BSLE_DFT_DRIVER_SRC_DIR := $(SDK_DIR)/application/dft
# config file
ACTIVE_KCONFIG_CONFIG := $(SDK_DIR)/.config
DEFAULT_KCONFIG_CONFIG := $(CONFIG_DIR)/ws73_default.config
LIGHT_KCONFIG_CONFIG := $(CONFIG_DIR)/ws73_light.config

# other
define echo_ok
@echo -e "\033[1;32m$(1)\033[0m"
endef

#  导出由Kconfig生成的配置文件路径，供下层Makefile使用
ifneq ($(wildcard $(ACTIVE_KCONFIG_CONFIG)),)
WSCFG_KCONFIG_CONFIG = $(realpath $(ACTIVE_KCONFIG_CONFIG))
else
WSCFG_KCONFIG_CONFIG = $(realpath $(DEFAULT_KCONFIG_CONFIG))
endif
LIGHT_WSCFG_KCONFIG_CONFIG = $(realpath $(LIGHT_KCONFIG_CONFIG))
export WSCFG_KCONFIG_CONFIG
export LIGHT_WSCFG_KCONFIG_CONFIG
export WSCFG_AUTOCONFIG_H = $(OUTPUT_BIN_DIR)/autoconfig.h
export LIGHT_WSCFG_AUTOCONFIG_H = $(OUTPUT_BIN_DIR)/light_autoconfig.h

include $(WSCFG_KCONFIG_CONFIG)
include $(LIGHT_WSCFG_KCONFIG_CONFIG)

export DIR_MAP_CONFIG_FILE = release.mk

ALL_CBB_BUILD_TARGETS := platform wifi
LIGHT_CBB_BUILD_TARGETS := platform_light wifi_light
ALL_CBB_CLEAN_TARGETS := platform_clean wifi_clean
ALL_CBB_BUILD_TARGETS_ANDROID := platform wifi

ifdef WSCFG_BLE_COMPILE_BY_DEFAULT
ALL_CBB_BUILD_TARGETS += ble
ALL_CBB_BUILD_TARGETS += dft_driver
ALL_CBB_BUILD_TARGETS += dft_channel
ALL_CBB_BUILD_TARGETS_ANDROID += ble_android
ALL_CBB_CLEAN_TARGETS += ble_clean ble_android_clean
LIGHT_CBB_BUILD_TARGETS += ble_light
endif

ifdef WSCFG_SLE_COMPILE_BY_DEFAULT
ALL_CBB_BUILD_TARGETS += sle
ALL_CBB_BUILD_TARGETS_ANDROID += sle
LIGHT_CBB_BUILD_TARGETS += sle_light
ALL_CBB_CLEAN_TARGETS += sle_clean
endif

# cpu counts
CPU_NUM := $(shell nproc)

.PHONY: wifi platform ble sle ini hso tools menuconfig ble_android full_build all clean

all: prepare $(ALL_CBB_BUILD_TARGETS)
	$(call echo_ok,all built!)

light: prepare_light $(LIGHT_CBB_BUILD_TARGETS)
	$(call echo_ok,light built!)

clean: $(ALL_CBB_CLEAN_TARGETS) hso_clean
	$(call echo_ok,all clean!)

full_build: prepare ini platform wifi ble_android sle hso
	$(call echo_ok,all built!)

android: prepare $(ALL_CBB_BUILD_TARGETS_ANDROID)
	$(call echo_ok,all android built!)

prepare_light:
	@mkdir -p $(OUTPUT_BIN_DIR) $(OUTPUT_BUILD_DIR)
	@python3 $(SCRIPT_DIR)/hconfig.py $(SDK_DIR)/Kconfig ${LIGHT_WSCFG_KCONFIG_CONFIG} -a ${LIGHT_WSCFG_AUTOCONFIG_H}
	@cp $(OUTPUT_BIN_DIR)/light_autoconfig.h $(OUTPUT_BIN_DIR)/autoconfig.h
	$(call echo_ok,prepare_light built!)

prepare:
	@mkdir -p $(OUTPUT_BIN_DIR) $(OUTPUT_BUILD_DIR)
	@python3 $(SCRIPT_DIR)/hconfig.py $(SDK_DIR)/Kconfig ${WSCFG_KCONFIG_CONFIG} -a ${WSCFG_AUTOCONFIG_H}

ini: prepare
	@cp -f $(CONFIG_DIR)/ws73_cfg_default.ini $(OUTPUT_BIN_DIR)/ws73_cfg.ini
	@python3 $(SCRIPT_DIR)/hconfig_to_ini.py $(CONFIG_DIR)/ini_map.csv $(SDK_DIR)/Kconfig ${WSCFG_KCONFIG_CONFIG} $(OUTPUT_BIN_DIR)/ws73_cfg.ini
	$(call echo_ok,INI file generate success in $(OUTPUT_BIN_DIR)/ws73_cfg.ini!)

ini_light: prepare_light
	@cp -f $(CONFIG_DIR)/ws73_cfg_default.ini $(OUTPUT_BIN_DIR)/ws73_cfg.ini
	@python3 $(SCRIPT_DIR)/hconfig_to_ini.py $(CONFIG_DIR)/ini_map.csv $(SDK_DIR)/Kconfig ${LIGHT_WSCFG_AUTOCONFIG_H} $(OUTPUT_BIN_DIR)/ws73_cfg.ini
	$(call echo_ok,ini_light file generate success in $(OUTPUT_BIN_DIR)/ws73_cfg.ini!)

wifi: prepare
	cd $(WIFI_SRC_DIR) && \
    CONFIG_WSXX_KERNEL_MODULES_BUILD_SUPPORT=yes\
    make -j$(CPU_NUM) && cp -f wifi_soc.ko $(OUTPUT_BIN_DIR)/
	$(call echo_ok,WIFI ko built success in $(OUTPUT_BIN_DIR)/wifi_soc.ko!)

wifi_light: prepare_light
	cd $(WIFI_SRC_DIR) && \
    CONFIG_WSXX_KERNEL_MODULES_BUILD_SUPPORT=yes\
    make -j$(CPU_NUM) && cp -f wifi_soc.ko $(OUTPUT_BIN_DIR)/
	$(call echo_ok, wifi_light ko built success in $(OUTPUT_BIN_DIR)/wifi_soc.ko!)

wifi_clean: prepare
	cd $(WIFI_SRC_DIR) && \
    CONFIG_WSXX_KERNEL_MODULES_BUILD_SUPPORT=yes\
    make clean
	@find $(WIFI_SRC_DIR) -name "*.o" -exec rm {} \;
	@find $(WIFI_SRC_DIR) -name "*.cmd" -exec rm {} \;

platform: prepare ini
	cd $(PLATFORM_SRC_DIR) && \
    BUILD_DEVICE_WITH_ROM_REPO=yes\
    make -j$(CPU_NUM) && cp -f plat_soc.ko $(OUTPUT_BIN_DIR)/
	$(call echo_ok,platform ko built success in $(OUTPUT_BIN_DIR)/plat_soc.ko!)

platform_light: prepare_light ini_light
	cd $(PLATFORM_SRC_DIR) && \
    BUILD_DEVICE_WITH_ROM_REPO=yes\
    make -j$(CPU_NUM) && cp -f plat_soc.ko $(OUTPUT_BIN_DIR)/
	$(call echo_ok,platform_light ko built success in $(OUTPUT_BIN_DIR)/plat_soc.ko!)

platform_clean: prepare
	cd $(PLATFORM_SRC_DIR) && \
    make clean
	@find $(PLATFORM_SRC_DIR) -name "*.o" -exec rm {} \;
	@find $(PLATFORM_SRC_DIR) -name "*.cmd" -exec rm {} \;

ble: prepare
	cd $(BSLE_SRC_DIR)/ble_driver/linux && \
    make -j$(CPU_NUM) && cp -f ble_soc.ko $(OUTPUT_BIN_DIR)/
	$(call echo_ok,ble_linux ko built success in $(OUTPUT_BIN_DIR)/ble_soc.ko!)

dft_driver: prepare
	cd $(BSLE_DFT_DRIVER_SRC_DIR)/bsle_dft_driver && \
    make -j$(CPU_NUM) && cp -f bp_test.ko $(OUTPUT_BIN_DIR)/
	$(call echo_ok,ble_linux ko built success in $(OUTPUT_BIN_DIR)/bp_test.ko!)

dft_channel: prepare
	cd $(BSLE_DFT_DRIVER_SRC_DIR)/bsle_dft_channel/linux && \
    make -j$(CPU_NUM) && cp -f build/bp_channel $(OUTPUT_BIN_DIR)/
	$(call echo_ok,ble_linux ko built success in $(OUTPUT_BIN_DIR)/bp_channel!)
	
ble_light: prepare_light
	cd $(BSLE_SRC_DIR)/ble_driver/linux && \
    make -j$(CPU_NUM) && cp -f ble_soc.ko $(OUTPUT_BIN_DIR)/
	$(call echo_ok,ble_light_linux ko built success in $(OUTPUT_BIN_DIR)/ble_soc.ko!)

ble_clean: prepare
	cd $(BSLE_SRC_DIR)/ble_driver/linux && \
    make clean

ble_android: prepare
	cd $(BSLE_SRC_DIR)/ble_driver/android && \
    make -j$(CPU_NUM) && cp -f ble_soc.ko $(OUTPUT_BIN_DIR)/
	$(call echo_ok,ble_android ko built success in $(OUTPUT_BIN_DIR)/ble_soc.ko!)

ble_android_clean: prepare
	cd $(BSLE_SRC_DIR)/ble_driver/android && \
    make clean

sle: prepare
	cd $(BSLE_SRC_DIR)/sle_driver && \
    make -j$(CPU_NUM) && cp -f sle_soc.ko $(OUTPUT_BIN_DIR)/
	$(call echo_ok,sle ko built success in $(OUTPUT_BIN_DIR)/sle_soc.ko!)

sle_light: prepare_light
	cd $(BSLE_SRC_DIR)/sle_driver && \
    make -j$(CPU_NUM) && cp -f sle_soc.ko $(OUTPUT_BIN_DIR)/
	$(call echo_ok,sle_light ko built success in $(OUTPUT_BIN_DIR)/sle_soc.ko!)

sle_clean: prepare
	cd $(BSLE_SRC_DIR)/sle_driver && \
    make clean

hso_clean: prepare
	@rm -rf $(OUTPUT_DIR)/database_es0

hso: prepare
	@echo "Generate hdb..."
	@cp -rf $(CONFIG_DIR)/hso/database_es0 $(OUTPUT_DIR)/
	@python3 $(SCRIPT_DIR)/build_host_hso.py -c $(CONFIG_DIR)/hso/generate_hdb.ini
	$(call echo_ok,hso generate success in $(OUTPUT_DIR)/database_es0!)

tools_clean: prepare
	@make -f $(OPEN_SROUCE_DIR)/open_source.mk\
    SDK_OUTPUT_BUILD_DIR=$(OUTPUT_BUILD_DIR)\
    SDK_OUTPUT_BIN_DIR=$(OUTPUT_BIN_DIR) clean
	
tools_clean_light: prepare
	@make -f $(OPEN_SROUCE_DIR)/open_source_light.mk\
    SDK_OUTPUT_BUILD_DIR=$(OUTPUT_BUILD_DIR)\
    SDK_OUTPUT_BIN_DIR=$(OUTPUT_BIN_DIR) clean

tools: prepare
	@make -j$(CPU_NUM) -f $(OPEN_SROUCE_DIR)/open_source.mk\
    SDK_OUTPUT_BUILD_DIR=$(OUTPUT_BUILD_DIR)\
    SDK_OUTPUT_BIN_DIR=$(OUTPUT_BIN_DIR)
	$(call echo_ok,open source tools built success!)
	
tools_light: prepare
	@make -j$(CPU_NUM) -f $(OPEN_SROUCE_DIR)/open_source_light.mk\
    SDK_OUTPUT_BUILD_DIR=$(OUTPUT_BUILD_DIR)\
    SDK_OUTPUT_BIN_DIR=$(OUTPUT_BIN_DIR)
	$(call echo_ok,open source tools built success!)

