/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#define LOG_TAG "NearlinkHidHostServiceJni"

#define LOG_NDEBUG 1
#define ARRAY_0 0
#define ARRAY_1 1
#define ARRAY_4 4
#define ARRAY_5 5

#include "android_runtime/AndroidRuntime.h"
#include "utils/Log.h"
#include <string.h>
#include "aapi_sle_hh_intf.h"
#include "aapi_sle_ssap_client.h"
#include "com_android_nearlink.h"

namespace android {

static jmethodID method_onConnectStateChanged;
static jmethodID method_onHandshake;

static jobject mCallbacksObj = NULL;

static void connection_state_callback(uint16_t conn_id, const sle_addr_t *address, sle_acb_state_t conn_state,
    sle_pair_state_t pair_state, sle_disc_reason_t disc_reason)
{
    ALOGE("connection_state_callback address = 0x%02X:0x%02X:**:**:0x%02X:0x%02X",
        address->addr[ARRAY_0],
        address->addr[ARRAY_1],
        address->addr[ARRAY_4],
        address->addr[ARRAY_5]);
    ALOGE("connection_state_callback: conn_state = 0x%02X, pair_state = 0x%02X, disc_reason = 0x%02X",
        conn_state, pair_state, disc_reason);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid())
        return;
    if (!mCallbacksObj) {
        ALOGE("%s: mCallbacksObj is null", __func__);
        return;
    }
    ScopedLocalRef<jbyteArray> addr(sCallbackEnv.get(), sCallbackEnv->NewByteArray(SLE_ADDR_LEN));
    if (!addr.get()) {
        ALOGE("%s: Fail to new jbyteArray bd addr", __func__);
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr.get(), 0, SLE_ADDR_LEN, reinterpret_cast<const jbyte *>(address->addr));

    sCallbackEnv->CallVoidMethod(
        mCallbacksObj, method_onConnectStateChanged, addr.get(), (jint)conn_state, (jint)pair_state, (jint)disc_reason);
}

static void handshake_callback(uint16_t conn_id, const sle_addr_t *address, sle_acb_state_t conn_state)
{
    ALOGE("handshake_callback");
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid())
        return;
    if (!mCallbacksObj) {
        ALOGE("%s: mCallbacksObj is null", __func__);
        return;
    }

    ScopedLocalRef<jbyteArray> addr(sCallbackEnv.get(), sCallbackEnv->NewByteArray(SLE_ADDR_LEN));
    if (!addr.get()) {
        ALOGE("Fail to new jbyteArray bd addr for handshake callback");
        return;
    }
    sCallbackEnv->SetByteArrayRegion(addr.get(), 0, SLE_ADDR_LEN, reinterpret_cast<const jbyte *>(address->addr));

    sCallbackEnv->CallVoidMethod(mCallbacksObj, method_onHandshake, addr.get(), (jint)conn_state);
}

static sle_hh_callbacks_t sNearlinkHidCallbacks = {connection_state_callback, handshake_callback};

// Define native functions

static void classInitNative(JNIEnv *env, jclass clazz)
{
    ALOGE("classInitNative");
    method_onConnectStateChanged = env->GetMethodID(clazz, "onConnectStateChanged", "([BIII)V");
    method_onHandshake = env->GetMethodID(clazz, "onHandshake", "([BI)V");

    ALOGI("%s: succeeds", __func__);
}

static void initializeNative(JNIEnv *env, jobject object)
{
    if (mCallbacksObj != NULL) {
        ALOGW("Cleaning up Nearlink GID callback object");
        env->DeleteGlobalRef(mCallbacksObj);
        mCallbacksObj = NULL;
    }

    errcode_t result = sle_hh_init(&sNearlinkHidCallbacks);
    ALOGE("initializeNative result: 0x%x\n", result);
    if (result != ERRCODE_SLE_SUCCESS) {
        ALOGE("Failed to initialize Nearlink HID, status: %d", result);
        return;
    }

    mCallbacksObj = env->NewGlobalRef(object);
}

static void cleanupNative(JNIEnv *env, jobject object)
{
    ALOGW("Cleaning up Nearlink HID Interface...");
    sle_hh_cleanup();

    if (mCallbacksObj != NULL) {
        ALOGW("Cleaning up Nearlink GID callback object");
        env->DeleteGlobalRef(mCallbacksObj);
        mCallbacksObj = NULL;
    }
}

