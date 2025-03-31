/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
#define LOG_TAG "adapterService-jni"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/Log.h"
#include "utils/Log.h"
#include "utils/misc.h"
#include "sle_device_discovery.h"
#include "aapi_sle_dd.h"
#include "aapi_sle_ssap_client.h"
#include "com_android_nearlink.h"
#include "com_android_adapter.h"
#include "sle_errcode.h"
#include "sle_common.h"
#include <nativehelper/JNIHelp.h>

#include <android_util_Binder.h>
#include <base/logging.h>
#include <base/strings/stringprintf.h>
#include <cutils/properties.h>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <mutex>

namespace android {

// 存放地址的数组大小
const int ADDRESS_ARRAY_SIZE = 6;

static jmethodID method_nearlink_enable_callback;
static jmethodID method_nearlink_disable_callback;

static jobject sJniAdapterServiceObj;

static JNIEnv *callbackEnv = NULL;

JNIEnv *getCallbackEnv()
{
    return callbackEnv;
}

void nearlink_adapter_enable_callback(errcode_t status)
{
    ALOGE("adapter_sle_enable_callback status: %x\n", status);
    if (callbackEnv == NULL) {
        JavaVM *vm = AndroidRuntime::getJavaVM();
        JavaVMAttachArgs args;
        char name[] = "SLE Service Callback Thread";
        args.version = JNI_VERSION_1_6;
        args.name = name;
        args.group = NULL;
        vm->AttachCurrentThread(&callbackEnv, &args);
        ALOGE("Callback thread attached: %p", callbackEnv);
    }
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }
    sCallbackEnv->CallVoidMethod(sJniAdapterServiceObj, method_nearlink_enable_callback, status);
}

void do_disable_cbk(errcode_t status)
{
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }

    sCallbackEnv->CallVoidMethod(sJniAdapterServiceObj, method_nearlink_disable_callback, status);
}

void nearlink_adapter_disable_callback(errcode_t status)
{
    ALOGE("adapter_sle_disable_callback status: %x\n", status);

    do_disable_cbk(status);

    JavaVM *vm = AndroidRuntime::getJavaVM();
    vm->DetachCurrentThread();
    ALOGE("Callback thread detached: %p", callbackEnv);
    callbackEnv = NULL;
}

void nearlink_dfr_callback()
{
    ALOGE("nearlink_dfr_callback, will kill myself\n");
    pid_t pid = getpid();
    int32_t ret = kill(pid, SIGKILL);
    if (ret != 0) {
        ALOGE("kill self failed, pid: %d, errno:%d\n", pid, errno);
        return;
    }
    return;
}
static sle_announce_seek_callbacks_t msle_callbacks = {
    nearlink_adapter_enable_callback,
    nearlink_adapter_disable_callback,
    nearlink_announce_enable_callback,
    nearlink_announce_disable_callback,
    nearlink_announce_terminal_callback,
    NULL,
    nearlink_seek_start_callback,
    nearlink_seek_disable_callback,
    nearlink_seek_result_callback,
    nearlink_dfr_callback
};

static void classInitNative(JNIEnv *env, jclass clazz)
{
    ALOGE("com_android_nearlink_adapterservice classInitNative start");

    method_nearlink_enable_callback = env->GetMethodID(clazz, "onEnabledCallback", "(I)V");

    method_nearlink_disable_callback = env->GetMethodID(clazz, "onDisabledCallback", "(I)V");

    ALOGE("com_android_nearlink_adapterservice classInitNative end");
}

static jboolean initNative(JNIEnv *env, jobject obj)
{
    ALOGE("%s", __func__);

    sJniAdapterServiceObj = env->NewGlobalRef(obj);

    int ret = sle_announce_seek_register_callbacks(&msle_callbacks);

    ALOGE("sle_announce_seek_register_callbacks result: %d\n", ret);

    return JNI_TRUE;
}

static void cleanupNative(JNIEnv *env,jobject object)
{
    ALOGV("%s", __func__);
    if (sJniAdapterServiceObj != NULL) {
        env->DeleteGlobalRef(sJniAdapterServiceObj);
        sJniAdapterServiceObj = NULL;
    }
}

static void enableNative(JNIEnv *env, jobject obj)
{
    ALOGE("%s", __func__);
    ALOGE("==enableNative==");
    int ret = aapi_enable_sle();
    ALOGE("enable_sle result: %x\n", ret);
}

static void disableNative(JNIEnv *env, jobject obj)
{
    ALOGE("%s", __func__);

    int ret = disable_sle();

    ALOGE("disable_sle result: %d\n", ret);
}

static jint getLocalNameNative(JNIEnv *env, jobject obj, jobject nameObj)
{
    ALOGE("%s", __func__);

    uint8_t name[SLE_NAME_MAX_LEN] = {0};
    uint8_t len = SLE_NAME_MAX_LEN;
    int ret = sle_get_local_name(name, &len);

    ALOGE("sle_get_local_name result: 0x%x , len %d \n", ret, len);

    if (ret != 0) {
        return ret;
    }
    jclass nameObjClass = env->FindClass("com/android/nearlink/nlservice/ProtoDeviceName");
    if (env->IsInstanceOf(nameObj, nameObjClass)) {
        ALOGE("set filed \n");
        jfieldID lenField = env->GetFieldID(nameObjClass, "len", "I");
        env->SetIntField(nameObj, lenField, len);
        ALOGE("set filed  len end\n");
        jfieldID nameFileld = env->GetFieldID(nameObjClass, "name", "[B");

        ScopedLocalRef<jbyteArray> tempName(env, env->NewByteArray(len));
        if (!tempName.get()) {
            ALOGE("%s: Fail to new jbyteArray bd addr", __func__);
            return -1;
        }

        env->SetByteArrayRegion(tempName.get(), 0, len, reinterpret_cast<const jbyte *>(name));

        env->SetObjectField(nameObj, nameFileld, tempName.get());

        ALOGE("set filed  name end\n");
    }

    ALOGE("sle_get_local_name result: 0x%x\n", ret);
    if (ret == 0) {
        ALOGE("sle_get_local_name name: %s\n", name);
    }
    return ret;
}

