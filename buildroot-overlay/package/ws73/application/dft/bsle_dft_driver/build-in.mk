CUR_MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_MK_DIR := $(dir $(CUR_MK_PATH))
CUR_MK_DIR := $(realpath $(CUR_MK_DIR))

# 计算第一个参数相对于第二个参数的相对路径
define calc_relpath
$(shell realpath $(1) --relative-to=$(2))
endef

# 项目顶层目录（不要其他Makefile中使用）
_TOP_DIR := $(realpath $(CUR_MK_DIR)/../../../../../../)

# 平台目录
PLATFORM_INC_DIR := $(_TOP_DIR)/host/wifi/platform/inc
PLATFORM_CFG_DIR := $(_TOP_DIR)/host/common/customize
PLATFORM_DRV_DIR := $(_TOP_DIR)/host/wifi/platform/driver
PLATFORM_DEVICE_DIR := $(_TOP_DIR)/device/soc/vendor/ws73
PLATFORM_HCC_DIR := $(_TOP_DIR)/utils/hcc
PLATFORM_OSAL_DIR := $(_TOP_DIR)/utils/osal
PLATFORM_LIBC_SEC_DIR := $(_TOP_DIR)/utils/securec
