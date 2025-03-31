/*
 * Copyright (c) @CompanyNameMagicTag 2024. All rights reserved.
 * Description: ble uuid server sample.
 */
 
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bts_def.h"
#include "securec.h"
#include "errcode.h"
#include "bts_gatt_stru.h"
#include "bts_gatt_client.h"
#include "bts_le_gap.h"
#include "bts_device_manager.h"
#include "ble_client_scan.h"
#include "bts_le_gap.h"
#include "soc_osal.h"
#include "ble_gatt_client.h"

#define UUID16_LEN 2
#define BLE_SLEEP_US 10000
#if SPEED_TEST_SEND
#define sample_at_log_print(fmt, args...)
#else
#define sample_at_log_print(fmt, args...) printf(fmt, ##args)
#endif

/* client id, invalid client id is "0" */
static uint8_t g_client_id = 0;
/* client app uuid for test */
static bt_uuid_t g_client_app_uuid = {UUID16_LEN, {0}};
/* ble enable status */
static uint8_t g_ble_enable_status = 0;
#if SUPPORT_GATT_C
/* ble connect id */
static uint16_t g_connect_id = 0;
/* ble mtu */
static uint16_t g_mtu = 517;
/* ble connect count */
static uint16_t g_connect_cnt = 0;
/* ble ssapc tx handle */
static uint16_t g_tx_handle = -1;
#endif

#if SPEED_TEST_SEND
static pthread_t g_send_thread = -1;
static sle_send_data_state_t g_send_flag = SEND_STATE_IDLE;
#endif

static void ble_gatt_client_print_hex(const uint8_t *data, uint16_t len, char *flag)
{
    sample_at_log_print("%s:", flag);
    for (uint8_t i = 0; i < len; i++) {
        sample_at_log_print("0x%02x ", data[i]);
    }
    sample_at_log_print("\n");
}

#if SUPPORT_GATT_C
static void ble_gatt_client_connect_device(gap_scan_result_data_t *scan_result_data)
{
    gap_ble_stop_scan();

    bd_addr_t client_addr = {0};
    client_addr.type = scan_result_data->addr.type;
    if (memcpy_s(client_addr.addr, BD_ADDR_LEN, scan_result_data->addr.addr, BD_ADDR_LEN) != EOK) {
        sample_at_log_print("%s add server app addr memcpy failed\r\n");
        return;
    }
    gap_ble_connect_remote_device(&client_addr);
}

static errcode_t ble_gatt_client_send_report(uint16_t handle, uint8_t *data, uint16_t len)
{
    gattc_handle_value_t param = {0};
    param.handle = handle;
    param.data = data;
    param.data_len = len;

    errcode_t ret = gattc_write_cmd(g_client_id, g_connect_id, &param);
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("gattc_write_cmd failed, errcode: %d\r\n", ret);
    }
    return ret;
}

#if SPEED_TEST_SEND
static unsigned long ble_gatt_client_m_time_to_usec(unsigned long tv_sec, unsigned long tv_usec)
{
    return (tv_sec * THOUSAND * THOUSAND + tv_usec);
}

static void ble_gatt_client_ms_delay(int msec)
{
    osal_timeval pre_time = { 0 };
    osal_timeval cur_time = { 0 };
    osal_gettimeofday(&pre_time);
    unsigned long pre_us = ble_gatt_client_m_time_to_usec(pre_time.tv_sec, pre_time.tv_usec);
    while (1) {
        osal_gettimeofday(&cur_time);
        if (ble_gatt_client_m_time_to_usec(cur_time.tv_sec, cur_time.tv_usec) - pre_us > msec * THOUSAND) {
            break;
        }
    }
    return;
}

static void ble_gatt_client_init_data(uint8_t *data)
{
    sample_at_log_print("[gatt client] init data\r\n");
    for (int i = 0; i <= PACKAGE_SIZE; i++) {
        data[i] = i; // 这里仅为示例，将所有元素初始化为它们的索引值
    }
}

