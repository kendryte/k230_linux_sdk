LOCAL_PATH:= $(call my-dir)

# Nearlink APK

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := Nearlink
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform
LOCAL_USE_AAPT2 := true
LOCAL_JNI_SHARED_LIBRARIES := libnearlink_jni
LOCAL_JAVA_LIBRARIES := javax.obex telephony-common services.net
# Android12新增<user-libraries>校验 https://source.android.google.cn/devices/tech/dalvik/art-class-loader-context?hl=zh-cn
LOCAL_USES_LIBRARIES  := javax.obex
# Android12新增校验
LOCAL_STATIC_JAVA_LIBRARIES := \
        com.android.vcard \
        sap-api-java-static \
        services.net \
        libprotobuf-java-lite

LOCAL_STATIC_ANDROID_LIBRARIES := android-support-v4
LOCAL_PROGUARD_ENABLED := disabled
include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
