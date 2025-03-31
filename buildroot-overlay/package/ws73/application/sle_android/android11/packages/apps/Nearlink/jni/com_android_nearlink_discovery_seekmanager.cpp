/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#define LOG_TAG "seekmanager-jni"
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

static const int SEEK_RS_ARRAY_0 = 0;
static const int SEEK_RS_ARRAY_4 = 4;
static const int SEEK_RS_ARRAY_5 = 5;
static const int INIT_SEEK_OK = 1;
static const int UN_INIT_SEEK = 0;

static jmethodID method_nearlink_seek_start_callback;
static jmethodID method_nearlink_seek_disable_callback;
static jmethodID method_nearlink_seek_result_callback;

static jobject sJniSeekManagerObj;
static volatile int is_seek_init = UN_INIT_SEEK;

static void classInitNative(JNIEnv *env, jclass clazz)
{

    ALOGE("com_android_nearlink_adapterservice classInitNative start");

    method_nearlink_seek_start_callback = env->GetMethodID(clazz, "onStartSeekCallback", "(I)V");

    method_nearlink_seek_disable_callback = env->GetMethodID(clazz, "onSeekDisableCallback", "(I)V");

    method_nearlink_seek_result_callback = env->GetMethodID(clazz, "onSeekResultCallback", "(II[BI[BIII[B)V");

    ALOGE("com_android_nearlink_adapterservice classInitNative end");
}

static jboolean initNative(JNIEnv *env, jobject obj)
{
    ALOGE("%s", __func__);

    sJniSeekManagerObj = env->NewGlobalRef(obj);
    is_seek_init = INIT_SEEK_OK;

    return JNI_TRUE;
}

static void cleanupNative(JNIEnv *env,jobject object)
{
    ALOGV("%s", __func__);
    if (sJniSeekManagerObj != NULL) {
        env->DeleteGlobalRef(sJniSeekManagerObj);
        sJniSeekManagerObj = NULL;
    }
    is_seek_init = UN_INIT_SEEK;
}

void nearlink_seek_start_callback(errcode_t status)
{
    ALOGE("adapter_sle_start_seek_callback status: %x\n", status);

    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }
    sCallbackEnv->CallVoidMethod(sJniSeekManagerObj, method_nearlink_seek_start_callback, status);
}
void nearlink_seek_disable_callback(errcode_t status)
{
    ALOGE("adapter_sle_seek_disable_callback status: %x\n", status);

    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }
    sCallbackEnv->CallVoidMethod(sJniSeekManagerObj, method_nearlink_seek_disable_callback, status);
}

#ifdef SEEK_LOG_CONFIG
static void showSeekResultData(int dataLength, uint8_t *data)
{
    if (dataLength <= 0) {
        ALOGE("dataLength  <= 0");
        return;
    }

    if(data == NULL){
        ALOGE("data is NULL");
        return;    
    }

    ALOGE("=====seekdata len : %u ======", dataLength);
    for (int i = 0; i < dataLength; i++) {
        uint8_t el = data[i];
        ALOGE("seekdata[%u]=%u \n", i, el);
    }
}
#endif /* SEEK_LOG_CONFIG */

