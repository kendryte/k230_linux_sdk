/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#include "com_android_nearlink_ssap.h"
#include "aapi_sle_ssap_client.h"
#include "sle_connection_manager.h"

#undef LOG_TAG
#define LOG_TAG "SSAP-Client-JNI"

#define CLIENT_ID_BIT 8
#define CONN_ID_MASK 0xFF
#define ARRAY_0 0
#define ARRAY_4 4
#define ARRAY_5 5

namespace android {

void from_java_uuid(sle_uuid_t *uu, jlong uuid_msb, jlong uuid_lsb)
{
    if (uuid_msb == 0 && uuid_lsb == 0) {
        uu->len = 0;
    } else {
        uu->len = SLE_UUID_LEN;
        for (int i = 0; i < 8; i++) {
            uu->uuid[7 - i] = (uuid_msb >> (8 * i)) & 0xFF;
            uu->uuid[15 - i] = (uuid_lsb >> (8 * i)) & 0xFF;
        }
    }
}

uint64_t uuid_lsb(const sle_uuid_t *uu)
{
    uint64_t lsb = 0;

    for (int i = 8; i <= 15; i++) {
        lsb <<= 8;
        lsb |= uu->uuid[i];
    }

    return lsb;
}

uint64_t uuid_msb(const sle_uuid_t *uu)
{
    uint64_t msb = 0;

    for (int i = 0; i <= 7; i++) {
        msb <<= 8;
        msb |= uu->uuid[i];
    }

    return msb;
}

/**
 * Static variables
 */
static jobject mCallbacksObj = NULL;

/**
 * Client callback methods
 */
static jmethodID method_onClientRegistered;           /* self call */
static jmethodID method_onConnectionStateChanged;     /* sle_host call & self call */
static jmethodID method_onStructureFoundComplete;     /* sle_host call & self call */
static jmethodID method_newSsapService;               /* self call */
static jmethodID method_newSsapProperty;              /* self call */
static jmethodID method_newSsapDescriptor;            /* self call */
static jmethodID method_onStructureGot;               /* self call */
static jmethodID method_onReadCfm;                    /* sle_host call */
static jmethodID method_onReadByUuidComplete;         /* sle_host call */
static jmethodID method_onWriteCfm;                   /* sle_host call */
static jmethodID method_onInfoExchanged;              /* sle_host call */
static jmethodID method_onNotification;               /* sle_host call */
static jmethodID method_onReadRemoteRssi;             /* sle_host call */
static jmethodID method_onConnectionParameterUpdated; /* sle_host call */

void connect_state_changed_cb(uint16_t conn_id, const sle_addr_t *addr, sle_acb_state_t conn_state,
    sle_pair_state_t pair_state, sle_disc_reason_t disc_reason)
{
    uint8_t client_id = conn_id >> CLIENT_ID_BIT;
    conn_id &= CONN_ID_MASK;
    ALOGD("connect_state_changed_cb client_id=%d conn_id=%d addr_type=%d addr=%02x:**:**:**:%02x:%02x "
          "conn_state=0x%02x pair_state=0x%02x disc_reason=0x%02x",
        client_id,
        conn_id,
        addr->type,
        addr->addr[ARRAY_0],
        addr->addr[ARRAY_4],
        addr->addr[ARRAY_5],
        conn_state,
        pair_state,
        disc_reason);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    ScopedLocalRef<jbyteArray> val(call_back_env.get(), call_back_env->NewByteArray(SLE_ADDR_LEN));
    if (val.get()) {
        call_back_env->SetByteArrayRegion(val.get(), 0, SLE_ADDR_LEN, (jbyte *)addr->addr);
    }
    call_back_env->CallVoidMethod(mCallbacksObj,
        method_onConnectionStateChanged,
        client_id,
        conn_id,
        addr->type,
        val.get(),
        conn_state,
        pair_state,
        disc_reason);
    ALOGD("connect_state_changed_cb finished");
}

void pair_complete_cb(uint16_t conn_id, const sle_addr_t *addr, errcode_t status)
{
    uint8_t client_id = conn_id >> CLIENT_ID_BIT;
    conn_id &= CONN_ID_MASK;
    ALOGD("pair_complete_callback client_id=%d conn_id=%d addr_type=%d addr=%02x:**:**:**:%02x:%02x status=%d",
        client_id,
        conn_id,
        addr->type,
        addr->addr[ARRAY_0],
        addr->addr[ARRAY_4],
        addr->addr[ARRAY_5],
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    ScopedLocalRef<jbyteArray> val(call_back_env.get(), call_back_env->NewByteArray(SLE_ADDR_LEN));
    if (val.get()) {
        call_back_env->SetByteArrayRegion(val.get(), 0, SLE_ADDR_LEN, (jbyte *)addr->addr);
    }
    call_back_env->CallVoidMethod(mCallbacksObj,
        method_onConnectionStateChanged,
        client_id,
        conn_id,
        addr->type,
        val.get(),
        SLE_ACB_STATE_CONNECTED,
        SLE_PAIR_PAIRED,
        0);
    ALOGD("pair_complete_callback finished");
}

void connect_param_update_cb(uint16_t conn_id, errcode_t status, const sle_connection_param_update_evt_t *param)
{
    uint8_t client_id = conn_id >> CLIENT_ID_BIT;
    conn_id &= CONN_ID_MASK;
    ALOGD("connect_param_update_cb client_id=%d conn_id=%d interval=%d latency=%d supervision=%d status=%d",
        client_id,
        conn_id,
        param->interval,
        param->latency,
        param->supervision,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    call_back_env->CallVoidMethod(mCallbacksObj,
        method_onConnectionParameterUpdated,
        client_id,
        conn_id,
        param->interval,
        param->latency,
        param->supervision,
        status);
    ALOGD("connect_param_update_cb finished");
}

void read_rssi_cb(uint16_t conn_id, int8_t rssi, errcode_t status)
{
    uint8_t client_id = conn_id >> CLIENT_ID_BIT;
    conn_id &= CONN_ID_MASK;
    ALOGD("read_rssi_cb client_id=%d conn_id=%d rssi=%d status=%d", client_id, conn_id, rssi, status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    call_back_env->CallVoidMethod(mCallbacksObj, method_onReadRemoteRssi, client_id, conn_id, rssi, status);
    ALOGD("read_rssi_cb finished");
}

void find_structure_cmp_cb(
    uint8_t client_id, uint16_t conn_id, ssapc_find_structure_result_t *structure_result, errcode_t status)
{
    ALOGD("find_structure_cmp_cb - client_id=%d conn_id=%d type=%d status=%d",
        client_id,
        conn_id,
        structure_result->type,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    call_back_env->CallVoidMethod(mCallbacksObj, method_onStructureFoundComplete, client_id, conn_id, status);
    ALOGD("find_structure_cmp_cb finished");
}

void read_cfm_cb(uint8_t client_id, uint16_t conn_id, ssapc_handle_value_t *read_data, errcode_t status)
{
    ALOGD("read_cfm_cb - client_id=%d conn_id=%d handle=%d type=%d status=%d",
        client_id,
        conn_id,
        read_data->handle,
        read_data->type,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    ScopedLocalRef<jbyteArray> val(call_back_env.get(), call_back_env->NewByteArray(read_data->data_len));
    if (val.get()) {
        call_back_env->SetByteArrayRegion(val.get(), 0, read_data->data_len, (jbyte *)read_data->data);
    }
    call_back_env->CallVoidMethod(
        mCallbacksObj, method_onReadCfm, client_id, conn_id, read_data->handle, read_data->type, val.get(), status);
    ALOGD("read_cfm_cb finished");
}

void read_by_uuid_cmp_cb(
    uint8_t client_id, uint16_t conn_id, ssapc_read_by_uuid_cmp_result_t *cmp_result, errcode_t status)
{
    ALOGD("read_by_uuid_cmp_cb - client_id=%d conn_id=%d type=%d status=%d",
        client_id,
        conn_id,
        cmp_result->type,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    call_back_env->CallVoidMethod(mCallbacksObj,
        method_onReadByUuidComplete,
        client_id,
        conn_id,
        uuid_msb(&(cmp_result->uuid)),
        uuid_lsb(&(cmp_result->uuid)),
        cmp_result->type,
        status);
    ALOGD("read_by_uuid_cmp_cb finished");
}

void write_cfm_cb(uint8_t client_id, uint16_t conn_id, ssapc_write_result_t *write_result, errcode_t status)
{
    ALOGD("write_cfm_cb - client_id=%d conn_id=%d handle=%d type=%d status=%d",
        client_id,
        conn_id,
        write_result->handle,
        write_result->type,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    call_back_env->CallVoidMethod(
        mCallbacksObj, method_onWriteCfm, client_id, conn_id, write_result->handle, write_result->type, status);
    ALOGD("write_cfm_cb finished");
}

void exchange_info_cb(uint8_t client_id, uint16_t conn_id, ssap_exchange_info_t *param, errcode_t status)
{
    ALOGD("exchange_info_cb - client_id=%d conn_id=%d mtu_size=%d version=%d status=%d",
        client_id,
        conn_id,
        param->mtu_size,
        param->version,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    call_back_env->CallVoidMethod(
        mCallbacksObj, method_onInfoExchanged, client_id, conn_id, param->mtu_size, param->version, status);
    ALOGD("exchange_info_cb finished");
}

void notification_cb(uint8_t client_id, uint16_t conn_id, ssapc_handle_value_t *data, errcode_t status)
{
    ALOGD("notification_cb - client_id=%d conn_id=%d handle=%d type=%d status=%d",
        client_id,
        conn_id,
        data->handle,
        data->type,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    ScopedLocalRef<jbyteArray> val(call_back_env.get(), call_back_env->NewByteArray(data->data_len));
    if (val.get()) {
        call_back_env->SetByteArrayRegion(val.get(), 0, data->data_len, (jbyte *)data->data);
    }
    call_back_env->CallVoidMethod(
        mCallbacksObj, method_onNotification, client_id, conn_id, data->handle, data->type, 1, val.get(), status);
    ALOGD("notification_cb finished");
}

void indication_cb(uint8_t client_id, uint16_t conn_id, ssapc_handle_value_t *data, errcode_t status)
{
    ALOGD("indication_cb - client_id=%d conn_id=%d handle=%d type=%d status=%d",
        client_id,
        conn_id,
        data->handle,
        data->type,
        status);
    CallbackEnv call_back_env(__func__);
    if (!call_back_env.valid()) {
        ALOGE("call_back_env is not valid.");
        return;
    }
    ScopedLocalRef<jbyteArray> val(call_back_env.get(), call_back_env->NewByteArray(data->data_len));
    if (val.get()) {
        call_back_env->SetByteArrayRegion(val.get(), 0, data->data_len, (jbyte *)data->data);
    }
    call_back_env->CallVoidMethod(
        mCallbacksObj, method_onNotification, client_id, conn_id, data->handle, data->type, 0, val.get(), status);
    ALOGD("indication_cb finished");
}

static sle_connection_callbacks_t sConnectionCallbacks = {
    connect_state_changed_cb,
    NULL, /* sle_connect_param_update_req_callback */
    connect_param_update_cb,
    NULL, /* sle_auth_complete_callback */
    pair_complete_cb,
    read_rssi_cb,
    NULL /* sle_low_latency_callback */,
    NULL
};

static ssapc_callbacks_t sSsapClientCallbacks = {NULL, /* ssapc_find_structure_callback */
    NULL,                                              /* ssapc_find_property_callback */
    find_structure_cmp_cb,
    read_cfm_cb,
    read_by_uuid_cmp_cb,
    write_cfm_cb,
    exchange_info_cb,
    notification_cb,
    indication_cb};

static aapi_callbacks_t sAndroidApiCallbacks = {sConnectionCallbacks, sSsapClientCallbacks};

/**
 * Native function definitions
 */
static void classInitNative(JNIEnv *env, jclass clazz)
{
    method_onClientRegistered = env->GetMethodID(clazz, "onClientRegistered", "(IJJI)V");
    method_onConnectionStateChanged = env->GetMethodID(clazz, "onConnectionStateChanged", "(III[BIII)V");
    method_onStructureFoundComplete = env->GetMethodID(clazz, "onStructureFoundComplete", "(III)V");
    method_newSsapService =
        env->GetMethodID(clazz, "newSsapService", "(Ljava/util/ArrayList;JJI)Landroid/nearlink/NearlinkSsapService;");
    method_newSsapProperty = env->GetMethodID(clazz,
        "newSsapProperty",
        "(Landroid/nearlink/NearlinkSsapService;JJII)Landroid/nearlink/NearlinkSsapProperty;");
    method_newSsapDescriptor =
        env->GetMethodID(clazz, "newSsapDescriptor", "(Landroid/nearlink/NearlinkSsapProperty;I)V");
    method_onStructureGot = env->GetMethodID(clazz, "onStructureGot", "(IILjava/util/ArrayList;)V");
    method_onReadCfm = env->GetMethodID(clazz, "onReadCfm", "(IIII[BI)V");
    method_onReadByUuidComplete = env->GetMethodID(clazz, "onReadByUuidComplete", "(IIJJII)V");
    method_onWriteCfm = env->GetMethodID(clazz, "onWriteCfm", "(IIIII)V");
    method_onInfoExchanged = env->GetMethodID(clazz, "onInfoExchanged", "(IIIII)V");
    method_onNotification = env->GetMethodID(clazz, "onNotification", "(IIIIZ[BI)V");
    method_onReadRemoteRssi = env->GetMethodID(clazz, "onReadRemoteRssi", "(IIII)V");
    method_onConnectionParameterUpdated = env->GetMethodID(clazz, "onConnectionParameterUpdated", "(IIIIII)V");
    ALOGD("classInitNative: Success!");
}

static void initializeNative(JNIEnv *env, jobject object)
{
    ALOGD("%s", __func__);
    if (mCallbacksObj != NULL) {
        ALOGE("Cleaning up Nearlink SSAP callback object");
        env->DeleteGlobalRef(mCallbacksObj);
        mCallbacksObj = NULL;
    }
    mCallbacksObj = env->NewGlobalRef(object);
}

static void cleanupNative(JNIEnv *env, jobject object)
{
    ALOGD("%s", __func__);
    if (mCallbacksObj != NULL) {
        env->DeleteGlobalRef(mCallbacksObj);
        mCallbacksObj = NULL;
    }
}

static jint registerClientNative(JNIEnv *env, jobject object, jlong uuid_lsb, jlong uuid_msb)
{
    ALOGD("%s", __func__);
    sle_uuid_t uu;
    uint8_t client_id = 0;

    from_java_uuid(&uu, uuid_msb, uuid_lsb);
    errcode_t error_code = aapi_ssapc_register_client(&uu, &sAndroidApiCallbacks, &client_id);
    if (error_code == ERRCODE_SLE_SUCCESS) {
        env->CallVoidMethod(object, method_onClientRegistered, client_id, uuid_msb, uuid_lsb, error_code);
    }
    RETURN(error_code);
}

static jint unregisterClientNative(JNIEnv *env, jobject object, jint client_id)
{
    ALOGD("%s", __func__);
    errcode_t error_code = aapi_ssapc_unregister_client(client_id);
    RETURN(error_code);
}

static jint connectNative(JNIEnv *env, jobject object, jint client_id, jint addr_type, jbyteArray addr)
{
    sle_addr_t sAddr;
    env->GetByteArrayRegion(addr, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
    sAddr.type = addr_type;
    ALOGD("%s client_id=%d addr_type=%d addr=%02x:**:**:**:%02x:%02x",
        __func__,
        client_id,
        addr_type,
        sAddr.addr[ARRAY_0],
        sAddr.addr[ARRAY_4],
        sAddr.addr[ARRAY_5]);
    aapi_conn_state_t out;
    errcode_t error_code = aapi_ssapc_connect_peer((uint8_t)client_id, &sAddr, &out);
    if (error_code == ERRCODE_SLE_SUCCESS && out.state == AAPI_OUT_STATUS_FINISHED) {
        uint8_t pair_state = SLE_PAIR_NONE;
        error_code = sle_get_pair_state(&sAddr, &pair_state);
        if (error_code != ERRCODE_SLE_SUCCESS) {
            ALOGD("%s get pair state failed: %d", __func__, error_code);
        } else {
            ALOGD("Already connected, conn_id=%d conn_state=%d pair_state=%d", out.conn_id, out.conn_state, pair_state);
            ScopedLocalRef<jbyteArray> val(env, env->NewByteArray(SLE_ADDR_LEN));
            if (val.get()) {
                env->SetByteArrayRegion(val.get(), 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
            }
            env->CallVoidMethod(object,
                method_onConnectionStateChanged,
                client_id,
                out.conn_id,
                addr_type,
                val.get(),
                out.conn_state,
                pair_state,
                0);
        }
    }
    RETURN(error_code);
}

static jint disconnectNative(JNIEnv *env, jobject object, jint client_id, jint addr_type, jbyteArray addr)
{
    sle_addr_t sAddr;
    env->GetByteArrayRegion(addr, 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
    sAddr.type = addr_type;
    ALOGD("%s client_id=%d addr_type=%d addr=%02x:**:**:**:%02x:%02x",
        __func__,
        client_id,
        addr_type,
        sAddr.addr[ARRAY_0],
        sAddr.addr[ARRAY_4],
        sAddr.addr[ARRAY_5]);
    uint8_t out_statu;
    errcode_t error_code = aapi_ssapc_disconnect_peer((uint8_t)client_id, &sAddr, &out_statu);
    if (error_code == ERRCODE_SLE_SUCCESS && out_statu == AAPI_OUT_STATUS_FINISHED) {
        ScopedLocalRef<jbyteArray> val(env, env->NewByteArray(SLE_ADDR_LEN));
        if (val.get()) {
            env->SetByteArrayRegion(val.get(), 0, SLE_ADDR_LEN, (jbyte *)sAddr.addr);
        }
        env->CallVoidMethod(object,
            method_onConnectionStateChanged,
            client_id,
            -1,
            addr_type,
            val.get(),
            SLE_ACB_STATE_DISCONNECTED,
            SLE_PAIR_NONE,
            SLE_DISCONNECT_BY_LOCAL);
    }
    RETURN(error_code);
}

static jint findStructureNative(JNIEnv *env, jobject object, jint client_id, jint conn_id)
{
    ALOGD("%s client_id=%d conn_id=%d", __func__, client_id, conn_id);
    uint8_t out_statu;
    errcode_t error_code = aapi_ssapc_find_structure((uint8_t)client_id, (uint16_t)conn_id, &out_statu);
    if (error_code == ERRCODE_SLE_SUCCESS && out_statu == AAPI_OUT_STATUS_FINISHED) {
        ALOGD("Already found.");
        env->CallVoidMethod(object, method_onStructureFoundComplete, client_id, conn_id, ERRCODE_SLE_SUCCESS);
    }
    RETURN(error_code);
}

static void getStructureNative(JNIEnv *env, jobject object, jint client_id, jint conn_id)
{
    ALOGD("%s client_id=%d conn_id=%d", __func__, client_id, conn_id);
    aapi_ssapc_structure_node_t *p_structure = NULL;
    errcode_t error_code = aapi_ssapc_get_structure((uint8_t)client_id, (uint16_t)conn_id, &p_structure);
    if (error_code == ERRCODE_SLE_SUCCESS) {
        jclass arrayListclazz = env->FindClass("java/util/ArrayList");
        ScopedLocalRef<jobject> array(
            env, env->NewObject(arrayListclazz, env->GetMethodID(arrayListclazz, "<init>", "()V")));
        while (p_structure != NULL) {
            aapi_ssapc_structure_t *srv = &(p_structure->service);
            jobject service = env->CallObjectMethod(object,
                method_newSsapService,
                array.get(),
                uuid_msb(&(srv->uuid)),
                uuid_lsb(&(srv->uuid)),
                srv->start_hdl);
            aapi_ssapc_property_node_t *prop = srv->property_list;
            while (prop != NULL) {
                jobject property = env->CallObjectMethod(object,
                    method_newSsapProperty,
                    service,
                    uuid_msb(&(prop->property.uuid)),
                    uuid_lsb(&(prop->property.uuid)),
                    prop->property.handle,
                    prop->property.operate_indication);
                for (int i = 0; i < prop->property.descriptors_count; i++) {
                    env->CallVoidMethod(object, method_newSsapDescriptor, property, prop->property.descriptors_type[i]);
                }
                prop = prop->next;
            }
            p_structure = p_structure->next;
        }
        env->CallVoidMethod(object, method_onStructureGot, client_id, conn_id, array.get(), error_code);
    } else {
        env->CallVoidMethod(object, method_onStructureGot, client_id, conn_id, NULL, error_code);
    }
}

static jint readRequestByUuidNative(JNIEnv *env, jobject object, jint client_id, jint conn_id, jint type,
    jint start_handle, jint end_handle, jlong property_uuid_lsb, jlong property_uuid_msb)
{
    ALOGD("%s client_id=%d conn_id=%d type=%d start_handle=%d end_handle=%d property_uuid_lsb=0x%llx "
          "property_uuid_msb=0x%llx",
        __func__,
        client_id,
        conn_id,
        type,
        start_handle,
        end_handle,
        property_uuid_lsb,
        property_uuid_msb);
    ssapc_read_req_by_uuid_param_t param;
    memset(&param, 0, sizeof(ssapc_read_req_by_uuid_param_t));
    param.type = (uint8_t)type;
    param.start_hdl = (uint16_t)start_handle;
    param.end_hdl = (uint16_t)end_handle;
    from_java_uuid(&(param.uuid), property_uuid_msb, property_uuid_lsb);
    errcode_t error_code = aapi_ssapc_read_req_by_uuid((uint8_t)client_id, (uint16_t)conn_id, &param);
    RETURN(error_code);
}

static jint readRequestNative(JNIEnv *env, jobject object, jint client_id, jint conn_id, jint handle, jint type)
{
    ALOGD("%s client_id=%d conn_id=%d handle=%d type=%d", __func__, client_id, conn_id, handle, type);
    errcode_t error_code = aapi_ssapc_read_req((uint8_t)client_id, (uint16_t)conn_id, (uint16_t)handle, (uint8_t)type);
    RETURN(error_code);
}

static jint writeRequestNative(JNIEnv *env, jobject object, jint client_id, jint conn_id, jint handle, jint type,
    jboolean need_response, jbyteArray data)
{
    ALOGD("%s client_id=%d conn_id=%d handle=%d type=%d need_response=%d",
        __func__,
        client_id,
        conn_id,
        handle,
        type,
        need_response);
    ssapc_write_param_t param;
    memset(&param, 0, sizeof(ssapc_write_param_t));
    param.handle = (uint16_t)handle;
    param.type = (uint8_t)type;
    param.data_len = (uint16_t)env->GetArrayLength(data);
    jbyte *p_data = env->GetByteArrayElements(data, NULL);
    if (p_data == NULL) {
        ALOGW("Send value is null, no need to send.");
        return ERRCODE_SLE_SUCCESS;
    }
    param.data = (uint8_t *)p_data;
    errcode_t error_code;
    if (need_response) {
        error_code = aapi_ssapc_write_req((uint8_t)client_id, (uint16_t)conn_id, &param);
    } else {
        error_code = ssapc_write_cmd((uint8_t)client_id, (uint16_t)conn_id, &param);
    }
    RETURN(error_code);
}

static jint exchangeInfoRequestNative(
    JNIEnv *env, jobject object, jint client_id, jint conn_id, jint mtu_size, jint version)
{
    ALOGD("%s client_id=%d conn_id=%d mtu_size=%d version=%d", __func__, client_id, conn_id, mtu_size, version);
    ssap_exchange_info_t param;
    memset(&param, 0, sizeof(ssap_exchange_info_t));
    param.mtu_size = (uint32_t)mtu_size;
    param.version = (uint16_t)version;
    errcode_t error_code = ssapc_exchange_info_req((uint8_t)client_id, (uint16_t)conn_id, &param);
    RETURN(error_code);
}

static jint readRemoteRssiNative(JNIEnv *env, jobject object, jint client_id, jint conn_id)
{
    ALOGD("%s client_id=%d conn_id=%d", __func__, client_id, conn_id);
    errcode_t error_code = aapi_sle_read_remote_device_rssi((uint8_t)client_id, (uint16_t)conn_id);
    RETURN(error_code);
}

static jint connectionParameterUpdateNative(JNIEnv *env, jobject object, jint client_id, jint conn_id,
    jint min_interval, jint max_interval, jint latency, jint timeout)
{
    ALOGD("%s client_id=%d conn_id=%d", __func__, client_id, conn_id);
    sle_connection_param_update_t param;
    param.conn_id = (uint16_t)conn_id;
    param.interval_min = (uint16_t)min_interval;
    param.interval_max = (uint16_t)max_interval;
    param.max_latency = (uint16_t)latency;
    param.supervision_timeout = (uint16_t)timeout;
    errcode_t error_code = sle_update_connect_param(&param);
    RETURN(error_code);
}

static jint setTxDataLengthNative(JNIEnv *env, jobject object, jint client_id, jint conn_id, jint tx_octets)
{
    ALOGD("%s client_id=%d conn_id=%d tx_octets=%d", __func__, client_id, conn_id, tx_octets);
    errcode_t error_code = sle_set_data_len((uint16_t) conn_id, (uint16_t) tx_octets);
    RETURN(error_code);
}

// JNI functions defined in SsapClientService class.
static JNINativeMethod sMethods[] = {{"classInitNative", "()V", (void *)classInitNative},
    {"initializeNative", "()V", (void *)initializeNative},
    {"cleanupNative", "()V", (void *)cleanupNative},
    {"registerClientNative", "(JJ)I", (void *)registerClientNative},
    {"unregisterClientNative", "(I)I", (void *)unregisterClientNative},
    {"connectNative", "(II[B)I", (void *)connectNative},
    {"disconnectNative", "(II[B)I", (void *)disconnectNative},
    {"findStructureNative", "(II)I", (void *)findStructureNative},
    {"getStructureNative", "(II)V", (void *)getStructureNative},
    {"readRequestByUuidNative", "(IIIIIJJ)I", (void *)readRequestByUuidNative},
    {"readRequestNative", "(IIII)I", (void *)readRequestNative},
    {"writeRequestNative", "(IIIIZ[B)I", (void *)writeRequestNative},
    {"exchangeInfoRequestNative", "(IIII)I", (void *)exchangeInfoRequestNative},
    {"readRemoteRssiNative", "(II)I", (void *)readRemoteRssiNative},
    {"connectionParameterUpdateNative", "(IIIIII)I", (void *)connectionParameterUpdateNative},
    {"setTxDataLengthNative", "(III)I", (void *)setTxDataLengthNative},
};

int register_com_android_nearlink_ssap_client(JNIEnv *env)
{
    return jniRegisterNativeMethods(env, "com/android/nearlink/ssap/SsapClientService", sMethods, NELEM(sMethods));
}
}  // namespace android