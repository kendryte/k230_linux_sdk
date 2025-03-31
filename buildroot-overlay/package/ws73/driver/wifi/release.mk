CUR_MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_MK_DIR := $(dir $(CUR_MK_PATH))
CUR_MK_DIR := $(realpath $(CUR_MK_DIR))

# 计算第一个参数相对于第二个参数的相对路径
define calc_relpath
$(shell python3 -c "import os.path; print(os.path.relpath('$(1)','$(2)'))")
endef

# 内置目录（不要在其他Makefile中使用）
_TOP_DIR := $(realpath $(CUR_MK_DIR))
_WIFI_DIR := $(_TOP_DIR)
_PLATFORM_DIR := $(realpath $(CUR_MK_DIR)/../platform)

# 平台相关目录
PLATFROM_DRV_DIR := $(_PLATFORM_DIR)/drv
PLATFROM_LIBC_SEC_DIR := $(_PLATFORM_DIR)/libc_sec
PLATFROM_OSAL_DIR := $(_PLATFORM_DIR)/osal
PLATFROM_DFX_DIR := $(_PLATFORM_DIR)/diag/dfx
PLATFROM_HCC_DIR := $(_PLATFORM_DIR)/hcc
PLATFROM_PM_DIR := $(_PLATFORM_DIR)/pm
PLATFROM_ZDIAG_DIR := $(_PLATFORM_DIR)/diag
PLATFROM_ZDIAG_ADAPT_DIR := $(_PLATFORM_DIR)/diag/zdiag_adapt
PLATFORM_INC_DIR := $(_PLATFORM_DIR)/inc
PLATFORM_DEVICE_DIR := $(_PLATFORM_DIR)/drv/device

# wifi device目录
WIFI_DEVICE_DIR := $(_WIFI_DIR)/device

# 算法目录
ALG_DIR := $(_WIFI_DIR)/alg

# 定制参数配置目录
COMMON_DIR := $(_WIFI_DIR)/common

# 前端射频校准目录
FE_DIR := $(_WIFI_DIR)/fe

# wifi特性目录
FEATURE_DIR := $(_WIFI_DIR)/feature

# 转发代码目录
FORWARD_DIR := $(_WIFI_DIR)/forward

# Framework公共框架
FRW_DIR := $(_WIFI_DIR)/frw

# 驱动层目录
HAL_DIR := $(_WIFI_DIR)/hal

# Hmac层目录
HMAC_DIR := $(_WIFI_DIR)/hmac

# 对外公共文件目录
INC_DIR := $(_WIFI_DIR)/inc

# Mac层相关代码目录
MAC_DIR := $(_WIFI_DIR)/mac

# 初始化相关代码目录
MAIN_DIR := $(_WIFI_DIR)/main

# wifi代码封装的os相关代码目录
OAL_DIR := $(_WIFI_DIR)/oal

# 代码版本目录
VERSION_DIR := $(_WIFI_DIR)/version

# wifi适配层目录
WAL_DIR := $(_WIFI_DIR)/wal