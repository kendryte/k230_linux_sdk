/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: host coex msg adapt source file
 * Author: BGTP
 * Create: 2023-08-03
 */
#include "coex_msg_adapt.h"

#ifdef _PRE_BSLE_GATEWAY

#include "plat_hcc_srv.h"
#include "plat_hcc_msg_type.h"
#include "oal_debug.h"

#define COEX_GATEWAY_RX_BUFFER_SIZE 4
#define COEX_GATEWAY_RX_BUFFER_BLOCK 8

// wifi通知蓝牙的状态，plat_ko注册回调
typedef void (*btcoex_wifi2ble_msg_callback)(void *event);

typedef struct {
    btcoex_wifi2ble_msg_callback btcoex_wifi2ble_func;
} btcoex_wifi2ble_cfg_stru;

coex_bsle_status_tx_gateway_t g_coex_msg_tx_gw = { 0 };
coex_wl_status_rx_gateway_t g_coex_msg_rx_dev = { 0 };
btcoex_wifi2ble_cfg_stru g_wifi2ble_cbk;
td_u32 bt_coex_msg_host_send_to_bsle_device(td_u8 *buffer, td_u16 length);
td_void coex_plat_bsle_status_pack_and_send(coex_bsle_status_tx_gateway_t *change_msg);

extern void hmac_btcoex_ble2wifi_msg_callback(void *event);                               // 通知wifi
extern void hmac_btcoex_set_wifi2ble_cfg(btcoex_wifi2ble_cfg_stru *wifi_cfg, td_u32 len); // wifi回调

// gateway--->plat--->device
td_void coex_gateway_wifi_status_send(coex_wl_status_rx_gateway_t *send_dev_msg)
{
    td_u32 tmp_send_msg = send_dev_msg->wifi_status_stru;
    td_u8 buffer[COEX_GATEWAY_RX_BUFFER_SIZE] = {0};
    td_u8 i;
    for (i = 0; i < COEX_GATEWAY_RX_BUFFER_SIZE; i++) {
        buffer[i] = (td_u8)(tmp_send_msg >> (i * COEX_GATEWAY_RX_BUFFER_BLOCK)) & 0xFF;
    }
    bt_coex_msg_host_send_to_bsle_device(buffer, sizeof(td_u32));
}

