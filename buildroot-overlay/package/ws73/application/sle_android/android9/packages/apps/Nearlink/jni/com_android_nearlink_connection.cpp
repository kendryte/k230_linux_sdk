/**
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: SLE connection manager.
 */

#define LOG_TAG "[SLE_CONN_CPP]--ConnectionJNI"
#define ARRAY_0 0
#define ARRAY_4 4
#define ARRAY_5 5

#include "android_runtime/AndroidRuntime.h"
#include "sle_connection_manager.h"
#include "com_android_nearlink.h"
#include "aapi_sle_ssap_client.h"
#include "utils/Log.h"

#include <string.h>

namespace android {
/**
 * Connection callback methods
 */
static jmethodID method_sle_connect_state_changed_callback;
static jmethodID method_sle_connect_param_update_req_callback;
static jmethodID method_sle_connect_param_update_callback;
static jmethodID method_sle_auth_complete_callback;
static jmethodID method_sle_pair_complete_callback;
static jmethodID method_sle_read_rssi_callback;

static jobject sJniAdapterServiceObj;
static jobject sJniCallbacksObj;
static jfieldID sJniCallbacksField;
static void connect_state_changed_cb(uint16_t connId, const sle_addr_t *addr, sle_acb_state_t connState,
    sle_pair_state_t pairState, sle_disc_reason_t discReason)
{
    ALOGE("connect_state_changed_cb begin");
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }
    ScopedLocalRef<jbyteArray> jcAddr(sCallbackEnv.get(), sCallbackEnv->NewByteArray(SLE_ADDR_LEN));
    sCallbackEnv->SetByteArrayRegion(jcAddr.get(), 0, SLE_ADDR_LEN, (const jbyte *)(addr->addr));

    // public void onConnectStateChanged(int connId, byte[] address, int addressType,
    // int connState, int pairState, int discReason)
    sCallbackEnv->CallVoidMethod(sJniCallbacksObj,
        method_sle_connect_state_changed_callback,
        connId,
        jcAddr.get(),
        addr->type,
        connState,
        pairState,
        discReason);
    ALOGE("connect_state_changed_cb end");
}

static void connect_param_update_req_cb(
    uint16_t connId, errcode_t status, const sle_connection_param_update_req_t *param)
{
    ALOGE("connect_param_update_req_cb begin");
    ALOGE("connect_param_update_req_cb errcode_t:0x%02x", status);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }
    sCallbackEnv->CallVoidMethod(sJniCallbacksObj,
        method_sle_connect_param_update_req_callback,
        connId,
        status,
        param->interval_min,
        param->interval_max,
        param->max_latency,
        param->supervision_timeout);
    ALOGE("connect_param_update_req_cb end");
}

