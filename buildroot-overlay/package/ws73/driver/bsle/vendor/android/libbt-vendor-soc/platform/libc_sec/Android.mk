LOCAL_PATH:= $(call my-dir)

libc_sec_src_files:= \
        src/vsprintf_s.c \
        src/wmemmove_s.c\
        src/strncat_s.c\
        src/vsnprintf_s.c\
        src/fwscanf_s.c\
        src/scanf_s.c\
        src/strcat_s.c\
        src/sscanf_s.c\
        src/secureprintoutput_w.c\
        src/wmemcpy_s.c\
        src/wcsncat_s.c\
        src/secureprintoutput_a.c\
        src/secureinput_w.c\
        src/memcpy_s.c\
        src/fscanf_s.c\
        src/vswscanf_s.c\
        src/secureinput_a.c\
        src/sprintf_s.c\
        src/memmove_s.c\
        src/swscanf_s.c\
        src/snprintf_s.c\
        src/vscanf_s.c\
        src/vswprintf_s.c\
        src/wcscpy_s.c\
        src/vfwscanf_s.c\
        src/memset_s.c\
        src/wscanf_s.c\
        src/vwscanf_s.c\
        src/strtok_s.c\
        src/wcsncpy_s.c\
        src/vfscanf_s.c\
        src/vsscanf_s.c\
        src/wcstok_s.c\
        src/securecutil.c\
        src/gets_s.c\
        src/swprintf_s.c\
        src/strcpy_s.c\
        src/wcscat_s.c\
        src/strncpy_s.c


include $(CLEAR_VARS)

#LOCAL_ODM_MODULE := true
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/src

LOCAL_SRC_FILES:=$(libc_sec_src_files)

LOCAL_CFLAGS += -Os -pipe
LOCAL_CFLAGS += -fno-short-enums
LOCAL_CFLAGS += -Wall -Wextra -Wundef -Wstrict-prototypes -Wfloat-equal -Wunused
LOCAL_CFLAGS += -DNDEBUG -DSECUREC_SUPPORT_STRTOLD=1
LOCAL_LDFLGAS := -s -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now
LOCAL_MULTILIB := both

LOCAL_MODULE:= libc_sec_bt

LOCAL_MODULE_PATH_32 := $(TARGET_OUT_VENDOR)/lib
LOCAL_MODULE_PATH_64 := $(TARGET_OUT_VENDOR)/lib64

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/src


LOCAL_SRC_FILES:=$(libc_sec_src_files)

LOCAL_CFLAGS += -Os -pipe
LOCAL_CFLAGS += -fno-short-enums
LOCAL_CFLAGS += -Wall -Wextra -Wundef -Wstrict-prototypes -Wfloat-equal -Wunused
LOCAL_CFLAGS += -DNDEBUG -DSECUREC_SUPPORT_STRTOLD=1

LOCAL_MULTILIB := both

LOCAL_MODULE:= libc_sec_bt

LOCAL_MODULE_PATH_32 := $(TARGET_OUT_VENDOR)/lib
LOCAL_MODULE_PATH_64 := $(TARGET_OUT_VENDOR)/lib64

include $(BUILD_STATIC_LIBRARY)
