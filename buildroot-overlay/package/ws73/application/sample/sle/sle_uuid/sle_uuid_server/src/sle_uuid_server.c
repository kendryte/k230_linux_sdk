/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: SLE UUID Server Source.
 */
#include <sys/time.h>
#include "securec.h"
#include "sle_common.h"
#include "sle_errcode.h"
#include "sle_connection_manager.h"
#include "sle_device_discovery.h"
#include "sle_uuid_server_adv.h"
#include "sle_uuid_server.h"

#define OCTET_BIT_LEN                     8
#define UUID_LEN_2                        2
#define UUID_INDEX                        14
#define BT_INDEX_4                        4
#define BT_INDEX_0                        0
#define UUID_BUFF_LENGTH                  0x100
#define UUID_16BIT_LEN                    2
#define UUID_128BIT_LEN                   16
#define sample_at_log_print(fmt, args...) printf(fmt, ##args)
#define SLE_UUID_SERVER_LOG               "[sle uuid server]"
#define SLE_SLEEP_US                      10000
#define RECV_PKT_CNT                      100
#define PKT_LEN                           200
#define SERVER_MTU_LEN                    1500
#define HUNDRED                           100

/* sle server app uuid for test */
static char g_sle_uuid_app_uuid[UUID_LEN_2] = { 0x12, 0x34 };
/* sle connect acb handle */
static uint16_t g_sle_conn_hdl = 0;
/* sle server handle */
static uint8_t g_server_id = 0;
/* sle service handle */
static uint16_t g_service_handle = 0;
/* sle ntf property handle */
static uint16_t g_indication_property_hdl = 0;
/* sle service count */
static uint8_t g_service_cnt = 0;
/* sle connect count */
static uint16_t g_connect_cnt = 0;
/* number of sle receive package from client */
static int g_recv_pkt_num = 0;
/* time of receiving the first package */
static uint64_t g_begin_tv_us = 0;
/* time of receiving the RECV_PKT_CNT package */
static uint64_t g_end_tv_us = 0;

