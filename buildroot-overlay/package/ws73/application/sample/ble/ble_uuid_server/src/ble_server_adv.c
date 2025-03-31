/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd.. 2023. All rights reserved.
 * Description: adv config for ble uuid server.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "securec.h"
#include "errcode.h"
#include "bts_def.h"
#include "bts_le_gap.h"
#include "ble_server_adv.h"

#define  NAME_MAX_LENGTH 15
#define EXT_ADV_OR_SCAN_RSP_DATA_LEN 251
#define u16_low_u8(val)  ((uint8_t)((uint16_t)(val) & 0xff))
#define u16_high_u8(val) ((uint8_t)(((uint16_t)(val) >> 8) & 0xff))

uint8_t g_local_name[ NAME_MAX_LENGTH] = { 'b', 'l', 'e', '_', 'u', 'u', 'i', 'd', '_', 's',
    'e', 'r', 'v', 'e', 'r' };

static uint8_t ble_set_adv_flag_data(uint8_t *set_adv_data_position, uint8_t max_len)
{
    errno_t n_ret;

    ble_adv_flag adv_flags = {
        .length = BLE_ADV_FLAG_LEN - BLE_GENERAL_BYTE_1,
        .adv_data_type = 1,
        .flags = BLE_ADV_FLAG_DATA,
    };
    n_ret = memcpy_s(set_adv_data_position, max_len, &adv_flags, BLE_ADV_FLAG_LEN);
    if (n_ret != EOK) {
        return 0;
    }
    return BLE_ADV_FLAG_LEN;
}

static uint8_t ble_set_adv_appearance(uint8_t *set_adv_data_position, uint8_t max_len)
{
    errno_t n_ret;
    ble_appearance_st adv_appearance_data = {
        .length = BLE_ADV_APPEARANCE_LENGTH - BLE_GENERAL_BYTE_1,
        .adv_data_type = BLE_ADV_APPEARANCE_DATA_TYPE,
        .catogory_id = { u16_low_u8(BLE_ADV_CATEGORY_KEYBOARD_VALUE), u16_high_u8(BLE_ADV_CATEGORY_KEYBOARD_VALUE) },
    };
    n_ret = memcpy_s(set_adv_data_position, max_len, &adv_appearance_data, BLE_ADV_APPEARANCE_LENGTH);
    if (n_ret != EOK) {
        return 0;
    }
    return BLE_ADV_APPEARANCE_LENGTH;
}

static uint8_t ble_set_adv_name(uint8_t *set_adv_data_position, uint8_t max_len)
{
    errno_t n_ret;
    uint8_t len;
    ble_local_name_st adv_local_name_data = { 0 };
    adv_local_name_data.length = (uint8_t)(BLE_ADV_PARAM_DATATYPE_LENGTH + sizeof(g_local_name));
    adv_local_name_data.adv_data_type = BLE_ADV_LOCAL_NAME_DATA_TYPE;
    len = BLE_ADV_PARAM_DATATYPE_LENGTH + BLE_ADV_PARAM_DATATYPE_LENGTH;
    n_ret = memcpy_s(set_adv_data_position, max_len, &adv_local_name_data, len);
    if (n_ret != EOK) {
        return 0;
    }
    n_ret = memcpy_s((set_adv_data_position + len), (size_t)(max_len - len), g_local_name, sizeof(g_local_name));
    if (n_ret != EOK) {
        return 0;
    }
    len = (uint8_t)(len + sizeof(g_local_name));
    return len;
}

static uint8_t ble_set_adv_appearance_data(uint8_t *set_adv_data_position, uint8_t max_len)
{
    uint8_t idx = 0;
    idx += ble_set_adv_appearance(set_adv_data_position, max_len);
    idx += ble_set_adv_name(set_adv_data_position + idx, (max_len - idx));
    return idx;
}

static uint16_t ble_uuid_server_set_adv_data(uint8_t *set_adv_data, uint8_t adv_data_max_len)
{
    uint8_t idx = 0;

    if ((set_adv_data == NULL) || (adv_data_max_len == 0)) {
        return 0;
    }
    idx += ble_set_adv_flag_data(set_adv_data, adv_data_max_len);
    idx += ble_set_adv_appearance_data(&set_adv_data[idx], adv_data_max_len - idx);
    return idx;
}

