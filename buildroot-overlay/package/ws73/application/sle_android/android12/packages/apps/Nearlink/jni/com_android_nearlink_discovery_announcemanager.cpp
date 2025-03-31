/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#define LOG_TAG "announcemanager-jni"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/Log.h"
#include "utils/Log.h"
#include "utils/misc.h"
#include "sle_device_discovery.h"
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

#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <mutex>

namespace android {

/* sle device name */
#define NAME_MAX_LENGTH 15
/* 连接调度间隔12.5ms，单位125us */
#define SLE_CONN_INTV_MIN_DEFAULT 0x64
/* 连接调度间隔12.5ms，单位125us */
#define SLE_CONN_INTV_MAX_DEFAULT 0x64
/* 连接调度间隔25ms，单位125us */
#define SLE_ADV_INTERVAL_MIN_DEFAULT 0xC8
/* 连接调度间隔25ms，单位125us */
#define SLE_ADV_INTERVAL_MAX_DEFAULT 0xC8
/* 超时时间5000ms，单位10ms */
#define SLE_CONN_SUPERVISION_TIMEOUT_DEFAULT 0x1F4
/* 超时时间4990ms，单位10ms */
#define SLE_CONN_MAX_LATENCY 0x1F3
/* 广播发送功率 */
#define SLE_ADV_TX_POWER 10
/* 广播ID */
#define SLE_ADV_HANDLE_DEFAULT 1
/* 最大广播数据长度 */
#define SLE_ADV_DATA_LEN_MAX 251

struct sle_adv_common_value {
    uint8_t length;
    uint8_t type;
    uint8_t value;
};

    static jmethodID method_nearlink_announce_enable_callback;
    static jmethodID method_nearlink_announce_disable_callback;
    static jmethodID method_nearlink_announce_terminal_callback;

    static jobject sJniAnnounceManagerObj;

    static void classInitNative(JNIEnv *env, jclass clazz)
    {
        ALOGE("com_android_nearlink_announcemanager classInitNative start");;

        method_nearlink_announce_enable_callback = env->GetMethodID(clazz, "onAnnounceEnabledCallback", "(II)V");
        method_nearlink_announce_disable_callback = env->GetMethodID(clazz, "onAnnounceDisabledCallback", "(II)V");
        method_nearlink_announce_terminal_callback = env->GetMethodID(clazz, "onAnnounceTerminaledCallback", "(I)V");
    }

    static jboolean initNative(JNIEnv *env, jobject obj)
    {
        ALOGE("%s", __func__);

        sJniAnnounceManagerObj = env->NewGlobalRef(obj);

        return JNI_TRUE;
    }

    static void cleanupNative(JNIEnv *env,jobject object)
    {
        ALOGV("%s", __func__);
        if (sJniAnnounceManagerObj != NULL) {
            env->DeleteGlobalRef(sJniAnnounceManagerObj);
            sJniAnnounceManagerObj = NULL;
        }
    }

    static jint setAnnounceParamNative(JNIEnv *env, jobject obj, jint  announceId, jint  announceHandle, jint  announceMode, jint  announceGtRole,
                                      jint  announceLevel, jlong announceIntervalMin, jlong announceIntervalMax, jint announceChannelMap,
                                      jbyte announceTxPower, jbyteArray jbyteArrayOwnAddress, jint ownAddrType, jstring peerAddress, jint peerAddrType,
                                      jint connIntervalMin, jint connIntervalMax, jint connMaxLatency, jint connSupervisionTimeout)
    {

        ALOGE("setAnnounceParamNative: announceId=%d\n,announceHandle=%d,announceMode=%d,announceGtRole=%d,announceLevel=%d,announceIntervalMin=%lld,"
             "announceIntervalMax=%lld,announceChannelMap=%d,announceTxPower=%d,connIntervalMin=%d,connIntervalMax=%d,connMaxLatency=%d,connSupervisionTimeout=%d", 
             announceId, announceHandle, announceMode, announceGtRole ,announceLevel, announceIntervalMin, announceIntervalMax, announceChannelMap, 
             announceTxPower, connIntervalMin, connIntervalMax, connMaxLatency, connSupervisionTimeout);

        ALOGE("%s", __func__);

        errcode_t res = -1;

        sle_announce_param_t param;
        memset(&param, 0, sizeof(sle_announce_param_t));

        param.announce_mode = announceMode;
        param.announce_handle = announceId;
        param.announce_gt_role = announceGtRole;
        param.announce_level = announceLevel;
        param.announce_channel_map = announceChannelMap;
        param.announce_interval_min = announceIntervalMin;
        param.announce_interval_max = announceIntervalMax;
        param.conn_interval_min = connIntervalMin;
        param.conn_interval_max = connIntervalMax;
        param.conn_max_latency = connMaxLatency;
        param.conn_supervision_timeout = connSupervisionTimeout;
        param.announce_tx_power = announceTxPower;

        res = sle_get_local_addr(&(param.own_addr));
        ALOGE("sle_get_local_addr: res=%d\n", res);
        int ret = sle_set_announce_param(announceId, &param);
        ALOGE("sle_set_announce_param result: %d\n", ret);
        return ret;
    }