static uint8_t g_sle_uuid_base[] = { 0x37, 0xBE, 0xA8, 0x80, 0xFC, 0x70, 0x11, 0xEA, \
    0xB7, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static void encode2byte_little(uint8_t *_ptr, uint16_t data)
{
    *(uint8_t *)((_ptr) + 1) = (uint8_t)((data) >> 0x8);
    *(uint8_t *)(_ptr) = (uint8_t)(data);
}

static void sle_uuid_set_base(sle_uuid_t *out)
{
    errcode_t ret = memcpy_s(out->uuid, SLE_UUID_LEN, g_sle_uuid_base, SLE_UUID_LEN);
    if (ret != EOK) {
        sample_at_log_print("%s sle_uuid_set_base memcpy fail\n", SLE_UUID_SERVER_LOG);
        out->len = 0;
        return ;
    }
    out->len = UUID_LEN_2;
}

static void sle_uuid_setu2(uint16_t u2, sle_uuid_t *out)
{
    sle_uuid_set_base(out);
    out->len = UUID_LEN_2;
    encode2byte_little(&out->uuid[UUID_INDEX], u2);
}

static void sle_uuid_print(sle_uuid_t *uuid)
{
    if (uuid == NULL) {
        sample_at_log_print("%s uuid_print,uuid is null\r\n", SLE_UUID_SERVER_LOG);
        return;
    }
    if (uuid->len == UUID_16BIT_LEN) {
        sample_at_log_print("%s uuid: %02x %02x.\n", SLE_UUID_SERVER_LOG,
            uuid->uuid[14], uuid->uuid[15]); /* 14 15: uuid index */
    } else if (uuid->len == UUID_128BIT_LEN) {
        sample_at_log_print("%s uuid: \n", SLE_UUID_SERVER_LOG); /* 14 15: uuid index */
        sample_at_log_print("%s 0x%02x 0x%02x 0x%02x \n", SLE_UUID_SERVER_LOG, uuid->uuid[0], uuid->uuid[1],
            uuid->uuid[2], uuid->uuid[3]);
        sample_at_log_print("%s 0x%02x 0x%02x 0x%02x \n", SLE_UUID_SERVER_LOG, uuid->uuid[4], uuid->uuid[5],
            uuid->uuid[6], uuid->uuid[7]);
        sample_at_log_print("%s 0x%02x 0x%02x 0x%02x \n", SLE_UUID_SERVER_LOG, uuid->uuid[8], uuid->uuid[9],
            uuid->uuid[10], uuid->uuid[11]);
        sample_at_log_print("%s 0x%02x 0x%02x 0x%02x \n", SLE_UUID_SERVER_LOG, uuid->uuid[12], uuid->uuid[13],
            uuid->uuid[14], uuid->uuid[15]);
    }
}

/* device通过handle向host发送数据：report */
errcode_t sle_uuid_server_send_report_by_handle(uint16_t handle, uint8_t *data, uint8_t len)
{
    sample_at_log_print("sle uuid server send by handle: %x\r\n", handle);
    ssaps_ntf_ind_t param = {0};
    param.handle = handle;
    param.type = SSAP_PROPERTY_TYPE_VALUE;
    param.value = data;
    param.value_len = len;

    sample_at_log_print("sle uuid server send by handle g_server_id 0x%x g_sle_conn_hdl 0x%x\r\n",
        g_server_id, g_sle_conn_hdl);
    return ssaps_notify_indicate(g_server_id, g_sle_conn_hdl, &param);
}

static void ssaps_mtu_changed_cbk(uint8_t server_id, uint16_t conn_id,  ssap_exchange_info_t *mtu_size,
    errcode_t status)
{
    sample_at_log_print("%s ssaps ssaps_mtu_changed_cbk callback server_id:%x, conn_id:%x, mtu_size:%x, status:%x\r\n",
        SLE_UUID_SERVER_LOG, server_id, conn_id, mtu_size->mtu_size, status);
}

static void ssaps_start_service_cbk(uint8_t server_id, uint16_t handle, errcode_t status)
{
    sample_at_log_print("%s start service cbk callback server_id:%d, handle:%x, status:%x\r\n", SLE_UUID_SERVER_LOG,
        server_id, handle, status);
    g_service_cnt++;
    if (status == 0 /* wait all service start */) {
        errcode_t ret = sle_uuid_server_adv_init();
        if (ret != ERRCODE_SLE_SUCCESS) {
            sample_at_log_print("%s sle_uuid_server_init,sle_uuid_server_adv_init fail :%x\r\n",
                                SLE_UUID_SERVER_LOG, ret);
        }
    }
}

static void ssaps_add_service_cbk(uint8_t server_id, sle_uuid_t *uuid, uint16_t handle, errcode_t status)
{
    sample_at_log_print("%s add service cbk callback server_id:%x, handle:%x, status:%x\r\n", SLE_UUID_SERVER_LOG,
        server_id, handle, status);
    sle_uuid_print(uuid);
}

static void ssaps_add_property_cbk(uint8_t server_id, sle_uuid_t *uuid, uint16_t service_handle,
    uint16_t handle, errcode_t status)
{
    sample_at_log_print("%s add property cbk callback server_id:%x, service_handle:%x,handle:%x, status:%x\r\n",
        SLE_UUID_SERVER_LOG, server_id, service_handle, handle, status);
    sle_uuid_print(uuid);
}

static void ssaps_add_descriptor_cbk(uint8_t server_id, sle_uuid_t *uuid, uint16_t service_handle,
    uint16_t property_handle, errcode_t status)
{
    sample_at_log_print("%s add descriptor cbk callback server_id:%x, service_handle:%x, property_handle:%x, \
        status:%x\r\n", SLE_UUID_SERVER_LOG, server_id, service_handle, property_handle, status);
    sle_uuid_print(uuid);
}

static void ssaps_delete_all_service_cbk(uint8_t server_id, errcode_t status)
{
    sample_at_log_print("%s delete all service callback server_id:%x, status:%x\r\n", SLE_UUID_SERVER_LOG,
        server_id, status);
    g_service_cnt = 0;
    ssaps_unregister_server(server_id);
}

static errcode_t sle_ssaps_register_cbks(ssaps_read_request_callback ssaps_read_callback, ssaps_write_request_callback
    ssaps_write_callback)
{
    errcode_t ret;
    ssaps_callbacks_t ssaps_cbk = {0};
    ssaps_cbk.add_service_cb = ssaps_add_service_cbk;
    ssaps_cbk.add_property_cb = ssaps_add_property_cbk;
    ssaps_cbk.add_descriptor_cb = ssaps_add_descriptor_cbk;
    ssaps_cbk.start_service_cb = ssaps_start_service_cbk;
    ssaps_cbk.delete_all_service_cb = ssaps_delete_all_service_cbk;
    ssaps_cbk.mtu_changed_cb = ssaps_mtu_changed_cbk;
    ssaps_cbk.read_request_cb = ssaps_read_callback;
    ssaps_cbk.write_request_cb = ssaps_write_callback;
    ret = ssaps_register_callbacks(&ssaps_cbk);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_ssaps_register_cbks,ssaps_register_callbacks fail :%x\r\n", SLE_UUID_SERVER_LOG,
            ret);
        return ret;
    }
    return ERRCODE_SLE_SUCCESS;
}

