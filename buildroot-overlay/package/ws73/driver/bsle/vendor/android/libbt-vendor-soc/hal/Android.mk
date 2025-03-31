# Copyright 2006 The Android Open Source Project
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

BT_DIR := $(TOP_DIR)system/bt

LOCAL_SRC_FILES := \
        bt_vendor_soc.c

LOCAL_C_INCLUDES += \
        $(LOCAL_PATH) \
        $(BT_DIR)/hci/include \
        $(LOCAL_PATH)/../platform/libc_sec/include

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libutils \
        liblog \
        libc_sec_bt

LOCAL_CFLAGS += -Os -pipe
LOCAL_CFLAGS += -fno-short-enums
LOCAL_CFLAGS += -Wall -Wextra -Wundef -Wstrict-prototypes -Wfloat-equal -Wunused
ifeq ($(TARGET_BUILD_VARIANT),user)
LOCAL_CFLAGS += \
        -fvisibility=hidden

endif
LOCAL_LDFLGAS := -s -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now

LOCAL_MODULE := libbt-vendor

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := soc
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
