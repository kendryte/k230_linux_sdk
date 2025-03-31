/**
 * Copyright (c) @CompanyNameMagicTag 2024. All rights reserved.
 * Description: BLE config client scan.
 */

#include "errcode.h"
#include "bts_def.h"
#include "bts_le_gap.h"
#include "ble_client_scan.h"

static uint16_t scan_interval = 96;
static uint16_t scan_window = 48;
static uint8_t  scan_type = GAP_BLE_SCAN_TYPE_ACTIVE;
static uint8_t  scan_phy = GAP_BLE_PHY_CODED;
static uint8_t  scan_filter_policy = GAP_BLE_SCAN_FILTER_POLICY_ACCEPT_ALL;

errcode_t ble_start_set_scan_parameters(void)
{
    gap_ble_scan_params_t ble_device_scan_params = { 0 };
    ble_device_scan_params.scan_interval = scan_interval;
    ble_device_scan_params.scan_window = scan_window;
    ble_device_scan_params.scan_type = scan_type;
    ble_device_scan_params.scan_phy = scan_phy;
    ble_device_scan_params.scan_filter_policy = scan_filter_policy;
    return gap_ble_set_scan_parameters(&ble_device_scan_params);
}

errcode_t ble_start_scan(void)
{
    return gap_ble_start_scan();
}