static errcode_t sle_uuid_server_service_add(void)
{
    sle_uuid_t service_uuid = {0};
    sle_uuid_setu2(SLE_UUID_SERVER_SERVICE, &service_uuid);
    bool is_primary = 1;
    errcode_t ret = ssaps_add_service_sync(g_server_id, &service_uuid, is_primary, &g_service_handle);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle uuid add service fail, ret:%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ERRCODE_SLE_FAIL;
    }
    return ERRCODE_SLE_SUCCESS;
}

static errcode_t sle_uuid_server_tx_property_add(uint8_t server_id, uint16_t srvc_handle)
{
    errcode_t ret = ERRCODE_SLE_SUCCESS;

    sle_uuid_t property_uuid = {0};
    sle_uuid_setu2(SLE_UUID_SERVER_TX, &property_uuid);

    ssaps_property_info_t property = {0};
    uint16_t handle = 0xFFFF;
    uint8_t property_value[] = {0x54, 0x58, 0x63};
    property.uuid = property_uuid;
    property.permissions = SLE_UUID_TX_PERMISSIONS;
    property.operate_indication = SLE_UUID_TX_OPERATION_INDICATION;
    property.value = property_value;
    property.value_len = sizeof(property_value);

    ret |= ssaps_add_property_sync(server_id, srvc_handle, &property, &handle);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle uuid add property fail, ret:%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ERRCODE_SLE_FAIL;
    }
    g_indication_property_hdl = handle;

    sample_at_log_print("%s sle uuid add property succ, g_tx_handle:%x\r\n",
                            SLE_UUID_SERVER_LOG, g_indication_property_hdl);

    ssaps_desc_info_t descriptor = {0};
    uint8_t ntf_value[] = { 0x01, 0x00 };
    sle_uuid_t descriptor_uuid = {0};
    sle_uuid_setu2(SLE_UUID_CONFIGURATION, &descriptor_uuid);
    descriptor.uuid = descriptor_uuid;
    descriptor.permissions = SLE_UUID_TX_PERMISSIONS;
    descriptor.type = SSAP_DESCRIPTOR_CLIENT_CONFIGURATION;
    descriptor.operate_indication = SLE_UUID_TX_OPERATION_INDICATION;
    descriptor.value = ntf_value;
    descriptor.value_len = sizeof(ntf_value);

    ret |= ssaps_add_descriptor_sync(server_id, srvc_handle, handle, &descriptor);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle uuid add descriptor fail, ret:%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ERRCODE_SLE_FAIL;
    }
    return ERRCODE_SLE_SUCCESS;
}

static errcode_t sle_uuid_server_rx_property_add(uint8_t server_id, uint16_t srvc_handle)
{
    errcode_t ret = ERRCODE_SLE_SUCCESS;

    sle_uuid_t property_uuid = {0};
    sle_uuid_setu2(SLE_UUID_SERVER_RX, &property_uuid);

    ssaps_property_info_t property = {0};
    uint16_t handle = 0xFFFF;
    uint8_t property_value[] = {0x52, 0x58, 0x63};
    property.uuid = property_uuid;
    property.permissions = SLE_UUID_RX_PERMISSIONS;
    property.operate_indication = SLE_UUID_RX_OPERATION_INDICATION;
    property.value = property_value;
    property.value_len = sizeof(property_value);
    ret |= ssaps_add_property_sync(server_id, srvc_handle, &property, &handle);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle uuid add property fail, ret:%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ERRCODE_SLE_FAIL;
    }

    sample_at_log_print("%s sle uuid add property succ, rx_handle:%x\r\n", SLE_UUID_SERVER_LOG, handle);

    ssaps_desc_info_t descriptor = {0};
    uint8_t ntf_value[] = {0x52, 0x58, 0x64};
    sle_uuid_t descriptor_uuid = {0};
    sle_uuid_setu2(SLE_UUID_CONFIGURATION, &descriptor_uuid);
    descriptor.uuid = descriptor_uuid;
    descriptor.permissions = SLE_UUID_RX_PERMISSIONS;
    descriptor.type = SSAP_DESCRIPTOR_CLIENT_CONFIGURATION;
    descriptor.operate_indication = SLE_UUID_RX_OPERATION_INDICATION;
    descriptor.value = ntf_value;
    descriptor.value_len = sizeof(ntf_value);
    ret |= ssaps_add_descriptor_sync(server_id, srvc_handle, handle, &descriptor);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle uuid add descriptor fail, ret:%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ERRCODE_SLE_FAIL;
    }
    return ERRCODE_SLE_SUCCESS;
}