void nearlink_seek_result_callback(sle_seek_result_info_t *seek_result_data)
{
    if(seek_result_data == NULL){
        ALOGE("seek cb ressult data is null");
        return;
    }
    ALOGE("seek cb addr:0x%02x:**:**:**:0x%02x:0x%02x ; rssi : %d "
    ",event_type:%d\n",
    seek_result_data->addr.addr[SEEK_RS_ARRAY_0],
    seek_result_data->addr.addr[SEEK_RS_ARRAY_4],
    seek_result_data->addr.addr[SEEK_RS_ARRAY_5],
    seek_result_data->rssi,
    seek_result_data->event_type);
    #ifdef SEEK_LOG_CONFIG
        showSeekResultData(seek_result_data->data_length, seek_result_data->data);
    #endif /* SEEK_LOG_CONFIG */
    if(is_seek_init == UN_INIT_SEEK){
        ALOGE("is_seek_init is %d, return", is_seek_init);
        return;
    }
    if(method_nearlink_seek_result_callback == NULL){
        ALOGE("jmethodID result callback is null");
        return;
    }
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) {
        return;
    }

    ScopedLocalRef<jbyteArray> jbData(sCallbackEnv.get(), sCallbackEnv->NewByteArray(seek_result_data->data_length));
    sCallbackEnv->SetByteArrayRegion(jbData.get(), 0, seek_result_data->data_length, (jbyte *)seek_result_data->data);

    ScopedLocalRef<jbyteArray> jcAddr(sCallbackEnv.get(), sCallbackEnv->NewByteArray(SLE_ADDR_LEN));
    sCallbackEnv->SetByteArrayRegion(jcAddr.get(), 0, SLE_ADDR_LEN, (const jbyte *)(seek_result_data->addr.addr));

    ScopedLocalRef<jbyteArray> jcDirectAddrr(sCallbackEnv.get(), sCallbackEnv->NewByteArray(SLE_ADDR_LEN));
    sCallbackEnv->SetByteArrayRegion(
        jcDirectAddrr.get(), 0, SLE_ADDR_LEN, (const jbyte *)(seek_result_data->direct_addr.addr));

    sCallbackEnv->CallVoidMethod(sJniSeekManagerObj,
        method_nearlink_seek_result_callback,
        seek_result_data->event_type,
        seek_result_data->addr.type,
        jcAddr.get(),
        seek_result_data->direct_addr.type,
        jcDirectAddrr.get(),
        seek_result_data->rssi,
        seek_result_data->data_status,
        seek_result_data->data_length,
        jbData.get());
}


static jint setSeekParamNative(JNIEnv *env, jobject obj, 
                                  jint ownAddrType,
                                  jint filterDuplicates,
                                  jint seekFilterPolicy,
                                  jint seekPhys,
                                  jint seekType,
                                  jint seekInterval,
                                  jint seekWindow)
{
    ALOGE("%s", __func__);
    int ret = 0;
    sle_seek_param_t param;

    ALOGE("setSeekParamNative ownAddrType=%d,filterDuplicates=%d,seekFilterPolicy=%d,seekPhys=%d,seekType=%d,seekInterval=%d,seekWindow=%d\n",
                    ownAddrType,filterDuplicates,seekFilterPolicy,seekPhys,seekType,seekInterval,seekWindow);
    param.own_addr_type = ownAddrType;
    param.filter_duplicates = filterDuplicates;
    param.seek_filter_policy = seekFilterPolicy;
    param.seek_phys = seekPhys;
    param.seek_type[0] = seekType;
    param.seek_interval[0] = seekInterval;
    param.seek_window[0] = seekWindow;

    ret = sle_set_seek_param(&param);
    ALOGE("sle_set_seek_param result: %x\n", ret);
    return ret;
}

static jint startSeekNative(JNIEnv *env, jobject obj){
     ALOGE("%s", __func__);
     int ret = 0;
     ALOGE("sle_start_seek result: %x\n", ret);
     ret = sle_start_seek();
     ALOGE("sle_start_seek result: %x\n", ret);
     return ret;
}

static jint stopSeekNative(JNIEnv *env, jobject obj) {

    ALOGE("%s", __func__);
    int ret = 0;
    ALOGE("sle_stop_seek result: %x\n", ret);
    ret = sle_stop_seek();
    ALOGE("sle_stop_seek result: %x\n", ret);
    return ret;
}

static JNINativeMethod sMethods[] = {
    /* name, signature, funcPtr */
    {"classInitNative", "()V", (void *)classInitNative},
    {"initNative", "()Z", (void *)initNative},
    {"cleanupNative", "()V", (void *)cleanupNative},
    {"setSeekParamNative", "(IIIIIII)I", (void *)setSeekParamNative},
    {"startSeekNative", "()I", (void *)startSeekNative},
    {"stopSeekNative", "()I", (void *)stopSeekNative}};

int register_com_android_nearLink_SeekManager(JNIEnv *env)
{
    return jniRegisterNativeMethods(env, "com/android/nearlink/discovery/SeekManager", sMethods, NELEM(sMethods));
}

} /* namespace android */