static jboolean connectHidNative(JNIEnv *env, jobject object, jbyteArray address)
{
    ALOGD("connectHidNative");
    sle_addr_t sAddr;
    env->GetByteArrayRegion(address, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
    sAddr.type = 0;
    ALOGD("connect address = 0x%02X:0x%02X:**:**:0x%02X:0x%02X",
          sAddr.addr[ARRAY_0], sAddr.addr[ARRAY_1], sAddr.addr[ARRAY_4], sAddr.addr[ARRAY_5]);
    aapi_conn_state_t out;
    errcode_t result = sle_hh_connect(&sAddr,&out);
    ALOGD("nearlink_hh_connect result = 0x%02X, conn_state = 0x%02X", result, out.conn_state);
    if (result == ERRCODE_SLE_SUCCESS && out.state == AAPI_OUT_STATUS_FINISHED) {
        // 表示链路已经建立，此时协议栈不会触发状态变化的回调，自己构造一个返回到上层
        ALOGD("device has connected, sync state");
        env->CallVoidMethod(mCallbacksObj,method_onConnectStateChanged,
                                     address, out.conn_state, out.pair_state, 0x0);
        return JNI_TRUE;
    }
    if (result != ERRCODE_SLE_SUCCESS) {
        if (result == ERRCODE_SLE_BUSY) {
            ALOGE("Nearlink HID connect busy");
            return JNI_TRUE;
        }
        ALOGE("Failed HID channel connection, result: 0x%02X", result);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static jboolean disconnectHidNative(JNIEnv *env, jobject object, jbyteArray address)
{
    ALOGD("disconnectHidNative");
    sle_addr_t sAddr;
    env->GetByteArrayRegion(address, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
    sAddr.type = 0;
    ALOGD("disconnect address = 0x%02X:0x%02X:**:**:0x%02X:0x%02X",
          sAddr.addr[ARRAY_0], sAddr.addr[ARRAY_1], sAddr.addr[ARRAY_4], sAddr.addr[ARRAY_5]);
    uint8_t out;
    errcode_t result = sle_hh_disconnect(&sAddr, &out);
    if (result == ERRCODE_SLE_SUCCESS && out == AAPI_OUT_STATUS_FINISHED) {
        // 表示链路已经断开，此时协议栈不会触发状态变化的回调，自己构造一个返回到上层
        ALOGD("device has disconnected, sync state");
        env->CallVoidMethod(mCallbacksObj, method_onConnectStateChanged,
                                     address, 0x2, 0x3, 0x0);
        return JNI_TRUE;
    }
    if (result != ERRCODE_SLE_SUCCESS) {
        ALOGE("Failed disconnect hid channel, status: %d", result);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static jboolean setReportNative(JNIEnv *env, jobject object, jbyteArray address, jbyte reportType, jstring report)
{
    ALOGE("%s: reportType = %d", __func__, reportType);

    sle_addr_t sAddr;
    env->GetByteArrayRegion(address, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
    sAddr.type = 0;
    jint rType = reportType;
    const char *c_report = env->GetStringUTFChars(report, NULL);

    jboolean ret = JNI_TRUE;
    errcode_t result = sle_hh_set_report(&sAddr, (sle_hh_report_type_t)rType, strlen(c_report), (uint8_t *)c_report);
    if (result != ERRCODE_SLE_SUCCESS) {
        ALOGE("Failed set report, status: %d", result);
        ret = JNI_FALSE;
    }
    env->ReleaseStringUTFChars(report, c_report);

    return ret;
}

static JNINativeMethod sMethods[] = {
    {"classInitNative", "()V", (void *)classInitNative},
    {"initializeNative", "()V", (void *)initializeNative},
    {"cleanupNative", "()V", (void *)cleanupNative},
    {"connectHidNative", "([B)Z", (void *)connectHidNative},
    {"disconnectHidNative", "([B)Z", (void *)disconnectHidNative},
    {"setReportNative", "([BBLjava/lang/String;)Z", (void *)setReportNative},
};

int register_com_android_nearlink_hid_host(JNIEnv *env)
{
    return jniRegisterNativeMethods(env, "com/android/nearlink/hid/HidHostService", sMethods, NELEM(sMethods));
}
}  // namespace android