static errcode_t sle_uuid_server_add(void)
{
    errcode_t ret = ERRCODE_SLE_SUCCESS;
    sle_uuid_t app_uuid = {0};
    ssap_exchange_info_t server_mtu = {SERVER_MTU_LEN, 1};

    sample_at_log_print("%s sle uuid add service in\r\n", SLE_UUID_SERVER_LOG);
    app_uuid.len = sizeof(g_sle_uuid_app_uuid);
    if (memcpy_s(app_uuid.uuid, app_uuid.len, g_sle_uuid_app_uuid, sizeof(g_sle_uuid_app_uuid)) != EOK) {
        return ERRCODE_SLE_FAIL;
    }
    ssaps_register_server(&app_uuid, &g_server_id);
    if (sle_uuid_server_service_add() != ERRCODE_SLE_SUCCESS) {
        ssaps_unregister_server(g_server_id);
        return ERRCODE_SLE_FAIL;
    }
    ret = ssaps_set_info(g_server_id, &server_mtu);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle uuid set mtu failed, ret:%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ERRCODE_SLE_FAIL;
    }
    if (sle_uuid_server_tx_property_add(g_server_id, g_service_handle) != ERRCODE_SLE_SUCCESS) {
        ssaps_unregister_server(g_server_id);
        return ERRCODE_SLE_FAIL;
    }
    if (sle_uuid_server_rx_property_add(g_server_id, g_service_handle) != ERRCODE_SLE_SUCCESS) {
        ssaps_unregister_server(g_server_id);
        return ERRCODE_SLE_FAIL;
    }
    sample_at_log_print("%s sle uuid add service, server_id:%x, service_handle:%x, indication_handle:%x\r\n",
        SLE_UUID_SERVER_LOG, g_server_id, g_service_handle, g_indication_property_hdl);
    ret = ssaps_start_service(g_server_id, g_service_handle);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle uuid add service fail, ret:%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ERRCODE_SLE_FAIL;
    }
    sample_at_log_print("%s sle uuid add service out\r\n", SLE_UUID_SERVER_LOG);
    return ERRCODE_SLE_SUCCESS;
}

static void sle_ssaps_read_request_cbk(uint8_t server_id, uint16_t conn_id, ssaps_req_read_cb_t *read_cb_para,
    errcode_t status)
{
    sample_at_log_print("sle uuid server read request cbk.\r\n");
}

// 返回微秒
uint64_t sle_khtest_clock_gettime_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL);
}

static uint32_t GetFloatInt(float in)
{
    return (uint32_t)(((uint64_t)(in * HUNDRED)) / HUNDRED);
}

static uint32_t GetFloatDec(float in)
{
    return (uint32_t)(((uint64_t)(in * HUNDRED)) % HUNDRED);
}

static void sle_ssaps_write_request_cbk(uint8_t server_id, uint16_t conn_id, ssaps_req_write_cb_t *write_cb_para,
    errcode_t status)
{
    g_recv_pkt_num++;
    if (g_recv_pkt_num == 1) {
        g_begin_tv_us = sle_khtest_clock_gettime_us();
    } else if (g_recv_pkt_num == RECV_PKT_CNT) {
        g_end_tv_us = sle_khtest_clock_gettime_us();
        float time = (float)(g_end_tv_us - g_begin_tv_us) / 1000000.0;
        float speed = PKT_LEN * RECV_PKT_CNT * 8 / time / 1000.0;
        printf("speed = %d.%d Kbps\r\n", GetFloatInt(speed), GetFloatDec(speed));
        g_recv_pkt_num = 0;
    }
}

static void sle_sspas_reset_count_rate(void)
{
    g_recv_pkt_num = 0;
    g_begin_tv_us = 0;
    g_end_tv_us = 0;
}

