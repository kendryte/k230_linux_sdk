CUR_MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_MK_DIR := $(dir $(CUR_MK_PATH))
CUR_MK_DIR := $(realpath $(CUR_MK_DIR))

# 计算第一个参数相对于第二个参数的相对路径
define calc_relpath
$(shell python3 -c "import os.path; print(os.path.relpath('$(1)','$(2)'))")
endef

# 项目顶层目录（不要其他Makefile中使用）
_TOP_DIR := $(realpath $(CUR_MK_DIR))

# 平台其他目录
ALGORITHM_DIR := $(_TOP_DIR)/drv/algorithm

# 业务日志处理相关目录
ZDIAG_DIR := $(_TOP_DIR)/diag
DIAG_DIR := $(_TOP_DIR)/diag/diag
ZDIAG_ADAPT_DIR := $(_TOP_DIR)/diag/zdiag_adapt
DFX_DIR := $(_TOP_DIR)/diag/dfx
ZDIAG_ADAPT_INCLUDE := $(_TOP_DIR)/diag/zdiag_adapt/include
DFX_ZDIAG_ROMABLE_DIR := $(_TOP_DIR)/diag/dfx/zdiag/romable

# 平台device侧目录
PLATFORM_DEVICE_DIR := $(_TOP_DIR)/drv/device

# USB/PCIe/SDIO驱动目录
DRV_DIR := $(_TOP_DIR)/drv

# device与host通行实现目录
HCC_DIR := $(_TOP_DIR)/hcc

# 头文件目录
INC_DIR := $(_TOP_DIR)/inc

# 安全C库函数实现目录
LIBC_SEC_DIR := $(_TOP_DIR)/libc_sec

# GPIO管理及上下电实现目录
MAIN_DIR := $(_TOP_DIR)/main

# os适配层目录
OSAL_DIR := $(_TOP_DIR)/osal
OSAL_ADAPT_DIR := $(_TOP_DIR)/osal/osal_adapt

# 电源管理
PM_DIR := $(_TOP_DIR)/pm

# 配置文件操作实现目录
CFG_DIR := $(_TOP_DIR)/cfg

# 异常处理实现目录
EXCE_DIR := $(_TOP_DIR)/exce

# 固件下载实现目录
FIRMWARE_DOWNLOAD_DIR := $(_TOP_DIR)/firmware_download