CUR_MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_MK_DIR := $(dir $(CUR_MK_PATH))
CUR_MK_DIR := $(realpath $(CUR_MK_DIR))

include $(CUR_MK_DIR)/$(DIR_MAP_CONFIG_FILE)
include $(WSCFG_KCONFIG_CONFIG)

# get relative path
REL_DRV_DIR := $(call calc_relpath,$(DRV_DIR),$(CUR_MK_DIR))
REL_HCC_DIR := $(call calc_relpath,$(HCC_DIR),$(CUR_MK_DIR))
REL_LIBC_SEC_DIR := $(call calc_relpath,$(LIBC_SEC_DIR),$(CUR_MK_DIR))
REL_MAIN_DIR := $(call calc_relpath,$(MAIN_DIR),$(CUR_MK_DIR))
REL_OSAL_DIR := $(call calc_relpath,$(OSAL_DIR),$(CUR_MK_DIR))
REL_OSAL_ADAPT_DIR := $(call calc_relpath,$(OSAL_ADAPT_DIR),$(CUR_MK_DIR))
REL_PM_DIR := $(call calc_relpath,$(PM_DIR),$(CUR_MK_DIR))
REL_DFX_ZDIAG_ROMABLE_DIR := $(call calc_relpath,$(DFX_ZDIAG_ROMABLE_DIR),$(CUR_MK_DIR))
REL_DFX_DIR := $(call calc_relpath,$(DFX_DIR),$(CUR_MK_DIR))
REL_ZDIAG_ADAPT_DIR := $(call calc_relpath,$(ZDIAG_ADAPT_DIR),$(CUR_MK_DIR))
REL_ZDIAG_DIR := $(call calc_relpath,$(ZDIAG_DIR),$(CUR_MK_DIR))
REL_DIAG_DIR := $(call calc_relpath,$(DIAG_DIR),$(CUR_MK_DIR))
REL_ALGORITHM_DIR := $(call calc_relpath,$(ALGORITHM_DIR),$(CUR_MK_DIR))
REL_CFG_DIR := $(call calc_relpath,$(CFG_DIR),$(CUR_MK_DIR))
REL_EXCE_DIR := $(call calc_relpath,$(EXCE_DIR),$(CUR_MK_DIR))
REL_FIRMWARE_DOWNLOAD_DIR := $(call calc_relpath,$(FIRMWARE_DOWNLOAD_DIR),$(CUR_MK_DIR))

ifeq ($(strip $(WSCFG_KERNEL_DIR)),)
$(error ERR: WSCFG_KERNEL_DIR is NULL)
endif

KO_NAME := plat_soc

include $(CUR_MK_DIR)/ws73_comm_defconfig
include $(CUR_MK_DIR)/plat_ws73_default_defconfig
include $(HCC_DIR)/build/linux/Makefile

##################################################
all-src :=
src-list :=
SRC_INCLUDES := $(HCC_SRC_INCLUDES)
############### Add src here #####################
src-list += $(REL_MAIN_DIR)
src-list += $(hcc-src-list)

SRC_INCLUDES += -I$(DFX_DIR)/include

all-src += $(REL_ZDIAG_ADAPT_DIR)/hcc_diag_log.c
ifeq ($(WSCFG_PLAT_DIAG_LOG_OUT), y)
PLAT_DEBUG_CFLAGS = -DCONFIG_PLAT_SUPPORT_OAL_PRINT
all-src += $(REL_DFX_DIR)/zdiag/zdiag_adapt_layer.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_channel.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_cmd_dst.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_cmd_src.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_filter.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_ind_dst.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_ind_src.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_mem.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_pkt.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_pkt_router.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_rx_api.c
all-src += $(REL_DFX_DIR)/zdiag/zdiag_tx.c

src-list += $(REL_ALGORITHM_DIR)/crc/

src-list += $(REL_DFX_ZDIAG_ROMABLE_DIR)

SRC_INCLUDES += -I$(DFX_DIR)/zdiag
SRC_INCLUDES += -I$(DFX_DIR)/zdiag/include
SRC_INCLUDES += -I$(DFX_DIR)/zdiag/dfx_include
SRC_INCLUDES += -I$(OSAL_DIR)/include

