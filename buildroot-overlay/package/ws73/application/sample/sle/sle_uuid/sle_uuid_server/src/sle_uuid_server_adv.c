/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: sle adv config for sle uuid server.
 */
#include "securec.h"
#include "errcode.h"
#include "sle_common.h"
#include "sle_uuid_server.h"
#include "sle_device_discovery.h"
#include "sle_errcode.h"
#include "string.h"
#include "sle_uuid_server_adv.h"

/* sle device name */
#define NAME_MAX_LENGTH 16
/* 连接调度间隔12.5ms，单位125us */
#define SLE_CONN_INTV_MIN_DEFAULT                 0x10
/* 连接调度间隔12.5ms，单位125us */
#define SLE_CONN_INTV_MAX_DEFAULT                 0x10
/* 连接调度间隔25ms，单位125us */
#define SLE_ADV_INTERVAL_MIN_DEFAULT              0xC8
/* 连接调度间隔25ms，单位125us */
#define SLE_ADV_INTERVAL_MAX_DEFAULT              0xC8
/* 超时时间5000ms，单位10ms */
#define SLE_CONN_SUPERVISION_TIMEOUT_DEFAULT      0x1F4
/* 超时时间4990ms，单位10ms */
#define SLE_CONN_MAX_LATENCY                      0x1F3
/* 广播发送功率 */
#define SLE_ADV_TX_POWER  10
/* 广播ID */
#define SLE_ADV_HANDLE_DEFAULT                    1
/* 最大广播数据长度 */
#define SLE_ADV_DATA_LEN_MAX                      251
/* 广播名称 */
static uint8_t sle_local_name[NAME_MAX_LENGTH] = "sle_uuid_server";
#define SLE_SERVER_INIT_DELAY_MS    1000
#define sample_at_log_print(fmt, args...) printf(fmt, ##args)
#define SLE_UUID_SERVER_LOG "[sle uuid server]"

static uint16_t sle_set_adv_local_name(uint8_t *adv_data, uint16_t max_len)
{
    errno_t ret;
    uint8_t index = 0;

    uint8_t *local_name = sle_local_name;
    uint8_t local_name_len = sizeof(sle_local_name) - 1;
    sample_at_log_print("%s local_name_len = %d\r\n", SLE_UUID_SERVER_LOG, local_name_len);
    sample_at_log_print("%s local_name: ", SLE_UUID_SERVER_LOG);
    for (uint8_t i = 0; i < local_name_len; i++) {
        sample_at_log_print("0x%02x ", local_name[i]);
    }
    sample_at_log_print("\r\n");
    adv_data[index++] = local_name_len + 1;
    adv_data[index++] = SLE_ADV_DATA_TYPE_COMPLETE_LOCAL_NAME;
    ret = memcpy_s(&adv_data[index], max_len - index, local_name, local_name_len);
    if (ret != EOK) {
        sample_at_log_print("%s memcpy fail\r\n", SLE_UUID_SERVER_LOG);
        return 0;
    }
    return (uint16_t)index + local_name_len;
}

static uint16_t sle_set_adv_data(uint8_t *adv_data)
{
    size_t len = 0;
    uint16_t idx = 0;
    errno_t  ret = 0;

    len = sizeof(struct sle_adv_common_value);
    struct sle_adv_common_value adv_disc_level = {
        .length = len - 1,
        .type = SLE_ADV_DATA_TYPE_DISCOVERY_LEVEL,
        .value = SLE_ANNOUNCE_LEVEL_NORMAL,
    };
    ret = memcpy_s(&adv_data[idx], SLE_ADV_DATA_LEN_MAX - idx, &adv_disc_level, len);
    if (ret != EOK) {
        sample_at_log_print("%s adv_disc_level memcpy fail\r\n", SLE_UUID_SERVER_LOG);
        return 0;
    }
    idx += len;

    len = sizeof(struct sle_adv_common_value);
    struct sle_adv_common_value adv_access_mode = {
        .length = len - 1,
        .type = SLE_ADV_DATA_TYPE_ACCESS_MODE,
        .value = 0,
    };
    ret = memcpy_s(&adv_data[idx], SLE_ADV_DATA_LEN_MAX - idx, &adv_access_mode, len);
    if (ret != EOK) {
        sample_at_log_print("%s adv_access_mode memcpy fail\r\n", SLE_UUID_SERVER_LOG);
        return 0;
    }
    idx += len;

    return idx;
}

