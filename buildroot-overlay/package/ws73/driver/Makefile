# for android build system

CUR_MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_MK_DIR := $(dir $(CUR_MK_PATH))
CUR_MK_DIR := $(realpath $(CUR_MK_DIR))

export DIR_MAP_CONFIG_FILE=release.mk
export WSCFG_KCONFIG_CONFIG=$(CUR_MK_DIR)/android.config
export WSCFG_AUTOCONFIG_H = $(CUR_MK_DIR)/android_autoconfig.h
export CONFIG_WSXX_KERNEL_MODULES_BUILD_SUPPORT=yes

obj-y += platform/ wifi/ bsle/ble_driver/android/