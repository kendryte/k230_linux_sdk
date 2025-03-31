hcc-comm-list := $(REL_HCC_DIR)/comm/
hcc-src-list += $(hcc-comm-list)
hcc-cfg-list := $(REL_HCC_DIR)/cfg/
hcc-src-list += $(hcc-cfg-list)
hcc-host-list := $(REL_HCC_DIR)/host/
hcc-src-list += $(hcc-host-list)

HCC_SRC_INCLUDES = -I$(INC_DIR)/oal
HCC_SRC_INCLUDES += -I$(INC_DIR)/oal/linux
HCC_SRC_INCLUDES += -I$(OSAL_DIR)/include
HCC_SRC_INCLUDES += -I$(LIBC_SEC_DIR)/include

HCC_SRC_INCLUDES += -I$(HCC_DIR)
HCC_SRC_INCLUDES += -I$(INC_DIR)
HCC_SRC_INCLUDES += -I$(HCC_DIR)/comm
HCC_SRC_INCLUDES += -I$(HCC_DIR)/host
HCC_SRC_INCLUDES += -I$(PLATFORM_DEVICE_DIR)/romable/include
HCC_SRC_INCLUDES += -I$(PLATFORM_DEVICE_DIR)/romable/include/soc
HCC_SRC_INCLUDES += -I$(INC_DIR)/osal_adapt

###############################
HCC_EXTRA_CFLAGS = -DCONFIG_HCC_SUPPORT_FLOW_CONTRL \
				-DCONFIG_HCC_SUPPORT_FLOW_CONTRL_ACTIVE \
				-DCONFIG_HCC_SUPPORT_REG_OPT \
				-DCONFIG_HCC_SUPPORT_PATCH_OPT \
				-DHCC_SUPPORT_PLAT_MSG_SERVICE

ifneq ($(PLAT_LITE_EXTREME), y)
HCC_EXTRA_CFLAGS += -DCONFIG_HCC_SUPPORT_DFX
endif
###########################################################

ifeq ($(WSCFG_BUS_SDIO),y)
HCC_EXTRA_CFLAGS +=-DCONFIG_HCC_SUPPORT_SDIO
endif

ifeq ($(WSCFG_BUS_USB),y)
HCC_EXTRA_CFLAGS +=-DCONFIG_HCC_SUPPORT_USB
HCC_SRC_INCLUDES += -I$(PM_DIR)
HCC_SRC_INCLUDES += -I$(INC_DIR)/net
HCC_SRC_INCLUDES += -I$(INC_DIR)/net/linux
HCC_SRC_INCLUDES += -I$(INC_DIR)
endif

ifeq ($(WSCFG_BUS_UART),y)
HCC_EXTRA_CFLAGS +=-DCONFIG_HCC_SUPPORT_UART
endif