static jint setLocalNameNative(JNIEnv *env, jobject obj, jbyte nameLen, jbyteArray name)
{
    ALOGE("%s", __func__);

    jbyte *nameTemp = env->GetByteArrayElements(name, NULL);

    int ret = sle_set_local_name((const uint8_t *)nameTemp, nameLen);

    ALOGE("sle_set_local_name result: 0x%x\n", ret);

    env->ReleaseByteArrayElements(name, nameTemp, 0);

    return ret;
}


static jint getLocalAddrNative(JNIEnv *env, jobject obj, jobject addrObj)
{
    ALOGE("%s", __func__);
    sle_addr_t sAddr = {.type = 0, .addr = {0}};
    int ret = -1;

    jclass gAddrObjClass = env->FindClass("com/android/nearlink/nlservice/ProtoDeviceAddr");

    ret = sle_get_local_addr(&sAddr);

    ALOGE("sle_get_local_addr result: 0x%x\n", ret);
    if (env->IsInstanceOf(addrObj, gAddrObjClass)) {
        ALOGE("set filed \n");
        jfieldID typeId = env->GetFieldID(gAddrObjClass, "type", "I");
        env->SetIntField(addrObj, typeId, sAddr.type);
        ALOGE("set filed  typeId end\n");
        jfieldID addr = env->GetFieldID(gAddrObjClass, "addr", "[B");

        ScopedLocalRef<jbyteArray> addr1(env, env->NewByteArray(ADDRESS_ARRAY_SIZE));
        if (!addr1.get()) {
            ALOGE("%s: Fail to new jbyteArray bd addr", __func__);
            return -1;
        }

        env->SetByteArrayRegion(addr1.get(), 0, 6, reinterpret_cast<const jbyte *>(sAddr.addr));

        env->SetObjectField(addrObj, addr, addr1.get());

        ALOGE("getLocalAddrNative end\n");
    }

    ALOGE("getLocalAddrNative result: 0x%x\n", ret);

    return ret;
}

static jint setLocalAddrNative(JNIEnv *env, jobject obj, jbyte type, jbyteArray addr)
{
    ALOGE("%s", __func__);

    sle_addr_t sAddr;
    sAddr.type = type;
    env->GetByteArrayRegion(addr, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);

    int ret = aapi_sle_set_local_addr(&sAddr);

    ALOGE("sle_set_local_addr result: 0x%x\n", ret);

    return ret;
}


static JNINativeMethod sMethods[] = {
    /* name, signature, funcPtr */
    {"classInitNative", "()V", (void *)classInitNative},
    {"initNative", "()Z", (void *)initNative},
    {"cleanupNative", "()V", (void *)cleanupNative},
    {"enableNative", "()V", (void *)enableNative},
    {"disableNative", "()V", (void *)disableNative},
    {"getLocalNameNative", "(Lcom/android/nearlink/nlservice/ProtoDeviceName;)I", (void *)getLocalNameNative},
    {"setLocalNameNative", "(B[B)I", (void *)setLocalNameNative},
    {"getLocalAddrNative", "(Lcom/android/nearlink/nlservice/ProtoDeviceAddr;)I", (void *)getLocalAddrNative},
    {"setLocalAddrNative", "(B[B)I", (void *)setLocalAddrNative}};

int register_com_android_nearLink_AdapterService(JNIEnv *env)
{
    return jniRegisterNativeMethods(env, "com/android/nearlink/nlservice/AdapterService", sMethods, NELEM(sMethods));
}

} /* namespace android */

/*
 * JNI Initialization
 */
jint JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    JNIEnv *e;
    int status;

    ALOGV("Helloworld Adapter Service : loading JNI\n");

    // Check JNI version
    if (jvm->GetEnv((void **)&e, JNI_VERSION_1_6)) {
        ALOGE("JNI version mismatch error");
        return JNI_ERR;
    }

    status = android::register_com_android_nearLink_AdapterService(e);
    if (status < 0) {
        ALOGE("register_com_android_nearLink_AdapterService, status: %d", status);
        return JNI_ERR;
    }

    status = android::register_com_android_nearLink_AnnounceManager(e);
    if (status < 0) {
        ALOGE("register_com_android_nearLink_AnnounceManager, status: %d", status);
        return JNI_ERR;
    }

    status = android::register_com_android_nearLink_SeekManager(e);
    if (status < 0) {
        ALOGE("register_com_android_nearLink_SeekManager, status: %d", status);
        return JNI_ERR;
    }

    status = android::register_com_android_nearlink_ssap_client(e);
    if (status < 0) {
        ALOGE("register_com_android_nearlink_ssap_client, status: %d", status);
        return JNI_ERR;
    }

    status = android::register_com_android_nearlink_ConnectionService(e);
    if (status < 0) {
        ALOGE("register_com_android_nearlink_ConnectionService, status: %d", status);
        return JNI_ERR;
    }

    status = android::register_com_android_nearlink_hid_host(e);
    if (status < 0) {
        ALOGE("register_com_android_nearlink_hid_host, status: %d", status);
        return JNI_ERR;
    }

    return JNI_VERSION_1_4;
}