SRC_INCLUDES += -I$(DFX_DIR)/basic/include

all-src += $(REL_ZDIAG_DIR)/zdiag_cmd/diag_cmd_connect.c
all-src += $(REL_ZDIAG_DIR)/zdiag_cmd/diag_cmd_default_tbl.c
all-src += $(REL_ZDIAG_DIR)/zdiag_cmd/diag_cmd_filter.c
all-src += $(REL_ZDIAG_DIR)/zdiag_cmd/diag_cmd_psd.c
all-src += $(REL_ZDIAG_DIR)/zdiag_cmd/diag_cmd_mem_operate.c
all-src += $(REL_ZDIAG_DIR)/zdiag_cmd/diag_cmd_mem_read_write.c
all-src += $(REL_ZDIAG_DIR)/zdiag_cmd/diag_cmd_password.c
SRC_INCLUDES += -I$(ZDIAG_DIR)/zdiag_cmd/
SRC_INCLUDES += -I$(ZDIAG_DIR)/zdiag_cmd/include

src-list += $(REL_ZDIAG_DIR)/zdiag_cmd_shell/
SRC_INCLUDES += -I$(ZDIAG_DIR)/zdiag_cmd_shell/
SRC_INCLUDES += -I$(ZDIAG_DIR)/zdiag_cmd_shell/include

src-list += $(REL_ZDIAG_DIR)/zdiag_sample/
SRC_INCLUDES += -I$(ZDIAG_DIR)/zdiag_sample

all-src += $(REL_ZDIAG_ADAPT_DIR)/zdiag_adapt_os.c
ifeq ($(CONFIG_DFX_SUPPORT_SYSFS),y)
all-src += $(REL_ZDIAG_ADAPT_DIR)/zdiag_lib_dbg.c
endif

ifeq ($(CONFIG_DIAG_SUPPORT_SOCKET),y)
all-src += $(REL_ZDIAG_ADAPT_DIR)/zdiag_linux_socket.c
endif

ifeq ($(CONFIG_DIAG_SUPPORT_UART),y)
all-src += $(REL_ZDIAG_ADAPT_DIR)/zdiag_linux_uart.c
endif

ifeq ($(CONFIG_DIAG_SUPPORT_LOCAL_LOG),y)
all-src += $(REL_ZDIAG_ADAPT_DIR)/zdiag_local_log.c
endif
all-src += $(REL_ZDIAG_ADAPT_DIR)/zdiag_log_adapt.c
all-src += $(REL_ZDIAG_ADAPT_DIR)/zdiag_tx_proc.c
all-src += $(REL_DIAG_DIR)/diag_queue.c

ifeq ($(CONFIG_DFX_SUPPORT_SHELL_PROC), y)
all-src += $(REL_ZDIAG_ADAPT_DIR)/zdiag_shell_proc.c
endif
SRC_INCLUDES += -I$(ZDIAG_ADAPT_DIR)

SRC_INCLUDES += -I$(ZDIAG_ADAPT_INCLUDE)

src-list += $(REL_DFX_DIR)/last_dump/
SRC_INCLUDES += -I$(DFX_DIR)/last_dump/include
SRC_INCLUDES += -I$(DFX_DIR)/last_dump/dfx_include

else
all-src += $(REL_ZDIAG_ADAPT_DIR)/zdiag_log_adapt.c
all-src += $(REL_ZDIAG_DIR)/zdiag_sample/diag_sample_data.c
SRC_INCLUDES += -I$(DFX_DIR)/zdiag
SRC_INCLUDES += -I$(DFX_DIR)/zdiag/include
SRC_INCLUDES += -I$(DFX_DIR)/zdiag/dfx_include
SRC_INCLUDES += -I$(OSAL_DIR)/include
SRC_INCLUDES += -I$(ZDIAG_ADAPT_DIR)
SRC_INCLUDES += -I$(ZDIAG_ADAPT_INCLUDE)
SRC_INCLUDES += -I$(ZDIAG_DIR)/zdiag_cmd_shell/
SRC_INCLUDES += -I$(ZDIAG_DIR)/zdiag_cmd_shell/include
SRC_INCLUDES += -I$(ZDIAG_DIR)/zdiag_cmd/
SRC_INCLUDES += -I$(ZDIAG_DIR)/zdiag_cmd/include
endif  ## end for  WSCFG_PLAT_DIAG_LOG_OUT ##