static void ble_gatt_client_send_data(uint8_t *data, uint16_t data_len)
{
    ble_gatt_client_ms_delay(1);
    ble_gatt_client_send_report(g_tx_handle, data, data_len);
}

static void *ble_gatt_client_send_thread(uint8_t *arg)
{
    uint8_t data[PACKAGE_SIZE] = {0};
    ble_gatt_client_init_data(data);
    while (1) {
        // 0还未初始化完成，等待
        if (g_send_flag == SEND_STATE_IDLE) {
            usleep(MS_100);
            continue;
        }
        // 2退出
        if (g_send_flag == SEND_STATE_EXIT) {
            break;
        }
        ble_gatt_client_send_data(data, PACKAGE_SIZE);
    }
    return NULL;
}

int ble_gatt_client_send_thread_init()
{
    g_send_flag = SEND_STATE_IDLE;
    int ret = pthread_create(&g_send_thread, NULL, ble_gatt_client_send_thread, NULL);
    if (ret < 0) {
        sample_at_log_print("Err: fail to create write thread.\n");
        return -1;
    }
    sample_at_log_print("ble send data thread init succ.\n");
    usleep(MS_100);
    return 0;
}

void ble_gatt_client_send_thread_deinit()
{
    g_send_flag = SEND_STATE_EXIT;
    pthread_join(g_send_thread, NULL);
}

void ble_gatt_client_send_thread_reset()
{
    g_send_flag = SEND_STATE_IDLE;
}
#endif

// ---- gattc cbk begin ----
static void ble_gatt_client_discover_service_cbk(uint8_t client_id, uint16_t conn_id,
    gattc_discovery_service_result_t *service, errcode_t status)
{
    gattc_discovery_character_param_t param = {0};
    sample_at_log_print("[gatt client]Discovery service----client:%d conn_id:%d\n", client_id, conn_id);
    sample_at_log_print("            start handle:%d end handle:%d uuid_len:%d\n            uuid:",
        service->start_hdl, service->end_hdl, service->uuid.uuid_len);
    for (uint8_t i = 0; i < service->uuid.uuid_len; i++) {
        sample_at_log_print("%02x", service->uuid.uuid[i]);
    }
    sample_at_log_print("\n            status:%d\n", status);
    param.service_handle = service->start_hdl;
    param.uuid.uuid_len = 0; /* uuid length is zero, discover all character */
    gattc_discovery_character(g_client_id, conn_id, &param);
}

static void ble_gatt_client_discover_character_cbk(uint8_t client_id, uint16_t conn_id,
    gattc_discovery_character_result_t *character, errcode_t status)
{
    sample_at_log_print("[gatt client]Discovery character----client:%d conn_id:%d uuid_len:%d\n            uuid:",
        client_id, conn_id, character->uuid.uuid_len);
    for (uint8_t i = 0; i < character->uuid.uuid_len; i++) {
        sample_at_log_print("%02x", character->uuid.uuid[i]);
    }
    sample_at_log_print("\n            declare handle:%d value handle:%d properties:%x\n", character->declare_handle,
        character->value_handle, character->properties);
    sample_at_log_print("            status:%d\n", status);

    // assign character->value_handle to g_tx_handle for sending data to server
#if SPEED_TEST_SEND
    static uint8_t rx_uuids[2] = {0x2A, 0x8A};
    if (memcmp(character->uuid.uuid, rx_uuids, character->uuid.uuid_len) == 0) {
        g_tx_handle = character->value_handle;
    }
#endif

    gattc_discovery_descriptor(g_client_id, conn_id, character->declare_handle);
}

static void ble_gatt_client_discover_descriptor_cbk(uint8_t client_id, uint16_t conn_id,
    gattc_discovery_descriptor_result_t *descriptor, errcode_t status)
{
    sample_at_log_print("[gatt client]Discovery descriptor----client:%d conn_id:%d uuid len:%d\n            uuid:",
        client_id, conn_id, descriptor->uuid.uuid_len);
    for (uint8_t i = 0; i < descriptor->uuid.uuid_len; i++) {
        sample_at_log_print("%02x", descriptor->uuid.uuid[i]);
    }
    sample_at_log_print("\n            descriptor handle:%d\n", descriptor->descriptor_hdl);
    sample_at_log_print("            status:%d\n", status);
}