static void connect_param_update_cb(uint16_t connId, errcode_t status, const sle_connection_param_update_evt_t *param)
{
    ALOGE("connect_param_update_cb begin");
    ALOGE("connect_param_update_cb errcode_t:0x%02x", status);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }
    // public void onConnectParamUpdate(int connId, int errCode, int interval, int latency, int supervision)
    sCallbackEnv->CallVoidMethod(sJniCallbacksObj,
        method_sle_connect_param_update_callback,
        connId,
        status,
        param->interval,
        param->latency,
        param->supervision);
    ALOGE("connect_param_update_cb end");
}
static void auth_complete_cb(uint16_t connId, const sle_addr_t *addr, errcode_t status, const sle_auth_info_evt_t *evt)
{
    ALOGE("auth_complete_cb begin");
    ALOGE("auth_complete_cb errcode_t:0x%02x", status);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }
    ScopedLocalRef<jbyteArray> jcAddr(sCallbackEnv.get(), sCallbackEnv->NewByteArray(SLE_ADDR_LEN));
    sCallbackEnv->SetByteArrayRegion(jcAddr.get(), 0, SLE_ADDR_LEN, (const jbyte *)(addr->addr));

    ScopedLocalRef<jbyteArray> jcAuthInfo(sCallbackEnv.get(), sCallbackEnv->NewByteArray(SLE_LINK_KEY_LEN));
    sCallbackEnv->SetByteArrayRegion(jcAuthInfo.get(), 0, SLE_LINK_KEY_LEN, (const jbyte *)(evt->link_key));

    sCallbackEnv->CallVoidMethod(sJniCallbacksObj,
        method_sle_auth_complete_callback,
        connId,
        jcAddr.get(),
        addr->type,
        status,
        jcAuthInfo.get(),
        evt->crypto_algo,
        evt->key_deriv_algo,
        evt->integr_chk_ind);
    ALOGE("auth_complete_cb end");
}
static void pair_complete_cb(uint16_t connId, const sle_addr_t *addr, errcode_t status)
{
    ALOGE("pair_complete_cb begin");
    ALOGE("pair_complete_cb errcode_t:0x%02x", status);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }
    ScopedLocalRef<jbyteArray> jcAddr(sCallbackEnv.get(), sCallbackEnv->NewByteArray(SLE_ADDR_LEN));
    sCallbackEnv->SetByteArrayRegion(jcAddr.get(), 0, SLE_ADDR_LEN, (const jbyte *)(addr->addr));

    sCallbackEnv->CallVoidMethod(
        sJniCallbacksObj, method_sle_pair_complete_callback, connId, jcAddr.get(), addr->type, status);
    ALOGE("pair_complete_cb end");
}
static void read_rssi_cb(uint16_t connId, int8_t rssi, errcode_t status)
{
    ALOGE("read_rssi_cb begin");
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }

    sCallbackEnv->CallVoidMethod(sJniCallbacksObj, method_sle_read_rssi_callback, connId, rssi, status);
    ALOGE("read_rssi_cb end");
}

static sle_connection_callbacks_t g_sleCallbacks = {
    connect_state_changed_cb,
    connect_param_update_req_cb,
    connect_param_update_cb,
    auth_complete_cb,
    pair_complete_cb,
    read_rssi_cb,
    NULL /* sle_low_latency_callback */,
    NULL
};

static void classInitNative(JNIEnv *env, jclass clazz)
{
    ALOGE("classInitNative start");
    jclass jniCallbackClass = env->FindClass("com/android/nearlink/connection/JniConnectionCallbacks");
    sJniCallbacksField =
        env->GetFieldID(clazz, "mJniCallbacks", "Lcom/android/nearlink/connection/JniConnectionCallbacks;");
    method_sle_connect_state_changed_callback =
        env->GetMethodID(jniCallbackClass, "onConnectStateChanged", "(I[BIIII)V");
    method_sle_connect_param_update_req_callback =
        env->GetMethodID(jniCallbackClass, "onConnectParamUpdateReq", "(IIIIII)V");
    method_sle_connect_param_update_callback = env->GetMethodID(jniCallbackClass, "onConnectParamUpdate", "(IIIII)V");
    method_sle_auth_complete_callback = env->GetMethodID(jniCallbackClass, "onAuthComplete", "(I[BII[BIII)V");
    method_sle_pair_complete_callback = env->GetMethodID(jniCallbackClass, "onPairComplete", "(I[BII)V");
    method_sle_read_rssi_callback = env->GetMethodID(jniCallbackClass, "onReadRssi", "(III)V");
    ALOGE("classInitNative end");
}