ifeq ($(_PRE_PLAT_SLP_UART_FORWARD),y)
SRC_INCLUDES += -I$(DRV_DIR)/slp_forward
endif

SRC_INCLUDES += -I$(DRV_DIR)/include
SRC_INCLUDES += -I$(DRV_DIR)/plat_hcc_adapt

SRC_INCLUDES += -I$(DFX_DIR)/event/include
SRC_INCLUDES += -I$(DFX_DIR)/event/dfx_include

SRC_INCLUDES += -I$(DFX_DIR)/monitor/include
SRC_INCLUDES += -I$(DFX_DIR)/monitor/dfx_include
SRC_INCLUDES += -I$(DIAG_DIR)

SRC_INCLUDES += -I$(EXCE_DIR)
SRC_INCLUDES += -I$(FIRMWARE_DOWNLOAD_DIR)

all-src += $(REL_DRV_DIR)/oal/oal_kernel_file.c
all-src += $(REL_DRV_DIR)/oal/oal_schedule.c

all-src += $(REL_OSAL_DIR)/linux/osal_timer.c
all-src += $(REL_OSAL_DIR)/linux/osal_wait.c
all-src += $(REL_OSAL_DIR)/linux/osal_task.c
all-src += $(REL_OSAL_DIR)/linux/osal_fileops.c

# #src-list += osal_adapt/
all-src += $(REL_OSAL_ADAPT_DIR)/osal_adapt_event.c
all-src += $(REL_OSAL_ADAPT_DIR)/osal_adapt_timer.c

all-src += $(REL_CFG_DIR)/customize_bsle.c

ifneq ($(PLAT_LITE_EXTREME), y)
all-src += $(REL_OSAL_DIR)/linux/osal_addr.c
all-src += $(REL_OSAL_DIR)/linux/osal_cache.c
all-src += $(REL_OSAL_DIR)/linux/osal_interrupt.c
all-src += $(REL_OSAL_DIR)/linux/osal_atomic.c
all-src += $(REL_OSAL_ADAPT_DIR)/osal_adapt_atomic.c
all-src += $(REL_OSAL_DIR)/linux/osal_mutex.c
all-src += $(REL_OSAL_DIR)/linux/osal_semaphore.c
all-src += $(REL_OSAL_DIR)/linux/osal_spinlock.c
all-src += $(REL_OSAL_DIR)/linux/osal_string.c
all-src += $(REL_OSAL_DIR)/linux/osal_workqueue.c
all-src += $(REL_OSAL_ADAPT_DIR)/osal_adapt_string.c
all-src += $(REL_OSAL_DIR)/linux/osal_debug.c
all-src += $(REL_OSAL_ADAPT_DIR)/osal_adapt_task.c
all-src += $(REL_DRV_DIR)/oal/oal_main.c
all-src += $(REL_CFG_DIR)/customize_wifi.c
all-src += $(REL_CFG_DIR)/ini.c
endif

all-src += $(REL_FIRMWARE_DOWNLOAD_DIR)/plat_firmware.c
all-src += $(REL_PM_DIR)/plat_pm.c
all-src += $(REL_PM_DIR)/plat_pm_wlan.c
all-src += $(REL_PM_DIR)/plat_cali.c
all-src += $(REL_PM_DIR)/plat_pm_board.c
all-src += $(REL_PM_DIR)/plat_pm_board_ws73.c

ifeq ($(WSCFG_BUS_UART),y)
all-src += $(REL_FIRMWARE_DOWNLOAD_DIR)/plat_firmware_uart.c
endif
all-src += $(REL_EXCE_DIR)/plat_pm_dfr.c

all-src += $(REL_DRV_DIR)/plat_misc.c

all-src += $(REL_DRV_DIR)/tsensor/tsensor.c