static uint16_t ble_set_scan_response_data(uint8_t *scan_rsp_data, uint8_t scan_rsp_data_max_len)
{
    uint8_t idx = 0;
    errno_t n_ret;

    if (scan_rsp_data == NULL) {
        return 0;
    }
    if (scan_rsp_data_max_len == 0) {
        return 0;
    }

    /* tx power level */
    ble_tx_power_level_st tx_power_level = {
        .length = BLE_SCAN_RSP_TX_POWER_LEVEL_LEN - BLE_GENERAL_BYTE_1,
        .adv_data_type = BLE_ADV_TX_POWER_LEVEL,
        .tx_power_value = 0,
    };

    n_ret = memcpy_s(scan_rsp_data, scan_rsp_data_max_len, &tx_power_level, sizeof(ble_tx_power_level_st));
    if (n_ret != EOK) {
        return 0;
    }
    idx += BLE_SCAN_RSP_TX_POWER_LEVEL_LEN;

    /* set local name */
    scan_rsp_data[idx++] = sizeof(g_local_name) + BLE_GENERAL_BYTE_1;
    scan_rsp_data[idx++] = BLE_ADV_LOCAL_NAME_DATA_TYPE;
    if ((idx + sizeof(g_local_name)) > scan_rsp_data_max_len) {
        return 0;
    }
    n_ret = memcpy_s(&scan_rsp_data[idx], scan_rsp_data_max_len - idx, g_local_name, sizeof(g_local_name));
    if (n_ret != EOK) {
        return 0;
    }
    idx += sizeof(g_local_name);
    return idx;
}

uint8_t ble_set_adv_data(void)
{
    errcode_t n_ret = 0;
    uint16_t adv_data_len;
    uint16_t scan_rsp_data_len;
    uint8_t set_adv_data[EXT_ADV_OR_SCAN_RSP_DATA_LEN] = { 0 };
    uint8_t set_scan_rsp_data[EXT_ADV_OR_SCAN_RSP_DATA_LEN] = { 0 };
    gap_ble_config_adv_data_t cfg_adv_data = {0};

    /* set adv data */
    adv_data_len = ble_uuid_server_set_adv_data(set_adv_data, EXT_ADV_OR_SCAN_RSP_DATA_LEN);
    if ((adv_data_len > EXT_ADV_OR_SCAN_RSP_DATA_LEN) || (adv_data_len == 0)) {
        return 0;
    }
    /* set scan response data */
    scan_rsp_data_len = ble_set_scan_response_data(set_scan_rsp_data, EXT_ADV_OR_SCAN_RSP_DATA_LEN);
    if ((scan_rsp_data_len > EXT_ADV_OR_SCAN_RSP_DATA_LEN) || (scan_rsp_data_len == 0)) {
        return 0;
    }
    cfg_adv_data.adv_data = set_adv_data;
    cfg_adv_data.adv_length = adv_data_len;

    cfg_adv_data.scan_rsp_data = set_scan_rsp_data;
    cfg_adv_data.scan_rsp_length = scan_rsp_data_len;
    n_ret =  gap_ble_set_adv_data(BTH_GAP_BLE_ADV_HANDLE_DEFAULT, &cfg_adv_data);
    if (n_ret != 0) {
        return 0;
    }
    return 0;
}

uint8_t ble_start_adv(void)
{
    errcode_t n_ret = 0;
    gap_ble_adv_params_t adv_para = {0};
    int adv_id = BTH_GAP_BLE_ADV_HANDLE_DEFAULT;

    static uint8_t addr[BD_ADDR_LEN] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    memcpy_s(adv_para.own_addr.addr, BD_ADDR_LEN, addr, BD_ADDR_LEN);
    adv_para.own_addr.type = BT_ADDRESS_TYPE_PUBLIC_DEVICE_ADDRESS;

    adv_para.min_interval = BLE_ADV_MIN_INTERVAL;
    adv_para.max_interval = BLE_ADV_MAX_INTERVAL;
    adv_para.duration = BTH_GAP_BLE_ADV_FOREVER_DURATION;
    adv_para.peer_addr.type = BLE_PUBLIC_DEVICE_ADDRESS;
    /* 广播通道选择bitMap, 可参考BleAdvChannelMap */
    adv_para.channel_map = BLE_ADV_CHANNEL_MAP_CH_DEFAULT;
    adv_para.adv_type = BLE_ADV_TYPE_CONNECTABLE_UNDIRECTED;
    adv_para.adv_filter_policy = BLE_ADV_FILTER_POLICY_SCAN_ANY_CONNECT_ANY;
    (void)memset_s(&adv_para.peer_addr.addr, BD_ADDR_LEN, 0, BD_ADDR_LEN);
    n_ret = gap_ble_set_adv_param(adv_id, &adv_para);
    if (n_ret != 0) {
        return 0;
    }
    n_ret = gap_ble_start_adv(adv_id);
    if (n_ret != 0) {
        return 0;
    }
    return 0;
}

errcode_t ble_stop_adv(void)
{
    return gap_ble_stop_adv(BTH_GAP_BLE_ADV_HANDLE_DEFAULT);
}