/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: the head file for hmac ant switch
 * Create: 2022-5-10
 */

#ifndef __HMAC_ANT_SWITCH_H__
#define __HMAC_ANT_SWITCH_H__
#include "mac_user_ext.h"
#include "mac_vap_ext.h"
#include "hal_common_ops.h"

osal_void hmac_calc_rssi_th(const hmac_user_stru *hmac_user, hal_rssi_stru *hal_rssi);
osal_void hmac_ant_rx_frame(hal_to_dmac_device_stru *hal_dev, const hmac_vap_stru *hmac_vap);
osal_void hmac_ant_tbtt_process(hal_to_dmac_device_stru *hal_dev);
#endif