static void ble_gatt_client_discover_service_compl_cbk(uint8_t client_id, uint16_t conn_id, bt_uuid_t *uuid,
    errcode_t status)
{
    sample_at_log_print(
        "[gatt client]Discovery service complete----client:%d conn_id:%d uuid len:%d\n            uuid:",
        client_id, conn_id, uuid->uuid_len);
    for (uint8_t i = 0; i < uuid->uuid_len; i++) {
        sample_at_log_print("%02x", uuid->uuid[i]);
    }
    sample_at_log_print("            status:%d\n", status);
}

static void ble_gatt_client_discover_character_compl_cbk(uint8_t client_id, uint16_t conn_id,
    gattc_discovery_character_param_t *param, errcode_t status)
{
    sample_at_log_print(
        "[gatt client]Discovery character complete----client:%d conn_id:%d uuid len:%d\n            uuid:",
        client_id, conn_id, param->uuid.uuid_len);
    for (uint8_t i = 0; i < param->uuid.uuid_len; i++) {
        sample_at_log_print("%02x", param->uuid.uuid[i]);
    }
    sample_at_log_print("\n            service handle:%d\n", param->service_handle);
    sample_at_log_print("            status:%d\n", status);

    if (g_tx_handle != -1) {
        // using ble_gatt_client_send_report(g_tx_handle, data, data_len) automatically
        // if wanting to send data to server.
#if SPEED_TEST_SEND
        g_send_flag = SEND_STATE_SENDING;
#endif
    }
}

static void ble_gatt_client_discover_descriptor_compl_cbk(uint8_t client_id, uint16_t conn_id,
    uint16_t character_handle, errcode_t status)
{
    sample_at_log_print("[gatt client]Discovery descriptor complete----client:%d conn_id:%d\n", client_id, conn_id);
    sample_at_log_print("            charatcer handle:%d\n", character_handle);
    sample_at_log_print("            status:%d\n", status);
}

static void ble_gatt_client_read_cfm_cbk(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *read_result,
    gatt_status_t status)
{
    sample_at_log_print("[gatt client]Read result----client:%d conn_id:%d\n", client_id, conn_id);
    sample_at_log_print("            handle:%d data_len:%d\ndata:", read_result->handle, read_result->data_len);
    for (uint8_t i = 0; i < read_result->data_len; i++) {
        sample_at_log_print("%02x", read_result->data[i]);
    }
    sample_at_log_print("\n            status:%d\n", status);
}

static void ble_gatt_client_read_compl_cbk(uint8_t client_id, uint16_t conn_id, gattc_read_req_by_uuid_param_t *param,
    errcode_t status)
{
    sample_at_log_print("[gatt client]Read by uuid complete----client:%d conn_id:%d\n", client_id, conn_id);
    sample_at_log_print("start handle:%d end handle:%d uuid len:%d\n            uuid:",
        param->start_hdl, param->end_hdl, param->uuid.uuid_len);
    for (uint8_t i = 0; i < param->uuid.uuid_len; i++) {
        sample_at_log_print("%02x", param->uuid.uuid[i]);
    }
    sample_at_log_print("\n            status:%d\n", status);
}

static void ble_gatt_client_write_cfm_cbk(uint8_t client_id, uint16_t conn_id, uint16_t handle, gatt_status_t status)
{
    sample_at_log_print("[gatt client]Write result----client:%d conn_id:%d handle:%d\n", client_id, conn_id, handle);
    sample_at_log_print("            status:%d\n", status);
}

static errcode_t ble_gatt_client_discover_all_service(uint16_t conn_id)
{
    errcode_t ret = ERRCODE_BT_SUCCESS;
    bt_uuid_t service_uuid = {0}; /* uuid length is zero, discover all service */
    ret |= gattc_discovery_service(g_client_id, conn_id, &service_uuid);
    return ret;
}

