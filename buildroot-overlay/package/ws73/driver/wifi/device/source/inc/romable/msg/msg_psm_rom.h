/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: msg.
 * Create: 2021-11-29
 */

#ifndef MSG_PSM_ROM_H
#define MSG_PSM_ROM_H

#include "wlan_resource_common_rom.h"

typedef struct {
    mac_pm_ctrl_type_enum_uint8 pm_ctrl_type; /* mac_pm_ctrl_type_enum */
    mac_pm_switch_enum_uint8 pm_enable;       /* mac_pm_switch_enum */
} mac_cfg_ps_open_stru;

typedef struct {
    osal_u8 is_max_power_set : 1;
    osal_u8 is_keep_alive_set : 1;
    osal_u8 is_all_vap : 1;
    osal_u8 bit_rsv : 5;
    osal_u8 max_power_set;
    osal_u16 in_tbtt_offset;
} mac_psm_sta_pm_param_dev_stru;

typedef struct {
    osal_u16 pm_timer;
    osal_u16 pm_timer_cnt;
    osal_u16 beacon_timeout;
    osal_u16 mcast_timeout;
    osal_u16 tbtt_offset;
    osal_u16 sleep_time;
} mac_cfg_ps_param_stru;
#endif