# Copyright 2006 The Android Open Source Project
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

SRC_FILE_LIST := $(wildcard $(LOCAL_PATH)/../../../../driver/platform/libc_sec/src/*.c)
LOCAL_SRC_FILES += $(SRC_FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../driver/platform/libc_sec/include

LOCAL_CFLAGS += -Os -pipe
LOCAL_CFLAGS += -fno-short-enums
LOCAL_CFLAGS += -Wall -Wextra -Wundef -Wstrict-prototypes -Wfloat-equal -Wunused
LOCAL_CFLAGS += -DNDEBUG -DSECUREC_SUPPORT_STRTOLD=1 -DWSCFG_ANDROID
LOCAL_LDFLGAS := -s -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now
LOCAL_MULTILIB := both

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib

LOCAL_MODULE := libc_sec_bp
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
$(info nearlink-PRODUCT_OUT=$(PRODUCT_OUT))
TARGET_NEARLINK_PATH := $(PRODUCT_OUT)/vendor

LOCAL_SRC_FILES := \
        ../main.c \
        ../dft_channel/bsle_dft_chan_user.c \
        ../dft_shell/bsle_dft_at_server.c \
        ../dft_shell/msg_queue.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../dft_channel
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../dft_shell
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../driver/platform/libc_sec/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../driver/platform/osal/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../driver/platform/drv/device/romable/include

LOCAL_CFLAGS += -Werror
LOCAL_CFLAGS += -Wall
LOCAL_CFLAGS += -DHAVE_CONFIG_H
LOCAL_CFLAGS += -D_FILE_OFFSET_BITS=64
LOCAL_CFLAGS += -DWSCFG_ANDROID

LOCAL_STATIC_LIBRARIES := libc_sec_bp

LOCAL_MODULE := bp_channel

LOCAL_MODULE_PATH := $(TARGET_NEARLINK_PATH)/bin
include $(BUILD_EXECUTABLE)