    static void showData(int dataLength, uint8_t *data)
    {
        if (dataLength <= 0) {
            ALOGE("dataLength  <= 0");
            return;
        }
        if(data == NULL){
            ALOGE("data is NULL");
            return;
        }
        ALOGE("data.len : %u", dataLength);
        for (int i = 0; i < dataLength; i++) {
            uint8_t el = data[i];
            ALOGE("%02x", el);
        }
    }

    static jint setAnnounceDataNative(JNIEnv *env, jobject obj, jint announceId, jint announceDataLen, jint seekRspDataLen, jbyteArray announceData, jbyteArray seekRspData){
        ALOGE("%s", __func__);
       
        int ret;
        uint8_t announce_data_len = announceDataLen;
        uint8_t seek_data_len = seekRspDataLen;

        ALOGE("setAnnounceDataNative: announceId=%d, announceDataLen=%d, seekRspDataLen=%d\n", announceId, announce_data_len, seek_data_len);

        jbyte *announce_data_ptr = env->GetByteArrayElements(announceData, NULL);

        jbyte *seek_rsp_data_ptr = env->GetByteArrayElements(seekRspData, NULL);

        ALOGE("announceData\n");
        showData(announceDataLen, (uint8_t *) announce_data_ptr);
        ALOGE("seekRspData\n");
        showData(seekRspDataLen, (uint8_t *) seek_rsp_data_ptr);

        sle_announce_data_t data;

        data.announce_data = (uint8_t*) announce_data_ptr;
        data.announce_data_len = announce_data_len;


        data.seek_rsp_data = (uint8_t*) seek_rsp_data_ptr;
        data.seek_rsp_data_len = seek_data_len;

        ret = sle_set_announce_data(announceId , &data);
        
        ALOGE("sle_set_announce_data result: %d\n", ret);
        return ret;
    }

    static jint startAnnounceNative(JNIEnv *env, jobject obj, jint  announceId)
    {
        ALOGE("%s", __func__);
        ALOGE("startAnnounceNative: announceId=%d\n", announceId);
        int ret;

        ret = sle_start_announce(announceId);
        ALOGE("sle_start_announce result: %d\n", ret);
        return ret;
    }

    static jint stopAnnounceNative(JNIEnv *env, jobject obj, jint  announceId)
    {
        ALOGE("%s", __func__);
        ALOGE("stopAnnounceNative: announceId=%d\n", announceId);
        int ret;

        ret = sle_stop_announce(announceId);
        ALOGE("sle_stop_announce result: %d\n", ret);
        return ret;
    }


    void nearlink_announce_enable_callback(uint32_t announce_id, errcode_t status)
    {
        ALOGE("nearlink_announce_enable_callback entry: announce_id=%d,status=%d\n", announce_id, status);
        CallbackEnv sCallbackEnv(__func__);
        if (!sCallbackEnv.valid()) {
            return;
        }
        ALOGE("nearlink_announce_enable_callback: announce_id=%d,status=%d\n", announce_id, status);
        sCallbackEnv->CallVoidMethod(sJniAnnounceManagerObj, method_nearlink_announce_enable_callback, announce_id, status);
    }

    void nearlink_announce_disable_callback(uint32_t announce_id, errcode_t status)
    {
        ALOGE("nearlink_announce_disable_callback entry: announce_id=%d,status=%d\n", announce_id, status);
        CallbackEnv sCallbackEnv(__func__);
        if (!sCallbackEnv.valid()) {
            return;
        }
        ALOGE("nearlink_announce_disable_callback: announce_id=%d,status=%d\n", announce_id, status);
        sCallbackEnv->CallVoidMethod(sJniAnnounceManagerObj, method_nearlink_announce_disable_callback, announce_id, status);
    }

    void nearlink_announce_terminal_callback(uint32_t announce_id){
        ALOGE("nearlink_announce_terminal_callback entry: announce_id=%d\n", announce_id);
        CallbackEnv sCallbackEnv(__func__);
        if (!sCallbackEnv.valid()) {
            return;
        }
         ALOGE("nearlink_announce_terminal_callback: announce_id=%d\n", announce_id);
        sCallbackEnv->CallVoidMethod(sJniAnnounceManagerObj, method_nearlink_announce_terminal_callback, announce_id);
    }

    static JNINativeMethod sMethods[] = {
            /* name, signature, funcPtr */
            {"classInitNative", "()V", (void *)classInitNative},
            {"initNative", "()Z", (void *)initNative},
            {"cleanupNative", "()V", (void *)cleanupNative},
            {"setAnnounceParamNative", "(IIIIIJJIB[BILjava/lang/String;IIIII)I", (void *)setAnnounceParamNative},
            {"setAnnounceDataNative", "(III[B[B)I", (void *)setAnnounceDataNative},
            {"startAnnounceNative", "(I)I", (void *)startAnnounceNative},
            {"stopAnnounceNative", "(I)I", (void *)stopAnnounceNative}};

    int register_com_android_nearLink_AnnounceManager(JNIEnv *env)
    {
            return jniRegisterNativeMethods(env, "com/android/nearlink/discovery/AnnounceManager", sMethods, NELEM(sMethods));
    }


} /* namespace android */