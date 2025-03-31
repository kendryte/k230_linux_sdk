/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: chip init cfg.
 * Create: 2021-12-13
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include "hal_chip.h"
#include "wlan_spec.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "hal_rf.h"
#include "hal_device.h"
#include "frw_osal.h"
#include "hal_mac.h"
#include "hal_soc.h"
#include "fe_extern_if_host.h"
#include "hal_init.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_INIT_C

osal_void hal_device_hw_init(hal_to_dmac_device_stru *hal_device_base)
{
    hal_disable_machw_phy_and_pa();
    /* 初始化MAC硬件 */
    hal_initialize_machw();
    /* 初始化PHY */
    hal_initialize_phy();

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 初始化共存mac和phy控制寄存器 */
    hal_btcoex_init(hal_device_base);

    /* 初始化软件preempt机制参数 */
    hal_btcoex_sw_preempt_init(hal_device_base);
#ifdef _PRE_BSLE_GATEWAY
    hal_bsle_set_priority();
#endif
#endif

    /* 初始化hal tx power */
    hal_pow_initialize_tx_power(hal_device_base);

    // 功率控制初始化
    cali_pow_initialize_tx_power(hal_device_base);

    /* hal device 统计信息初始化 */
    hal_device_dft_stat_init(hal_device_base);
}