#if SPEED_TEST_SEND
static void ble_gatt_client_change_2M_phy(uint16_t conn_hdl)
{
    gap_le_set_phy_t phy_param = {
        .conn_handle = conn_hdl,
        .all_phys = 0,
        .tx_phys = GAP_BLE_PHY_2M,
        .rx_phys = GAP_BLE_PHY_2M,
        .phy_options = 0,
    };
    gap_ble_set_phy(&phy_param);
}

static void ble_gatt_client_change_tx_data_len(uint16_t conn_hdl)
{
    gap_le_set_data_length_t data_param = {
        .conn_handle = conn_hdl,
        .maxtxoctets = GAP_MAX_TX_OCTETS,
        .maxtxtime = GAP_MAX_TX_TIME,
    };
    gap_ble_set_data_length(&data_param);
}
#endif

/* 将handle接收到的数据返回 */
static void ble_gatt_client_send_report_back(gattc_handle_value_t *data)
{
    uint8_t *response = malloc((data->data_len + 1) * sizeof(uint8_t));
    memset_s(response, (data->data_len + 1), 0, (data->data_len + 1));
    memcpy_s(response, data->data_len + 1, data->data, data->data_len);
    sample_at_log_print("ble write cbk, report[%s], len: %hu\n", response, data->data_len);

    ble_gatt_client_send_report(g_tx_handle, response, data->data_len);
    free(response);
}

static void ble_gatt_client_mtu_changed_cbk(uint8_t client_id, uint16_t conn_id, uint16_t mtu_size, errcode_t status)
{
    sample_at_log_print("[gatt client]Mtu changed----client:%d conn_id:%d mtu size:%d\n", client_id, conn_id,
                        mtu_size);
    sample_at_log_print("            status:%d\n", status);

#if SPEED_TEST_SEND
    ble_gatt_client_change_2M_phy(g_connect_id);
    ble_gatt_client_change_tx_data_len(g_connect_id);
    usleep(MS_100);
#endif
    ble_gatt_client_discover_all_service(conn_id);
}

static void ble_gatt_client_notification_cbk(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *data,
    errcode_t status)
{
    sample_at_log_print("[gatt client]Receive notification----clientid:%d, conn_id:%d, status:%d, handle:%d\n",
        client_id, conn_id, status, data->handle);
    ble_gatt_client_print_hex(data->data, data->data_len, "recv notificaiton data");

#if !SPEED_TEST_SEND
    ble_gatt_client_send_report_back(data);
#endif
}

static void ble_gatt_client_indication_cbk(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *data,
    errcode_t status)
{
    sample_at_log_print("[gatt client]Receive indication----clientid:%d, conn_id:%d, status:%d, handle:%d\n",
        client_id, conn_id, status, data->handle);
    ble_gatt_client_print_hex(data->data, data->data_len, "recv indication data");

#if !SPEED_TEST_SEND
    ble_gatt_client_send_report_back(data);
#endif
}

/* ble client connect state change callback */
static void ble_gatt_client_connect_state_change_cbk(uint16_t conn_id, bd_addr_t *addr, gap_ble_conn_state_t conn_state,
    gap_ble_pair_state_t pair_state, gap_ble_disc_reason_t disc_reason)
{
    bd_addr_t client_addr = {0};
    client_addr.type = addr->type;
    g_connect_id = conn_id;
    if (memcpy_s(client_addr.addr, BD_ADDR_LEN, addr->addr, BD_ADDR_LEN) != EOK) {
        sample_at_log_print("add server app addr memcpy failed\r\n");
        return;
    }
    sample_at_log_print("connect state change conn_id: %d, status: %d, pair_status:%d, disc_reason %x\n",
        conn_id, conn_state, pair_state, disc_reason);

    if (conn_state == GAP_BLE_STATE_CONNECTED) {
        sample_at_log_print("connect change cbk conn_id =%d \n", conn_id);
        g_connect_cnt++;
        if (pair_state == GAP_BLE_PAIR_NONE) {
            errcode_t ret = gap_ble_pair_remote_device(addr);
            if (ret != ERRCODE_BT_SUCCESS) {
                sample_at_log_print("[gatt client]connect state changed callback, pair remote dev failed:%d.\r\n", ret);
            }
        }
        gattc_exchange_mtu_req(g_client_id, g_connect_id, g_mtu);
    } else if (conn_state == GAP_BLE_STATE_DISCONNECTED) {
        sample_at_log_print("[gatt client] connect change cbk conn disconnected. begin to scan.\n");
#if SPEED_TEST_SEND
        ble_gatt_client_send_thread_reset();
#endif
        gap_ble_remove_pair(addr);
        if (g_connect_cnt > 0) {
            g_connect_cnt--;
        }
        ble_start_scan();
    }
}