static uint16_t sle_set_scan_response_data(uint8_t *scan_rsp_data)
{
    uint16_t idx = 0;
    size_t scan_rsp_data_len = sizeof(struct sle_adv_common_value);

    struct sle_adv_common_value tx_power_level = {
        .length = scan_rsp_data_len - 1,
        .type = SLE_ADV_DATA_TYPE_TX_POWER_LEVEL,
        .value = SLE_ADV_TX_POWER,
    };
    errno_t ret = memcpy_s(scan_rsp_data, SLE_ADV_DATA_LEN_MAX, &tx_power_level, scan_rsp_data_len);
    if (ret != EOK) {
        sample_at_log_print("%s sle scan response data memcpy fail\r\n", SLE_UUID_SERVER_LOG);
        return 0;
    }
    idx += scan_rsp_data_len;

    /* set local name */
    idx += sle_set_adv_local_name(&scan_rsp_data[idx], SLE_ADV_DATA_LEN_MAX - idx);
    return idx;
}

static errcode_t sle_set_default_announce_param(void)
{
    sle_announce_param_t param = {0};
    unsigned char local_addr[SLE_ADDR_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
    param.announce_mode = SLE_ANNOUNCE_MODE_CONNECTABLE_SCANABLE;
    param.announce_handle = SLE_ADV_HANDLE_DEFAULT;
    param.announce_gt_role = SLE_ANNOUNCE_ROLE_T_CAN_NEGO;
    param.announce_level = SLE_ANNOUNCE_LEVEL_NORMAL;
    param.announce_channel_map = SLE_ADV_CHANNEL_MAP_DEFAULT;
    param.announce_interval_min = SLE_ADV_INTERVAL_MIN_DEFAULT;
    param.announce_interval_max = SLE_ADV_INTERVAL_MAX_DEFAULT;
    param.conn_interval_min = SLE_CONN_INTV_MIN_DEFAULT;
    param.conn_interval_max = SLE_CONN_INTV_MAX_DEFAULT;
    param.conn_max_latency = SLE_CONN_MAX_LATENCY;
    param.conn_supervision_timeout = SLE_CONN_SUPERVISION_TIMEOUT_DEFAULT;
    param.own_addr.type = SLE_ADDRESS_TYPE_PUBLIC;
    errcode_t ret = memcpy_s(param.own_addr.addr, SLE_ADDR_LEN, local_addr, SLE_ADDR_LEN);
    if (ret != EOK) {
        sample_at_log_print("%s sle_set_default_announce_param data memcpy fail\r\n", SLE_UUID_SERVER_LOG);
        return ret;
    }
    sample_at_log_print("%s sle_uuid_local addr: ", SLE_UUID_SERVER_LOG);
    for (uint8_t index = 0; index < SLE_ADDR_LEN; index++) {
        sample_at_log_print("0x%02x ", param.own_addr.addr[index]);
    }
    sample_at_log_print("\r\n");
    return sle_set_announce_param(param.announce_handle, &param);
}

static errcode_t sle_set_default_announce_data(void)
{
    errcode_t ret;
    uint8_t announce_data_len = 0;
    uint8_t seek_data_len = 0;
    sle_announce_data_t data = {0};
    uint8_t adv_handle = SLE_ADV_HANDLE_DEFAULT;
    uint8_t announce_data[SLE_ADV_DATA_LEN_MAX] = {0};
    uint8_t seek_rsp_data[SLE_ADV_DATA_LEN_MAX] = {0};
    uint8_t data_index = 0;

    announce_data_len = sle_set_adv_data(announce_data);
    data.announce_data = announce_data;
    data.announce_data_len = announce_data_len;

    sample_at_log_print("%s data.announce_data_len = %d\r\n", SLE_UUID_SERVER_LOG, data.announce_data_len);
    sample_at_log_print("%s data.announce_data: ", SLE_UUID_SERVER_LOG);
    for (data_index = 0; data_index<data.announce_data_len; data_index++) {
        sample_at_log_print("0x%02x ", data.announce_data[data_index]);
    }
    sample_at_log_print("\r\n");

    seek_data_len = sle_set_scan_response_data(seek_rsp_data);
    data.seek_rsp_data = seek_rsp_data;
    data.seek_rsp_data_len = seek_data_len;

    sample_at_log_print("%s data.seek_rsp_data_len = %d\r\n", SLE_UUID_SERVER_LOG, data.seek_rsp_data_len);
    sample_at_log_print("%s data.seek_rsp_data: ", SLE_UUID_SERVER_LOG);
    for (data_index = 0; data_index<data.seek_rsp_data_len; data_index++) {
        sample_at_log_print("0x%02x ", data.seek_rsp_data[data_index]);
    }
    sample_at_log_print("\r\n");

    ret = sle_set_announce_data(adv_handle, &data);
    if (ret == ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s set announce data success.\r\n", SLE_UUID_SERVER_LOG);
    } else {
        sample_at_log_print("%s set adv param fail.\r\n", SLE_UUID_SERVER_LOG);
    }
    return ERRCODE_SLE_SUCCESS;
}

static void sle_announce_enable_cbk(uint32_t announce_id, errcode_t status)
{
    sample_at_log_print("%s sle announce enable callback id:%02x, state:%x\r\n", SLE_UUID_SERVER_LOG, announce_id,
        status);
}

static void sle_announce_disable_cbk(uint32_t announce_id, errcode_t status)
{
    sample_at_log_print("%s sle announce disable callback id:%02x, state:%x\r\n", SLE_UUID_SERVER_LOG, announce_id,
        status);
}

static void sle_announce_terminal_cbk(uint32_t announce_id)
{
    sample_at_log_print("%s sle announce terminal callback id:%02x\r\n", SLE_UUID_SERVER_LOG, announce_id);
}

static void sle_enable_cbk(errcode_t status)
{
    sample_at_log_print("%s sle enable callback status:%x\r\n", SLE_UUID_SERVER_LOG, status);
    sle_enable_server_cbk();
}

errcode_t sle_uuid_announce_register_cbks(void)
{
    errcode_t ret = ERRCODE_SLE_SUCCESS;
    sle_announce_seek_callbacks_t seek_cbks = {0};
    seek_cbks.announce_enable_cb = sle_announce_enable_cbk;
    seek_cbks.announce_disable_cb = sle_announce_disable_cbk;
    seek_cbks.announce_terminal_cb = sle_announce_terminal_cbk;
    seek_cbks.sle_enable_cb = sle_enable_cbk;
    ret = sle_announce_seek_register_callbacks(&seek_cbks);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_uuid_announce_register_cbks,register_callbacks fail :%x\r\n",
        SLE_UUID_SERVER_LOG, ret);
        return ret;
    }
    return ERRCODE_SLE_SUCCESS;
}

errcode_t sle_uuid_server_adv_init(void)
{
    sample_at_log_print("sle uuid server adv init.\r\n");
    errcode_t ret = ERRCODE_SLE_SUCCESS;
    sle_set_default_announce_param();
    sle_set_default_announce_data();
    ret = sle_start_announce(SLE_ADV_HANDLE_DEFAULT);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_uuid_server_adv_init,sle_start_announce fail :%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ret;
    }
    return ERRCODE_SLE_SUCCESS;
}

errcode_t sle_uuid_server_adv_deinit(void)
{
    sample_at_log_print("sle uuid server adv deinit.\r\n");
    errcode_t ret = sle_stop_announce(SLE_ADV_HANDLE_DEFAULT);
    if (ret != ERRCODE_SLE_SUCCESS) {
        sample_at_log_print("%s sle_uuid_server_adv_init,sle_start_announce fail :%x\r\n", SLE_UUID_SERVER_LOG, ret);
        return ret;
    }
    return ERRCODE_SLE_SUCCESS;
}