static void sle_connect_state_changed_cbk(uint16_t conn_id, const sle_addr_t *addr,
    sle_acb_state_t conn_state, sle_pair_state_t pair_state, sle_disc_reason_t disc_reason)
{
    sample_at_log_print("%s connect state changed callback conn_id:0x%02x, conn_state:0x%x, pair_state:0x%x, \
        disc_reason:0x%x\r\n", SLE_UUID_SERVER_LOG,conn_id, conn_state, pair_state, disc_reason);
    sample_at_log_print("addr:\n");
    for (uint8_t i = 0; i < SLE_ADDR_LEN; i++) {
        sample_at_log_print("%2x ", addr->addr[i]);
    }
    sample_at_log_print("\n");
    
    if (conn_state == SLE_ACB_STATE_CONNECTED) {
        g_sle_conn_hdl = conn_id;
        g_connect_cnt++;
    }
    if (conn_state == SLE_ACB_STATE_DISCONNECTED) {
        sle_sspas_reset_count_rate();
        if (g_connect_cnt > 0) {
            g_connect_cnt--;
        }
        if (disc_reason != SLE_DISCONNECT_BY_LOCAL) {
            sle_uuid_server_adv_init();
        }
        g_sle_conn_hdl = 0;
    }
}

static void sle_pair_complete_cbk(uint16_t conn_id, const sle_addr_t *addr, errcode_t status)
{
    sample_at_log_print("%s pair complete conn_id:%02x, status:%x\r\n", SLE_UUID_SERVER_LOG,
        conn_id, status);
    sample_at_log_print("%s pair complete addr:%02x:**:**:**:%02x:%02x\r\n", SLE_UUID_SERVER_LOG,
        addr->addr[BT_INDEX_0], addr->addr[BT_INDEX_4]);
}

static errcode_t sle_conn_register_cbks(void)
{
    errcode_t ret;
    sle_connection_callbacks_t conn_cbks = {0};
    conn_cbks.connect_state_changed_cb = sle_connect_state_changed_cbk;
    conn_cbks.pair_complete_cb = sle_pair_complete_cbk;
    ret = sle_connection_register_callbacks(&conn_cbks);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_conn_register_cbks,sle_connection_register_callbacks fail :%x\r\n",
        SLE_UUID_SERVER_LOG, ret);
        return ret;
    }
    return ERRCODE_SLE_SUCCESS;
}

static errcode_t sle_uuid_server_register_persistence_info()
{
    char buff[BYTE_LEN_128] = {0};
    char *path = getcwd(buff, BYTE_LEN_128);
    sample_at_log_print("%s current path: %s", SLE_UUID_SERVER_LOG, buff);
    return sle_dev_manager_register_file_path(buff);
}

/* 初始化uuid server */
errcode_t sle_uuid_server_init(void)
{
    errcode_t ret = ERRCODE_SLE_SUCCESS;
    ret |= sle_uuid_announce_register_cbks();
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_uuid_server_init,sle_uuid_announce_register_cbks fail :%x\r\n",
        SLE_UUID_SERVER_LOG, ret);
        return ret;
    }
    ret |= sle_conn_register_cbks();
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_uuid_server_init,sle_conn_register_cbks fail :%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ret;
    }
    ret |= sle_ssaps_register_cbks(sle_ssaps_read_request_cbk, sle_ssaps_write_request_cbk);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_uuid_server_init,sle_ssaps_register_cbks fail :%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ret;
    }
    ret |= sle_uuid_server_register_persistence_info();
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_uuid_server_init,reg persistence_info failed, errcode: %d\n",
            SLE_UUID_SERVER_LOG, ret);
    }
    ret |= enable_sle();
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_uuid_server_init,enable_sle fail :%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ret;
    }
    sample_at_log_print("%s init ok\r\n", SLE_UUID_SERVER_LOG);
    return ERRCODE_SLE_SUCCESS;
}

/* 反初始化uuid server */
errcode_t sle_uuid_server_deinit(void)
{
    sample_at_log_print("%s deinit\r\n", SLE_UUID_SERVER_LOG);
    ssaps_delete_all_services(g_server_id);
    if (g_service_cnt == 0) {
        usleep(SLE_SLEEP_US);
    }
    sle_uuid_server_adv_deinit();

    disable_sle();
    return ERRCODE_SLE_SUCCESS;
}

errcode_t sle_enable_server_cbk(void)
{
    sample_at_log_print("sle enable server callback.\r\n");
    errcode_t ret = ERRCODE_SLE_SUCCESS;
    
    ret |= sle_uuid_server_add();
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_uuid_server_init,sle_uuid_server_add fail :%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ret;
    }
    return ERRCODE_SLE_SUCCESS;
}