/* ble client pair result callback */
static void ble_gatt_client_pair_result_cb(uint16_t conn_id, const bd_addr_t *addr, errcode_t status)
{
    sample_at_log_print("%s pair result conn_id: %d,status: %d \r\n", 0, conn_id, status);
    ble_gatt_client_print_hex(addr->addr, BD_ADDR_LEN, "pair result addr");
}

static errcode_t ble_gatt_client_gatt_callback_register(void)
{
    gattc_callbacks_t cb = {0};

    cb.discovery_svc_cb = ble_gatt_client_discover_service_cbk;
    cb.discovery_svc_cmp_cb = ble_gatt_client_discover_service_compl_cbk;
    cb.discovery_chara_cb = ble_gatt_client_discover_character_cbk;
    cb.discovery_chara_cmp_cb = ble_gatt_client_discover_character_compl_cbk;
#if SPEED_TEST_SEND
    cb.discovery_desc_cb = ble_gatt_client_discover_descriptor_cbk;
    cb.discovery_desc_cmp_cb = ble_gatt_client_discover_descriptor_compl_cbk;
    cb.read_cb = ble_gatt_client_read_cfm_cbk;
    cb.read_cmp_cb = ble_gatt_client_read_compl_cbk;
    cb.write_cb = ble_gatt_client_write_cfm_cbk;
#endif
    cb.mtu_changed_cb = ble_gatt_client_mtu_changed_cbk;
    cb.notification_cb = ble_gatt_client_notification_cbk;
    cb.indication_cb = ble_gatt_client_indication_cbk;
    
    return gattc_register_callbacks(&cb);
}
#endif

static void ble_gatt_client_enable_cbk(uint8_t status)
{
    sample_at_log_print("ble_client_enable status: %d\n", status);
    g_ble_enable_status++;

    errcode_t ret = 0;
    ret |= gattc_register_client(&g_client_app_uuid, &g_client_id);
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("gattc_register_client failed, errcode: %d\r\n", ret);
        return;
    }
    ret |= ble_start_set_scan_parameters();
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("ble_scan_start failed, errcode: %d\r\n", ret);
        return;
    }

    ret |= ble_start_scan();
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("[gatt client] start scan failed, errcode: %d.\r\n", ret);
        return;
    }
    sample_at_log_print("[gatt client] init ok\r\n");
}

static void ble_gatt_client_disable_cbk(uint8_t status)
{
    sample_at_log_print("ble_client_enable cbk, status: %d\n", status);
    if (g_ble_enable_status > 0) {
        g_ble_enable_status--;
    }
}

/* ble client set scan param callback */
static void ble_gatt_client_set_scan_param_cbk(errcode_t status)
{
    sample_at_log_print("set scan param status: %d\n", status);
}

/* ble client scan result callback */
static void ble_gatt_client_scan_result_cbk(gap_scan_result_data_t *scan_result_data)
{
    if (scan_result_data == NULL) {
        sample_at_log_print("ble_scan_ret_cbk, scan_result_data is null.\r\n");
        return;
    }
    ble_gatt_client_print_hex(scan_result_data->addr.addr, BD_ADDR_LEN, "[gatt client] scan result, addr");
    ble_gatt_client_print_hex(scan_result_data->adv_data, scan_result_data->adv_len, "adv data");
#if SUPPORT_GATT_C
    // process scan result
    // using ble_gatt_client_connect_device(gap_scan_result_data_t) if wanting to connect remote device
#endif
#if SPEED_TEST_SEND
    static uint8_t addr[BD_ADDR_LEN] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    if (memcmp(scan_result_data->addr.addr, addr, BD_ADDR_LEN) == 0) {
        ble_gatt_client_connect_device(scan_result_data);
    }
#endif
}

