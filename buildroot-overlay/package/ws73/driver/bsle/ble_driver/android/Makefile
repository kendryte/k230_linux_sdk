CUR_MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_MK_DIR := $(dir $(CUR_MK_PATH))
CUR_MK_DIR := $(realpath $(CUR_MK_DIR))

include $(CUR_MK_DIR)/$(DIR_MAP_CONFIG_FILE)
include $(WSCFG_KCONFIG_CONFIG)
ifneq ($(KERNELRELEASE),)
$(warning defconfig: ================ARCH=$(WSCFG_ARCH_NAME) CROSS_COMPILE=$(WSCFG_CROSS_COMPILE)=======================)

obj-m := ble_soc.o
ble_soc-objs := ble_host.o ble_hcc/ble_hcc.o bt_dev/bt_dev.o

_INCLUDES += -I$(PLATFORM_CFG_DIR)
_INCLUDES += -I$(PLATFORM_INC_DIR)/oal
_INCLUDES += -I$(PLATFORM_INC_DIR)/oal/linux
_INCLUDES += -I$(PLATFORM_DRV_DIR)/include
_INCLUDES += -I$(PLATFORM_DEVICE_DIR)/romable/include
_INCLUDES += -I$(PLATFORM_HCC_DIR)/cfg
_INCLUDES += -I$(PLATFORM_OSAL_DIR)/include
_INCLUDES += -I$(PLATFORM_HCC_DIR)/comm
_INCLUDES += -I$(PLATFORM_HCC_DIR)/host
_INCLUDES += -I$(PLATFORM_HCC_DIR)
_INCLUDES += -I$(PLATFORM_HCC_DIR)/inc
_INCLUDES += -I$(PLATFORM_LIBC_SEC_DIR)/include
_INCLUDES += -I$(CUR_MK_DIR)
_INCLUDES += -I$(CUR_MK_DIR)/ble_hcc
_INCLUDES += -I$(CUR_MK_DIR)/bt_dev
_INCLUDES += -I$(CUR_MK_DIR)/include

EXTRA_CFLAGS = $(_INCLUDES)
# CleanCode Build Option
EXTRA_CFLAGS += -Werror -Wno-error=unused-result -Wno-typedef-redefinition -Wno-macro-redefined -Wno-uninitialized
EXTRA_CFLAGS += -include $(WSCFG_AUTOCONFIG_H) -include $(PLATFORM_INC_DIR)/oneimage/oneimage.h

else
$(warning defconfig: ================$(WS_KERNEL_DIR)=======================)

ifeq ($(WSCFG_USING_GCC), y)
modules:
	make -C $(WSCFG_KERNEL_DIR) ARCH=$(WSCFG_ARCH_NAME) CROSS_COMPILE=$(WSCFG_CROSS_COMPILE) M=$(CUR_MK_DIR) modules KBUILD_MODPOST_WARN=1

else ifeq ($(WSCFG_USING_LLVM_CLANG), y)
LOCAL_CLANG_PATH:=$(subst ",,$(WSCFG_CLANG_PATH))
LOCAL_CLANG_EXTRA_CFLAGS:=$(subst ",,$(WSCFG_CLANG_EXTRA_CFLAGS))
LOCAL_CROSS_COMPILE:=$(subst ",,$(WSCFG_CROSS_COMPILE))

CC=$(LOCAL_CLANG_PATH)/clang
LD=$(LOCAL_CLANG_PATH)/ld.lld
NM=$(LOCAL_CLANG_PATH)/llvm-nm
AR=$(LOCAL_CLANG_PATH)/llvm-ar
OBJCOPY=$(LOCAL_CLANG_PATH)/llvm-objcopy

modules:
	$(MAKE) -C $(WSCFG_KERNEL_DIR) ARCH=$(WSCFG_ARCH_NAME) CROSS_COMPILE=$(LOCAL_CROSS_COMPILE) CC=$(CC) NM=$(NM) AR=$(AR) LD=$(LD) OBJCOPY=$(OBJCOPY) M=$(CUR_MK_DIR) modules -j LLVM=1 LLVM_IAS=1 KBUILD_MODPOST_WARN=1
else
$(error "error please specify an available compiler!")
endif

clean:
	rm -f *.o *.ko *.mod.c *.o.cmd .*.o.cmd .*.ko.cmd .*.mod.o.cmd *.symvers *.order *.tmp_versions

endif