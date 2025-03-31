/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#include "com_android_nearlink_ssap.h"
#include "sle_ssap_server.h"

#undef LOG_TAG
#define LOG_TAG "SSAP-Server-JNI"

namespace android {
/**
 * Static variables
 */
static jobject mCallbacksObj = NULL;

/**
 * Client callback methods
 */
static jmethodID method_onServerRegistered;
static jmethodID method_onServiceAdded;
static jmethodID method_onPropertyAdded;
static jmethodID method_onServiceStarted;
static jmethodID method_onAllServiceDeleted;
static jmethodID method_onServerReadRequest;
static jmethodID method_onServerWriteRequest;
static jmethodID method_onMtuChanged;

void start_service_cb(uint8_t server_id, uint16_t handle, errcode_t status)
{
    ALOGE("start_service_cb - server_id=%d handle=%d status=%d", server_id, handle, status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    call_back_env->CallVoidMethod(mCallbacksObj, method_onServiceStarted, server_id, handle, status);
    ALOGE("start_service_cb finished");
}

void delete_all_service_cb(uint8_t server_id, errcode_t status)
{
    ALOGE("delete_all_service_cb - server_id=%d status=%d", server_id, status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    call_back_env->CallVoidMethod(mCallbacksObj, method_onAllServiceDeleted, server_id, status);
    ALOGE("delete_all_service_cb finished");
}

void read_request_cb(uint8_t server_id, uint16_t conn_id, ssaps_req_read_cb_t *read_cb_para, errcode_t status)
{
    ALOGE("read_request_cb - server_id=%d conn_id=%d request_id=%d handle=%d type=%d need_rsp=%d need_authorize=%d "
          "status=%d",
        server_id,
        conn_id,
        read_cb_para->request_id,
        read_cb_para->handle,
        read_cb_para->type,
        read_cb_para->need_rsp,
        read_cb_para->need_authorize,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    call_back_env->CallVoidMethod(mCallbacksObj,
        method_onServerReadRequest,
        server_id,
        conn_id,
        read_cb_para->request_id,
        read_cb_para->handle,
        read_cb_para->type,
        read_cb_para->need_rsp,
        read_cb_para->need_authorize,
        status);
    ALOGE("read_request_cb finished");
}

void write_request_cb(uint8_t server_id, uint16_t conn_id, ssaps_req_write_cb_t *write_cb_para, errcode_t status)
{
    ALOGE("write_request_cb - server_id=%d conn_id=%d request_id=%d handle=%d type=%d need_rsp=%d need_authorize=%d "
          "status=%d",
        server_id,
        conn_id,
        write_cb_para->request_id,
        write_cb_para->handle,
        write_cb_para->type,
        write_cb_para->need_rsp,
        write_cb_para->need_authorize,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    ScopedLocalRef<jbyteArray> val(call_back_env.get(), call_back_env->NewByteArray(write_cb_para->length));
    if (val.get()) {
        call_back_env->SetByteArrayRegion(val.get(), 0, write_cb_para->length, (jbyte *)write_cb_para->value);
    }
    call_back_env->CallVoidMethod(mCallbacksObj,
        method_onServerWriteRequest,
        server_id,
        conn_id,
        write_cb_para->request_id,
        write_cb_para->handle,
        write_cb_para->type,
        write_cb_para->need_rsp,
        write_cb_para->need_authorize,
        val.get(),
        status);
    ALOGE("write_request_cb finished");
}

void mtu_changed_cb(uint8_t server_id, uint16_t conn_id, ssap_exchange_info_t *info, errcode_t status)
{
    ALOGE("mtu_changed_cb - server_id=%d conn_id=%d mtu_size=%d version=%d status=%d",
        server_id,
        conn_id,
        info->mtu_size,
        info->version,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    call_back_env->CallVoidMethod(
        mCallbacksObj, method_onMtuChanged, server_id, conn_id, info->mtu_size, info->version, status);
    ALOGE("mtu_changed_cb finished");
}

static ssaps_callbacks_t sSsapServerCallbacks = {NULL /* add_service_cb */,
    NULL /* add_property_cb */,
    NULL /* add_descriptor_cb */,
    start_service_cb,
    delete_all_service_cb,
    read_request_cb,
    write_request_cb,
    mtu_changed_cb};

/**
 * Native function definitions
 */
static void classInitNative(JNIEnv *env, jclass clazz)
{
    method_onServerRegistered = env->GetMethodID(clazz, "onServerRegistered", "(IJJI)V");
    method_onServiceAdded = env->GetMethodID(clazz, "onServiceAdded", "(IIJJ)V");
    method_onPropertyAdded = env->GetMethodID(clazz, "onPropertyAdded", "(IIJJ)V");
    method_onServiceStarted = env->GetMethodID(clazz, "onServiceStarted", "(III)V");
    method_onAllServiceDeleted = env->GetMethodID(clazz, "onAllServiceDeleted", "(II)V");
    method_onServerReadRequest = env->GetMethodID(clazz, "onServerReadRequest", "(IIIIIZZI)V");
    method_onServerWriteRequest = env->GetMethodID(clazz, "onServerWriteRequest", "(IIIIIZZ[BI)V");
    method_onMtuChanged = env->GetMethodID(clazz, "onMtuChanged", "(IIIII)V");
    ALOGE("classInitNative: Success!");
}

void initializeNative(JNIEnv *env, jobject object)
{
    ALOGE("%s", __func__);
    if (mCallbacksObj != NULL) {
        ALOGE("Cleaning up Nearlink SSAP callback object");
        env->DeleteGlobalRef(mCallbacksObj);
        mCallbacksObj = NULL;
    }

    errcode_t status = ssaps_register_callbacks(&sSsapServerCallbacks);
    if (status != ERRCODE_SLE_SUCCESS) {
        ALOGE("Failed to initialize Nearlink SSAP, status: %d", status);
        return;
    }

    mCallbacksObj = env->NewGlobalRef(object);
}

static void cleanupNative(JNIEnv *env, jobject object)
{
    ALOGE("%s", __func__);
    if (mCallbacksObj != NULL) {
        env->DeleteGlobalRef(mCallbacksObj);
        mCallbacksObj = NULL;
    }
}

static jint registerServerNative(JNIEnv *env, jobject object, jlong uuid_lsb, jlong uuid_msb)
{
    ALOGE("%s uuid_lsb=0x%llx, uuid_msb=0x%llx", __func__, uuid_lsb, uuid_msb);
    sle_uuid_t uu;
    uint8_t server_id;

    from_java_uuid(&uu, uuid_msb, uuid_lsb);
    errcode_t error_code = ssaps_register_server(&uu, &server_id);
    if (error_code == ERRCODE_SLE_SUCCESS) {
        env->CallVoidMethod(object, method_onServerRegistered, server_id, uuid_msb, uuid_lsb, error_code);
    }
    RETURN(error_code);
}

static jint unregisterServerNative(JNIEnv *env, jobject object, jint server_id)
{
    ALOGE("%s server_id=%d", __func__, server_id);
    errcode_t error_code = ssaps_unregister_server((uint8_t)server_id);
    RETURN(error_code);
}

static jint addServiceSyncNative(
    JNIEnv *env, jobject object, jint server_id, jlong service_uuid_lsb, jlong service_uuid_msb, jboolean is_primary)
{
    ALOGE("%s server_id=%d, service_uuid_lsb=0x%llx, service_uuid_msb=0x%llx, is_primary=%d",
        __func__,
        server_id,
        service_uuid_lsb,
        service_uuid_msb,
        is_primary);
    uint16_t handle = 0;
    sle_uuid_t uu;
    memset(&uu, 0, sizeof(sle_uuid_t));
    from_java_uuid(&uu, service_uuid_msb, service_uuid_lsb);
    errcode_t error_code = ssaps_add_service_sync((uint8_t)server_id, &uu, (bool)is_primary, &handle);
    if (error_code == ERRCODE_SLE_SUCCESS) {
        env->CallVoidMethod(object, method_onServiceAdded, server_id, handle, service_uuid_lsb, service_uuid_msb);
    }
    RETURN(error_code);
}

static jint addPropertySyncNative(JNIEnv *env, jobject object, jint server_id, jint service_handle,
    jlong property_uuid_lsb, jlong property_uuid_msb, jint permissions, jint operate_indication, jbyteArray value)
{
    ALOGE("%s server_id=%d, service_handle=%d, property_uuid_lsb=0x%llx, property_uuid_msb=0x%llx, permissions=%d, "
          "operate_indication=%d",
        __func__,
        server_id,
        service_handle,
        property_uuid_lsb,
        property_uuid_msb,
        permissions,
        operate_indication);
    uint16_t handle = 0;
    ssaps_property_info_t param;
    memset(&param, 0, sizeof(ssaps_property_info_t));
    from_java_uuid(&(param.uuid), property_uuid_msb, property_uuid_lsb);
    param.permissions = (uint16_t)permissions;
    param.operate_indication = (uint32_t)operate_indication;
    if (value != NULL) {
        param.value_len = (uint16_t)env->GetArrayLength(value);
        jbyte *p_data = env->GetByteArrayElements(value, NULL);
        if (p_data == NULL) {
            ALOGW("Send value is null, no need to send.");
            return ERRCODE_SLE_SUCCESS;
        }
        param.value = (uint8_t *)p_data;
    }
    errcode_t error_code = ssaps_add_property_sync((uint8_t)server_id, (uint16_t)service_handle, &param, &handle);
    if (error_code == ERRCODE_SLE_SUCCESS) {
        env->CallVoidMethod(object, method_onPropertyAdded, server_id, handle, property_uuid_lsb, property_uuid_msb);
    }
    RETURN(error_code);
}

static jint addDescriptorSyncNative(JNIEnv *env, jobject object, jint server_id, jint service_handle,
    jint property_handle, jlong property_uuid_lsb, jlong property_uuid_msb, jint permissions, jint operate_indication,
    jint type, jbyteArray value)
{
    ALOGE("%s server_id=%d, service_handle=%d, property_handle=%d, type=%d",
        __func__,
        server_id,
        service_handle,
        property_handle,
        type);
    ssaps_desc_info_t param;
    memset(&param, 0, sizeof(ssaps_desc_info_t));
    from_java_uuid(&(param.uuid), property_uuid_msb, property_uuid_lsb);
    param.permissions = (uint16_t)permissions;
    param.operate_indication = (uint32_t)operate_indication;
    param.type = (uint8_t)type;
    if (value != NULL) {
        param.value_len = (uint16_t)env->GetArrayLength(value);
        jbyte *p_data = env->GetByteArrayElements(value, NULL);
        if (p_data == NULL) {
            ALOGW("Send value is null, no need to send.");
            return ERRCODE_SLE_SUCCESS;
        }
        param.value = (uint8_t *)p_data;
    }
    errcode_t error_code =
        ssaps_add_descriptor_sync((uint8_t)server_id, (uint16_t)service_handle, (uint16_t)property_handle, &param);
    RETURN(error_code);
}

static jint startServiceNative(JNIEnv *env, jobject object, jint server_id, jint service_handle)
{
    ALOGE("%s server_id=%d, service_handle=%d", __func__, server_id, service_handle);
    errcode_t error_code = ssaps_start_service((uint8_t)server_id, (uint16_t)service_handle);
    RETURN(error_code);
}

static jint deleteAllServicesNative(JNIEnv *env, jobject object, jint server_id)
{
    ALOGE("%s server_id=%d", __func__, server_id);
    errcode_t error_code = ssaps_delete_all_services((uint8_t)server_id);
    RETURN(error_code);
}

static jint sendResponseNative(
    JNIEnv *env, jobject object, jint server_id, jint conn_id, jint request_id, int status, jbyteArray value)
{
    ALOGE("%s server_id=%d, conn_id=%d, request_id=%d, status=%d", __func__, server_id, conn_id, request_id, status);
    ssaps_send_rsp_t param;
    memset(&param, 0, sizeof(ssaps_send_rsp_t));
    param.request_id = (uint16_t)request_id;
    param.status = static_cast<uint8_t>(status);
    if (value != NULL) {
        param.value_len = (uint16_t)env->GetArrayLength(value);
        jbyte *p_data = env->GetByteArrayElements(value, NULL);
        if (p_data == NULL) {
            ALOGW("Send value is null, no need to send.");
            return ERRCODE_SLE_SUCCESS;
        }
        param.value = (uint8_t *)p_data;
    }
    errcode_t error_code = ssaps_send_response((uint8_t)server_id, (uint16_t)conn_id, &param);
    RETURN(error_code);
}

static jint notifyOrIndicateNative(
    JNIEnv *env, jobject object, jint server_id, jint conn_id, jint handle, int type, jbyteArray value)
{
    ALOGE("%s server_id=%d, conn_id=%d, handle=%d, type=%d", __func__, server_id, conn_id, handle, type);
    ssaps_ntf_ind_t param;
    memset(&param, 0, sizeof(ssaps_ntf_ind_t));
    param.handle = (uint16_t)handle;
    param.type = static_cast<uint8_t>(type);
    if (value != NULL) {
        param.value_len = (uint16_t)env->GetArrayLength(value);
        jbyte *p_data = env->GetByteArrayElements(value, NULL);
        if (p_data == NULL) {
            ALOGW("Send value is null, no need to send.");
            return ERRCODE_SLE_SUCCESS;
        }
        param.value = (uint8_t *)p_data;
    }
    errcode_t error_code = ssaps_notify_indicate((uint8_t)server_id, (uint16_t)conn_id, &param);
    RETURN(error_code);
}

static jint setInfoNative(JNIEnv *env, jobject object, jint server_id, jint mtu_size, jint version)
{
    ALOGE("%s server_id=%d, mtu_size=%d, version=%d", __func__, server_id, mtu_size, version);
    ssap_exchange_info_t param;
    memset(&param, 0, sizeof(ssap_exchange_info_t));
    param.mtu_size = (uint32_t)mtu_size;
    param.version = (uint16_t)version;
    errcode_t error_code = ssaps_set_info((uint8_t)server_id, &param);
    RETURN(error_code);
}

// JNI functions defined in SsapServerService class.
static JNINativeMethod sMethods[] = {
    {"classInitNative", "()V", (void *)classInitNative},
    {"initializeNative", "()V", (void *)initializeNative},
    {"cleanupNative", "()V", (void *)cleanupNative},
    {"registerServerNative", "(JJ)I", (void *)registerServerNative},
    {"unregisterServerNative", "(I)I", (void *)unregisterServerNative},
    {"addServiceSyncNative", "(IJJZ)I", (void *)addServiceSyncNative},
    {"addPropertySyncNative", "(IIJJII[B)I", (void *)addPropertySyncNative},
    {"addDescriptorSyncNative", "(IIIJJIII[B)I", (void *)addDescriptorSyncNative},
    {"startServiceNative", "(II)I", (void *)startServiceNative},
    {"deleteAllServicesNative", "(I)I", (void *)deleteAllServicesNative},
    {"sendResponseNative", "(IIII[B)I", (void *)sendResponseNative},
    {"notifyOrIndicateNative", "(IIII[B)I", (void *)notifyOrIndicateNative},
    {"setInfoNative", "(III)I", (void *)setInfoNative},
};

int register_com_android_nearlink_ssap_server(JNIEnv *env)
{
    return jniRegisterNativeMethods(env, "com/android/nearlink/ssap/SsapServerService", sMethods, NELEM(sMethods));
}

}  // namespace android