static errcode_t ble_gatt_client_dev_manager_callbacks(void)
{
    bts_dev_manager_callbacks_t dev_mgr = {0};

    dev_mgr.ble_enable_cb = ble_gatt_client_enable_cbk;
    dev_mgr.ble_disable_cb = ble_gatt_client_disable_cbk;

    return bts_dev_manager_register_callbacks(&dev_mgr);
}

static errcode_t ble_gatt_client_gap_callback_register(void)
{
    gap_ble_callbacks_t gap_cb = {0};

    gap_cb.set_scan_param_cb = ble_gatt_client_set_scan_param_cbk;
    gap_cb.scan_result_cb = ble_gatt_client_scan_result_cbk;
#if SUPPORT_GATT_C
    gap_cb.conn_state_change_cb = ble_gatt_client_connect_state_change_cbk;
    gap_cb.pair_result_cb = ble_gatt_client_pair_result_cb;
#endif
    return gap_ble_register_callbacks(&gap_cb);
}

static errcode_t ble_gatt_client_callback_register(void)
{
    errcode_t ret = ERRCODE_BT_SUCCESS;

    ret |= ble_gatt_client_dev_manager_callbacks();
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("[gatt client] reg dev manager cbk failed, errcode: %d.\r\n", ret);
        return ERRCODE_BT_FAIL;
    }

    ret |= ble_gatt_client_gap_callback_register();
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("[gatt client] reg gap cbk failed, errcode: %d.\r\n", ret);
        return ERRCODE_BT_FAIL;
    }
#if SUPPORT_GATT_C
    ret |= ble_gatt_client_gatt_callback_register();
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("[gatt client] reg client cbk failed, errcode: %d.\r\n", ret);
        return ERRCODE_BT_FAIL;
    }
#endif
    return ret;
}

static errcode_t ble_gatt_client_register_persistence_info()
{
    char buff[BYTE_LEN_128] = {0};
    char *path = getcwd(buff, BYTE_LEN_128);
    sample_at_log_print("[gatt client] current path: %s", buff);
    return bts_dev_manager_register_file_path(buff);
}

errcode_t ble_gatt_client_init(void)
{
    errcode_t ret = ERRCODE_BT_SUCCESS;

    ret |= ble_gatt_client_callback_register();
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("[gatt client] reg callbacks failed, errcode: %d\n", ret);
        return ERRCODE_BT_FAIL;
    }

    ret != ble_gatt_client_register_persistence_info();
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("[gatt client] reg persistence_info failed, errcode: %d\n", ret);
    }

    ret |= enable_ble();
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("[gatt client] enable ble failed, errcode: %d\n", ret);
        return ERRCODE_BT_FAIL;
    }
#if SPEED_TEST_SEND
    ble_gatt_client_send_thread_init();
#endif
    return ret;
}

errcode_t ble_gatt_client_deinit(void)
{
#if SPEED_TEST_SEND
    ble_gatt_client_send_thread_deinit();
#endif
    errcode_t ret = ERRCODE_BT_SUCCESS;
    gap_ble_stop_scan();
    ret |= gattc_unregister_client(g_client_id);
    if (ret != ERRCODE_BT_SUCCESS) {
        sample_at_log_print("gattc_unreg_client failed, errorcode: %d\r\n", ret);
        return ERRCODE_BT_FAIL;
    }
#if SUPPORT_GATT_C
    gap_ble_remove_all_pairs();
    while (g_connect_cnt) {
        usleep(BLE_SLEEP_US);
    }
#endif
    ret |= disable_ble();
    while (g_ble_enable_status) {
        usleep(BLE_SLEEP_US);
    }
    return ret;
}