SRC_INCLUDES += -I$(DRV_DIR)/tsensor

ifeq ($(_PRE_PLAT_SLP_UART_FORWARD),y)
all-src += $(REL_DRV_DIR)/slp_forward/slp_hcc_forward.c
endif

all-src += $(REL_DRV_DIR)/plat_hcc_adapt/plat_hcc_srv.c

all-src += $(REL_DRV_DIR)/host_efuse/efuse_opt.c
all-src += $(REL_DRV_DIR)/mac_addr/mac_addr.c

ifneq ($(CONFIG_NO_SECUREC),y)
all-src += $(REL_LIBC_SEC_DIR)/src/memcpy_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/memset_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/snprintf_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/sprintf_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/memmove_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/strcpy_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/strncpy_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/vsnprintf_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/vsprintf_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/strcat_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/strncat_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/strtok_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/secureprintoutput_a.c
all-src += $(REL_LIBC_SEC_DIR)/src/secureinput_a.c
all-src += $(REL_LIBC_SEC_DIR)/src/securecutil.c
all-src += $(REL_LIBC_SEC_DIR)/src/sscanf_s.c
all-src += $(REL_LIBC_SEC_DIR)/src/vsscanf_s.c
endif

############### Add src end #####################

all-src += $(foreach n,$(src-list),$(patsubst $(CUR_MK_DIR)/%,%,$(wildcard $(CUR_MK_DIR)/$(n)/*.c)))

ifeq ($(_PRE_PLAT_SHA256SUM_CHECK), y)
all-src += $(REL_DRV_DIR)/crypto/plat_sha256_calc.c
SRC_INCLUDES += -I$(DRV_DIR)/crypto
endif

# 替换路径中的//为/（Android编译中可能无法识别//）
all-src := $(subst //,/,$(all-src))

# 共存coex加入编译
all-src += $(REL_DRV_DIR)/coex_msg_adapt/coex_msg_adapt.c
SRC_INCLUDES += -I$(DRV_DIR)/coex_msg_adapt

$(KO_NAME)-objs := $(patsubst %.c,%.o,$(all-src))

# 按文件名排序
$(KO_NAME)-objs := $(shell echo $($(KO_NAME)-objs)|python3 -c "import os;files=input().strip().split();files.sort(key=lambda x:os.path.basename(x));print(' '.join(files))")
obj-m += $(KO_NAME).o

SRC_INCLUDES += -I$(DRV_DIR)/mac_addr
SRC_INCLUDES += -I$(DRV_DIR)/host_efuse
SRC_INCLUDES += -I$(INC_DIR)
SRC_INCLUDES += -I$(INC_DIR)/dfx
SRC_INCLUDES += -I$(INC_DIR)/hcc
SRC_INCLUDES += -I$(INC_DIR)/oal
SRC_INCLUDES += -I$(INC_DIR)/oal/linux
SRC_INCLUDES += -I$(INC_DIR)/net
SRC_INCLUDES += -I$(INC_DIR)/net/linux
SRC_INCLUDES += -I$(INC_DIR)/oam
SRC_INCLUDES += -I$(INC_DIR)/pm
SRC_INCLUDES += -I$(DRV_DIR)/oal
SRC_INCLUDES += -I$(DIAG_DIR)
SRC_INCLUDES += -I$(OSAL_DIR)/include
SRC_INCLUDES += -I$(PM_DIR)
SRC_INCLUDES += -I$(DRV_DIR)
SRC_INCLUDES += -I$(CFG_DIR)
SRC_INCLUDES += -I$(INC_DIR)/common
SRC_INCLUDES += -I$(INC_DIR)/oneimage
SRC_INCLUDES += -I$(LIBC_SEC_DIR)/include


##############################
SRC_INCLUDES += -I$(OSAL_DIR)/include
SRC_INCLUDES += -I$(HCC_DIR)
SRC_INCLUDES += -I$(HCC_DIR)/inc
SRC_INCLUDES += -I$(HCC_DIR)/comm
SRC_INCLUDES += -I$(HCC_DIR)/host
SRC_INCLUDES += -I$(HCC_DIR)/cfg
SRC_INCLUDES += -I$(INC_DIR)/osal_adapt
SRC_INCLUDES += -I$(MAIN_DIR)
#######################

SRC_INCLUDES += -I$(WSCFG_KERNEL_DIR)

KBUILD_CFLAGS += -DSECUREC_ENABLE_SCANF_FILE=0

KBUILD_CFLAGS += -DPLATFORM_DEBUG_ENABLE

ifneq ($(TARGET_BUILD_VARIANT),user)
KBUILD_CFLAGS += -DPLATFORM_DEBUG_ENABLE
KBUILD_CFLAGS += -DPLATFORM_SSI_FULL_LOG
endif

EXTRA_CFLAGS = $(SRC_INCLUDES)
EXTRA_CFLAGS += $(COMM_COPTS)
EXTRA_CFLAGS += $(PLAT_DEBUG_CFLAGS)
EXTRA_CFLAGS += $(COPTS) $(KBUILD_CFLAGS)

EXTRA_CFLAGS += $(HCC_EXTRA_CFLAGS)

# CleanCode Build Option
EXTRA_CFLAGS += -Werror -Os

ifeq ($(BOARD_ASIC),y)
EXTRA_CFLAGS += -DCHIP_ASIC=1
else
EXTRA_CFLAGS += -DCHIP_FPGA=1
endif

ifeq ($(BUILD_PILOT),yes)
EXTRA_CFLAGS += -DBOARD_PILOT
else
EXTRA_CFLAGS += -DBOARD_MPW
endif

ifeq ($(CONFIG_DFX_SUPPORT_SYSFS),y)
EXTRA_CFLAGS += -DCONFIG_DFX_SUPPORT_SYSFS
EXTRA_CFLAGS += -DCONFIG_DFX_SUPPORT_DEBUG
endif

EXTRA_CFLAGS += -DCONFIG_PLAT_TRNG_TRIG_RPT

EXTRA_CFLAGS += -Wuninitialized -Wempty-body -Wtype-limits

EXTRA_CFLAGS +=-DBOARD_DEFAULT=0 -DBOARD_DEFAULT_K4=1 -DBOARD_SHAOLINAXE=2

EXTRA_CFLAGS += -DHOST_BOARD=BOARD_DEFAULT

EXTRA_CFLAGS += -D_PRE_WLAN_PLAT_SUPPORT_HOST_HSO
EXTRA_CFLAGS += -DCONFIG_ZDIAG_PRINT_SUPPORT_LINUX_OS

EXTRA_CFLAGS += -DON_CHIP_LINUX_SDK

# 判断是否开放码点
ifneq ($(ROMBIN_OPEN),)
$(warning "rombin has been opened!")
EXTRA_CFLAGS += -DROMBIN_OPEN
endif
EXTRA_CFLAGS += -fno-pic
EXTRA_CFLAGS += -include $(WSCFG_AUTOCONFIG_H) -include $(INC_DIR)/oneimage/oneimage.h
MODFLAGS = -fno-pic

LOCAL_CROSS_COMPILE:=$(subst ",,$(WSCFG_CROSS_COMPILE))
LOCAL_WSCFG_EXTRA_CFLAGS:=$(subst ",,$(WSCFG_EXTRA_CFLAGS))
LOCAL_WSCFG_EXTRA_PARAMS:=$(subst ",,$(WSCFG_EXTRA_PARAMS))

EXTRA_CFLAGS += ${LOCAL_WSCFG_EXTRA_CFLAGS}

BUILD_DEVICE_WITH_ROM_REPO=yes

### USING GCC ###
ifeq ($(WSCFG_USING_GCC), y)

EXTRA_CFLAGS += -Wclobbered -Wmissing-parameter-type -Wold-style-declaration -Woverride-init
EXTRA_CFLAGS += -I$(CUR_MK_DIR)/../../../linux-7d4e1f444f461dbe3833bd99a4640e7b6c2cd529/include/linux/ -Wno-type-limits -Wno-error -Wno-discarded-qualifiers -Wno-implicit-fallthrough -Wno-return-type -Wno-implicit-function-declaration
STRIP=$(LOCAL_CROSS_COMPILE)strip

default:
	$(MAKE) -C $(WSCFG_KERNEL_DIR) ARCH=${WSCFG_ARCH_NAME} CROSS_COMPILE=${LOCAL_CROSS_COMPILE} M=$(CUR_MK_DIR) modules -j KBUILD_MODPOST_WARN=1 \
		$(LOCAL_WSCFG_EXTRA_PARAMS)
	$(STRIP) --strip-unneeded $(KO_NAME).ko

### USING LLVM_CLANG ###
else ifeq ($(WSCFG_USING_LLVM_CLANG), y)

EXTRA_CFLAGS += -Wno-error=incompatible-function-pointer-types
EXTRA_CFLAGS += -mcmodel=large
EXTRA_CFLAGS += -Woverride-init

LOCAL_CLANG_PATH:=$(subst ",,$(WSCFG_CLANG_PATH))

CC=$(LOCAL_CLANG_PATH)/clang
LD=$(LOCAL_CLANG_PATH)/ld.lld
NM=$(LOCAL_CLANG_PATH)/llvm-nm
AR=$(LOCAL_CLANG_PATH)/llvm-ar
OBJCOPY=$(LOCAL_CLANG_PATH)/llvm-objcopy
STRIP=$(LOCAL_CLANG_PATH)/llvm-strip

default:
	$(MAKE) -C $(WSCFG_KERNEL_DIR) ARCH=${WSCFG_ARCH_NAME} CROSS_COMPILE=${LOCAL_CROSS_COMPILE} \
		CC=$(CC) NM=$(NM) AR=$(AR) LD=$(LD) OBJCOPY=$(OBJCOPY) M=$(CUR_MK_DIR) modules -j KBUILD_MODPOST_WARN=1 \
		$(LOCAL_WSCFG_EXTRA_PARAMS)
	$(STRIP) --strip-unneeded $(KO_NAME).ko

else
$(error "error please specify an available compiler!")
endif

clean:
	$(MAKE) -C $(WSCFG_KERNEL_DIR) ARCH=${WSCFG_ARCH_NAME} M=$(CUR_MK_DIR) clean
	-find $(MAIN_DIR)/.. -type f -name "*.o" -exec rm -f {} \;
	-find $(MAIN_DIR)/.. -type f -name "*.cmd" -exec rm -f {} \;
	-find $(CUR_MK_DIR) -type f -name "*.o" -exec rm -f {} \;
	-find $(CUR_MK_DIR) -type f -name "*.cmd" -exec rm -f {} \;
	-find $(CFG_DIR) -type f -name "*.o" -exec rm -f {} \;
	-find $(CFG_DIR) -type f -name "*.o.cmd" -exec rm -f {} \;
	-find $(OSAL_DIR)/linux/ -type f -name "*.o" -exec rm -f {} \;
	-find $(OSAL_DIR)/linux/ -type f -name "*.o.cmd" -exec rm -f {} \;
	-find $(DFX_DIR) -type f -name "*.o" -exec rm -f {} \;
	-find $(DFX_DIR) -type f -name "*.o.cmd" -exec rm -f {} \;
	-find $(ZDIAG_ADAPT_DIR) -type f -name "*.o" -exec rm -f {} \;
	-find $(ZDIAG_ADAPT_DIR) -type f -name "*.o.cmd" -exec rm -f {} \;
	-find $(HCC_DIR) -type f -name "*.o" -exec rm -f {} \;
	-find $(HCC_DIR) -type f -name "*.o.cmd" -exec rm -f {} \;
	-find $(DFX_ZDIAG_ROMABLE_DIR) -type f -name "*.o" -exec rm -f {} \;
	-find $(DFX_ZDIAG_ROMABLE_DIR) -type f -name "*.o.cmd" -exec rm -f {} \;
	-find $(LIBC_SEC_DIR)/src/ -type f -name "*.o" -exec rm -f {} \;
	-find $(LIBC_SEC_DIR)/src/ -type f -name "*.o.cmd" -exec rm -f {} \;
