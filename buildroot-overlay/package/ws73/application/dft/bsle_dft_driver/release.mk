CUR_MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_MK_DIR := $(dir $(CUR_MK_PATH))
CUR_MK_DIR := $(realpath $(CUR_MK_DIR))

# 计算第一个参数相对于第二个参数的相对路径
define calc_relpath
$(shell python3 -c "import os.path; print(os.path.relpath('$(1)','$(2)'))")
endef

# 项目顶层目录（不要其他Makefile中使用）
_TOP_DIR := $(realpath $(CUR_MK_DIR))
_PLATFORM_DIR := $(realpath $(CUR_MK_DIR)/../../../driver/platform)

# 平台目录
PLATFORM_INC_DIR := $(_PLATFORM_DIR)/inc
PLATFORM_CFG_DIR := $(_PLATFORM_DIR)/cfg
PLATFORM_DRV_DIR := $(_PLATFORM_DIR)/drv
PLATFORM_DEVICE_DIR := $(_PLATFORM_DIR)/drv/device
PLATFORM_HCC_DIR := $(_PLATFORM_DIR)/hcc
PLATFORM_OSAL_DIR := $(_PLATFORM_DIR)/osal
PLATFORM_LIBC_SEC_DIR := $(_PLATFORM_DIR)/libc_sec