static jboolean initNative(JNIEnv *env, jobject obj)
{
    ALOGE("%s", __func__);

    sJniAdapterServiceObj = env->NewGlobalRef(obj);
    sJniCallbacksObj = env->NewGlobalRef(env->GetObjectField(obj, sJniCallbacksField));
    int ret = sle_connection_register_callbacks(&g_sleCallbacks);

    ALOGE("initNative result: 0x%02x", ret);

    return JNI_TRUE;
}
static void cleanupNative(JNIEnv *env, jobject object)
{
    ALOGV("%s", __func__);
    if (sJniCallbacksObj != NULL) {
        env->DeleteGlobalRef(sJniCallbacksObj);
        sJniCallbacksObj = NULL;
    }
}
static jint connectRemoteDeviceNative(JNIEnv *env, jobject obj, jbyteArray addr, jbyte type)
{
    ALOGE("%s", __func__);
    sle_addr_t sAddr;
    env->GetByteArrayRegion(addr, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
    sAddr.type = type;

    ALOGE("connectRemoteDeviceNative addr:  %02x:**:**:**:%02x:%02x",
        sAddr.addr[ARRAY_0],
        sAddr.addr[ARRAY_4],
        sAddr.addr[ARRAY_5]);

    int ret = sle_connect_remote_device(&sAddr);
    ALOGE("connectRemoteDeviceNative result: 0x%02x", ret);
    return ret;
}
static jint disconnectRemoteDeviceNative(JNIEnv *env, jobject obj, jbyteArray addr, jbyte type)
{
    ALOGE("%s", __func__);
    sle_addr_t sAddr;
    env->GetByteArrayRegion(addr, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
    sAddr.type = type;
    int ret = sle_disconnect_remote_device(&sAddr);
    ALOGE("disconnectRemoteDeviceNative result: 0x%02x", ret);
    return ret;
}

static jint updateConnectParamNative(JNIEnv *env, jobject obj, jshort connId, jshort interval_min, jshort interval_max,
    jshort max_latency, jshort supervision_timeout)
{
    ALOGE("%s", __func__);
    sle_connection_param_update_t sParam;
    sParam.conn_id = connId;
    sParam.interval_min = interval_min;
    sParam.interval_max = interval_max;
    sParam.max_latency = max_latency;
    sParam.supervision_timeout = supervision_timeout;
    int ret = sle_update_connect_param(&sParam);
    ALOGE("updateConnectParamNative result: 0x%02x", ret);
    return ret;
}
static jint pairRemoteDeviceNative(JNIEnv *env, jobject obj, jbyteArray addr, jbyte type)
{
    ALOGE("%s", __func__);
    sle_addr_t sAddr;
    env->GetByteArrayRegion(addr, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
    sAddr.type = type;
    int ret = sle_pair_remote_device(&sAddr);
    ALOGE("pairRemoteDeviceNative result: 0x%02x", ret);
    return ret;
}
static jint removePairedRemoteDeviceNative(JNIEnv *env, jobject obj, jbyteArray addr, jbyte type)
{
    ALOGE("%s", __func__);
    sle_addr_t sAddr;
    env->GetByteArrayRegion(addr, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
    sAddr.type = type;

    ALOGE("removePairedRemoteDeviceNative addr:  %02x:**:**:**:%02x:%02x",
        sAddr.addr[ARRAY_0],
        sAddr.addr[ARRAY_4],
        sAddr.addr[ARRAY_5]);

    int ret = aapi_sle_remove_paired_remote_device(&sAddr);
    ALOGE("removePairedRemoteDeviceNative result: 0x%02x", ret);
    return ret;
}

static jint removeAllPairsNative(JNIEnv *env, jobject obj)
{
    ALOGE("%s", __func__);
    int ret = sle_remove_all_pairs();
    ALOGE("removeAllPairsNative result: 0x%02x", ret);
    return ret;
}

static jint getPairedDevicesNumNative(JNIEnv *env, jobject obj)
{
    ALOGE("%s", __func__);
    uint16_t number = 0;
    int ret = sle_get_paired_devices_num(&number);
    ALOGE("getPairedDevicesNumNative result: 0x%02x", ret);
    ALOGE("getPairedDevicesNumNative result number: 0x%02x", number);
    return number;
}
static jint getPairedDevicesNative(JNIEnv *env, jobject obj, jobject list)
{
    ALOGE("%s", __func__);
    sle_addr_t addr[10];
    uint16_t number = 10;
    int ret = sle_get_paired_devices(addr, &number);
    ALOGE("getPairedDevicesNative number: %x", number);
    if (ret == 0 && number > 0) {
        jclass nameObjClass = env->FindClass("com/android/nearlink/connection/NearlinkAddressList");
        jmethodID mAdd = env->GetMethodID(nameObjClass, "add", "([BB)V");
        for (int i = 0; i < number; i++) {
            ScopedLocalRef<jbyteArray> tempName(env, env->NewByteArray(SLE_ADDR_LEN));
            if (!tempName.get()) {
                ALOGE("%s: Fail to new jbyteArray bd addr", __func__);
                return -1;
            }
            env->SetByteArrayRegion(tempName.get(), 0, SLE_ADDR_LEN, reinterpret_cast<const jbyte *>(addr[i].addr));
            ALOGE("getPairedDevicesNative addr-%x: %02x:**:**:**:%02x:%02x",
                i,
                addr[i].addr[ARRAY_0],
                addr[i].addr[ARRAY_4],
                addr[i].addr[ARRAY_5]);

            env->CallVoidMethod(list, mAdd, tempName.get(), addr[i].type);
        }
    }

    ALOGE("getPairedDevicesNative result: 0x%02x", ret);
    return ret;
}
static jint getPairStateNative(JNIEnv *env, jobject obj, jbyteArray addr, jbyte type)
{
    ALOGE("%s", __func__);
    sle_addr_t sAddr;
    env->GetByteArrayRegion(addr, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
    sAddr.type = type;
    uint8_t state = SLE_PAIR_NONE;
    int ret = sle_get_pair_state(&sAddr, &state);
    ALOGE("getPairStateNative state: 0x%02x", state);
    ALOGE("getPairStateNative result: 0x%02x", ret);

    if (ret == ERRCODE_SLE_SUCCESS) {
        return state;
    } else {
        return ret;
    }
}
static jint readRemoteDeviceRssiNative(JNIEnv *env, jobject obj, jshort connId)
{
    ALOGE("%s", __func__);
    int ret = sle_read_remote_device_rssi(connId);
    ALOGE("readRemoteDeviceRssiNative connId:%x,result: 0x%x", connId, ret);
    return ret;
}

static JNINativeMethod sMethods[] = {
    /* name, signature, funcPtr */
    {"classInitNative", "()V", (void *)classInitNative},
    {"initNative", "()Z", (void *)initNative},
    {"cleanupNative", "()V", (void *)cleanupNative},
    {"connectRemoteDeviceNative", "([BB)I", (void *)connectRemoteDeviceNative},
    {"disconnectRemoteDeviceNative", "([BB)I", (void *)disconnectRemoteDeviceNative},
    {"updateConnectParamNative", "(SSSSS)I", (void *)updateConnectParamNative},
    {"pairRemoteDeviceNative", "([BB)I", (void *)pairRemoteDeviceNative},
    {"removePairedRemoteDeviceNative", "([BB)I", (void *)removePairedRemoteDeviceNative},
    {"removeAllPairsNative", "()I", (void *)removeAllPairsNative},
    {"getPairedDevicesNumNative", "()I", (void *)getPairedDevicesNumNative},
    {"getPairedDevicesNative",
        "(Lcom/android/nearlink/connection/NearlinkAddressList;)I",
        (void *)getPairedDevicesNative},
    {"getPairStateNative", "([BB)I", (void *)getPairStateNative},
    {"readRemoteDeviceRssiNative", "(S)I", (void *)readRemoteDeviceRssiNative},
};

int register_com_android_nearlink_ConnectionService(JNIEnv *env)
{
    ALOGE("%s", __func__);
    return jniRegisterNativeMethods(
        env, "com/android/nearlink/connection/ConnectionService", sMethods, NELEM(sMethods));
}

} /* namespace android */