td_u32 coex_gateway_wifi_status_unpack(mac_bslecoex_event_stru *remote_msg)
{
    switch (remote_msg->coex_msg.msg_type) {
        case BTCOEX_WIFI_MSG_BW_CHANNEL:
            g_coex_msg_rx_dev.wlan_channel_num = remote_msg->wifi_freq_msg.channel;
            g_coex_msg_rx_dev.wlan_bandwidth = remote_msg->wifi_freq_msg.bw;
            break;
        case BTCOEX_WIFI_MSG_STATUS:
            g_coex_msg_rx_dev.wlan_on = remote_msg->wifi_status_msg.open_status;
            g_coex_msg_rx_dev.wlan_connect = remote_msg->wifi_status_msg.connect_status;
            g_coex_msg_rx_dev.wlan_power_status = remote_msg->wifi_status_msg.power_status;
            break;
        case BTCOEX_WIFI_MSG_SLOT:
            g_coex_msg_rx_dev.wlan_slot_num = remote_msg->wifi_slot_msg.slot_num;
            break;
        case BTCOEX_WIFI_MSG_RSSI:
            g_coex_msg_rx_dev.wlan_rssi_num = remote_msg->wifi_rssi_msg.rssi_num;
            break;
        default:
            return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

td_void btcoex_msg_recieve_from_gateway(td_void *event)
{
    mac_bslecoex_event_stru remote_msg = { 0 };
    td_bool wlan_on = (td_bool)g_coex_msg_rx_dev.wlan_on; // 存一下WiFi on状态

    remote_msg = *(mac_bslecoex_event_stru *)event;
    // 先存一下上一次WiFion的状态,如果WIFI on 从 0->1 把bgle的状态全量通知给WiFi
    coex_gateway_wifi_status_unpack(&remote_msg); // WIFI 的状态解包到g_coex_msg_rx_dev中

    if ((wlan_on ^ (td_bool)g_coex_msg_rx_dev.wlan_on) && (td_bool)g_coex_msg_rx_dev.wlan_on == true) {
        coex_bsle_status_tx_gateway_t bsle_state_change = { 0 };
        bsle_state_change.bsle_status_stru = 0xFFFFFFFF; // 变化位全置1, 全量通知
        coex_plat_bsle_status_pack_and_send(&bsle_state_change);
    }
    coex_gateway_wifi_status_send(&g_coex_msg_rx_dev);
}

// device--->plat--->gateway
td_u32 coex_plat_bsle_status_save(uint16_t msg_type, mac_bslecoex_event_stru *src_msg)
{
    switch (msg_type) {
        case BTCOEX_BLE_MSG_SCAN:
            src_msg->ble_scan_msg.ble_scan_status = g_coex_msg_tx_gw.bsle_scan;
            src_msg->ble_scan_msg.ble_self_cts_duration = g_coex_msg_tx_gw.bsle_scan_cts_dur;
            src_msg->ble_scan_msg.msg_type = BTCOEX_BLE_MSG_SCAN;
            break;
        case BTCOEX_BLE_MSG_STATUS:
            src_msg->ble_status_msg.ble_open_status = g_coex_msg_tx_gw.bsle_on;
            src_msg->ble_status_msg.ble_connect_status = g_coex_msg_tx_gw.bsle_conn;
            src_msg->ble_status_msg.ble_thrans = g_coex_msg_tx_gw.bsle_trans;
            src_msg->ble_status_msg.msg_type = BTCOEX_BLE_MSG_STATUS;
            break;
        case BTCOEX_BLE_MSG_BROADCAST:
            src_msg->ble_broadcast_msg.ble_broadcast_status = g_coex_msg_tx_gw.bsle_adv;
            src_msg->ble_broadcast_msg.ble_self_cts_duration = g_coex_msg_tx_gw.bsle_adv_cts_dur;
            src_msg->ble_broadcast_msg.msg_type = BTCOEX_BLE_MSG_BROADCAST;
            break;
        case BTCOEX_BLE_MSG_DEVICE_NUM:
            src_msg->ble_device_num_msg.ble_device_num = g_coex_msg_tx_gw.bsle_conn_num;
            src_msg->ble_device_num_msg.msg_type = BTCOEX_BLE_MSG_DEVICE_NUM;
            break;
        case BTCOEX_BLE_MSG_CTS:
            src_msg->ble_self_cts_duration_msg.ble_self_cts_duration = 0; // todo
            src_msg->ble_self_cts_duration_msg.msg_type = BTCOEX_BLE_MSG_CTS;
            break;
        case BTCOEX_BLE_MSG_AGGR_LEVEL:
            src_msg->ble_wifi_aggr_msg.ble_wifi_aggr_level = g_coex_msg_tx_gw.bsle_delba_level;
            src_msg->ble_self_cts_duration_msg.msg_type = BTCOEX_BLE_MSG_AGGR_LEVEL;
            break;
        default:
            return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

td_void coex_plat_bsle_status_pack_and_send(coex_bsle_status_tx_gateway_t *change_msg)
{
    mac_bslecoex_event_stru src_msg = { 0 };
    if (change_msg->bsle_on != 0 || change_msg->bsle_conn != 0 || change_msg->bsle_trans != 0) {
        coex_plat_bsle_status_save(BTCOEX_BLE_MSG_STATUS, &src_msg);
        hmac_btcoex_ble2wifi_msg_callback((void *)&src_msg);
    }
    if (change_msg->bsle_scan != 0 || change_msg->bsle_scan_cts_dur != 0) {
        coex_plat_bsle_status_save(BTCOEX_BLE_MSG_SCAN, &src_msg);
        hmac_btcoex_ble2wifi_msg_callback((void *)&src_msg);
    }
    if (change_msg->bsle_adv != 0 || change_msg->bsle_adv_cts_dur != 0) {
        coex_plat_bsle_status_save(BTCOEX_BLE_MSG_BROADCAST, &src_msg);
        hmac_btcoex_ble2wifi_msg_callback((void *)&src_msg);
    }
    if (change_msg->bsle_conn_num != 0) {
        coex_plat_bsle_status_save(BTCOEX_BLE_MSG_DEVICE_NUM, &src_msg);
        hmac_btcoex_ble2wifi_msg_callback((void *)&src_msg);
    }
    if (change_msg->bsle_delba_level != 0) {
        coex_plat_bsle_status_save(BTCOEX_BLE_MSG_AGGR_LEVEL, &src_msg);
        hmac_btcoex_ble2wifi_msg_callback((void *)&src_msg);
    }

}

/* 从device接收到的msg保存在全局变量中, 维护一个状态变化的就消息, 发给大网关时状态变化再发 */
td_u32 bt_c2h_coex_msg_device_send_to_plat_host(td_u8 *buffer, td_u16 length)
{
    td_u32 coex_msg_dev = ((uint32_t)buffer[0] | (((uint32_t)buffer[1] << 8) & 0xFF00) |
        (((uint32_t)buffer[2] << 16) & 0xFF0000) | (((uint32_t)buffer[3] << 24) & 0xFF000000));

    coex_bsle_status_tx_gateway_t bsle_state_change = { 0 };
    coex_bsle_status_tx_gateway_t bsle_state_now = g_coex_msg_tx_gw;
    g_coex_msg_tx_gw.bsle_status_stru = coex_msg_dev;
    // 找到wl状态变化点,记录业务变化的BIT位
    bsle_state_change.bsle_status_stru = bsle_state_now.bsle_status_stru ^ g_coex_msg_tx_gw.bsle_status_stru;
    // 根据消息结构的变化位, 发送对应的消息
    coex_plat_bsle_status_pack_and_send(&bsle_state_change);
    return EXT_ERR_SUCCESS;
}

td_u32 bt_coex_msg_host_send_to_bsle_device(td_u8 *buffer, td_u16 length)
{
    td_u32 ret = plat_msg_hcc_send(buffer, sizeof(td_u32), H2D_PLAT_CFG_MSG_COEX_GATEWAY_TO_BSLE);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("bt_coex_msg_send_to_plat_host fail\n");
    }
    return ret;
}

td_void coex_bsle_hcc_msg_plat_host_to_device_init(td_void)
{
    hcc_plat_msg_register(D2H_PLAT_CFG_MSG_COEX_BSLE_TO_GATEWAY,
        (hcc_plat_msg_callbck)bt_c2h_coex_msg_device_send_to_plat_host);
    g_wifi2ble_cbk.btcoex_wifi2ble_func = btcoex_msg_recieve_from_gateway;
    hmac_btcoex_set_wifi2ble_cfg(&g_wifi2ble_cbk, sizeof(btcoex_wifi2ble_cfg_stru));
}
#endif
