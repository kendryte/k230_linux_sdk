/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_coex_reg.c.
 * Create: 2020-7-3
 */

#ifndef __HAL_COEX_REG_H__
#define __HAL_COEX_REG_H__

/*****************************************************************************
  其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "hal_common_ops.h"
#include "hal_soc_reg.h"
#include "hal_gp_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_COEX_REG_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define BLE_STATUS_MASK   0xFFFF
#define BT_STATUS_OFFSET  16

#define BT_ACL_STATUS_MASK (BIT14)
#define BT_SCO_STATUS_MASK (BIT4)

#define BT_M2S_6SLOT_MASK (BIT0)
#define BT_M2S_LDAC_MASK (BIT1)
#define BT_M2S_A2DP_MASK (BIT3)

#define BT_ACL_STATUS_OFFSET_LEN 14
#define BT_SCO_STATUS_OFFSET_LEN 4

#define BTCOEX_POWSAVE_TIMEOUT_LEVEL0 100 // 音乐或者数传 ps时间 ms  蓝牙可能重传较多时，由65ms改成100ms
#define BTCOEX_POWSAVE_TIMEOUT_LEVEL1 200 // page或inquiry或ldac时间 ms
#define BTCOEX_POWSAVE_TIMEOUT_LEVEL2 300 // a2dp|transfer和page或inquiry的ps时间

#define BTCOEX_S2M_RESUME_TIMEOUT 3000 // 切回mimo 状态等待时间

/* wifi状态结构体, GP_REG2(0x40019388) 32bit */
typedef struct {   /* 73仅启用前4个状态 */
    osal_u32  wlan_on        : 1, /* WIFI：1：on,0:off */
            wlan_chan_num    : 4, /* 信道：1-14 */
            wlan_bandwidth   : 4, /* 带宽: 20/40M */
            wlan_connect     : 1, /* 连接 */
            wlan_sleep       : 1, /* 睡眠 */
            wlan_busy        : 1, /* WIFI：打流 */
            wlan_scan        : 1, /* 扫描 */
            wlan_ps_stop     : 1, /* 停止ps机制 */
            wlan_fre         : 1, /* 频段   */
            wlan_p2p_scan    : 1, /* p2p扫描 */
            wlan_p2p_conn    : 1, /* p2p连接 */

            wlan_dbac        : 4, /* dbac状态 */
            wlan_diff_chan   : 1, /* dbac下多信道共存 */
            wlan_joining     : 1, /* sta加入ap */
            wlan_pro_11bgn   : 1, /* 11bgn协议 */
            wlan_pro_11ax    : 1, /* 11ax协议 */
            wlan_ps_slot_step     : 2, /* ps请求申请提前量 */
            wlan_rsv              : 5;
} wifi_status_stru;

typedef union {
    wifi_status_stru   wifi_status;
    osal_u32           u32;
} wifi_status_union;

typedef struct {
    osal_u16  bt_on            : 1,
            bt_inquiry       : 1,
            bt_page          : 1,
            bt_a2dp          : 1,
            bt_sco           : 1,
            bt_data_send     : 1, // acl data send
            bt_data_rcv      : 1, // acl data recv
            bt_transfer      : 1, // not use (only wifi self) bt_data_send || bt_data_rcv
            bt_6slot         : 2, // BT soc slot, 12slot=1, 6solt=2
            bt_ldac          : 3, // a2dp_code状态:2/3为ldac,wifi侧处理时，重设1为ldac, 其他为0
            bt_acl           : 1, // 蓝牙有业务时，通过wifi发送ps状态给路由
            bt_ba            : 1, // not use (only wifi self)
            bt_resv          : 1;
} bt_status_stru;

typedef union {
    osal_u16 bt_status_reg;
    bt_status_stru bt_status;
} btcoex_bt_status_union;

typedef struct {
    osal_u16  ble_on         : 1, // 只要有ble_scan，ble_adv，ble_init，ble_con有一个启动, 就为1
            ble_scan         : 1, // BLE进入扫描状态
            ble_adv          : 1, // BLE起业务时, 有广播发送
            ble_init         : 1, // 有ble设备创建连接时, 才设置1
            ble_con          : 1, // ble设备已经建立连接
            ble_hid          : 1, // BLE HID设备
            ble_con_num      : 5, // ble总连接数
            ble_rsv          : 5;
} ble_status_stru;

typedef union {
    osal_u16        ble_status_reg;
    ble_status_stru ble_status;
} btcoex_ble_status_union;

typedef struct {
    osal_u16  req_ba        : 1, // 请求删建ba
              req_ps        : 1, // 请求wifi进入节能
              req_rsv       : 14;
} bsle_req_stru;

typedef union {
    osal_u16        ble_req_reg;
    bsle_req_stru   ble_req;
} btcoex_ble_req_union;

typedef struct {
    btcoex_ble_status_union ble_status;
    btcoex_bt_status_union  bt_status;
    btcoex_ble_req_union    action_req;
    osal_u16                rsv;
} hal_btcoex_btble_status_stru;

typedef struct {
    ble_status_stru ble_status;
    bt_status_stru  bt_status;
    bsle_req_stru   action_req;
    osal_u16        rsv;
    osal_u32        timestamp;
} hal_btcoex_btble_status_msg_stru;

typedef struct {
    osal_u8  abort_duration_on;
    osal_u8  msl_cnt_enable;
    osal_u8  post_dev_enable;
    osal_u8  rsv;
    osal_u32 abort_duration_start_us;
    osal_u32 xGji_LwFd_GLBihwdC_;
    osal_u32 xzyvwRO4SwzRlvoO_;

    osal_u32 msl_intr_cnt[HAL_BTCOEX_PS_BUTT];
    osal_u32 msl_bt_duration[HAL_BTCOEX_PS_BUTT];
    osal_u32 abort_start_cnt[HAL_BTCOEX_PS_BUTT];
    osal_u32 ps_on_duration;
    osal_u32 ps_on_last;
    osal_u32 abort_start_last;
    osal_u16 ps_msg_delay[8];  /* 分别记录1~8ms delay */

    /* DFX命令行使用 */
    osal_u32 msl_intr_cnt_record[HAL_BTCOEX_PS_BUTT];
    osal_u32 msl_bt_duration_record[HAL_BTCOEX_PS_BUTT];
    osal_u32 abort_start_cnt_record[HAL_BTCOEX_PS_BUTT];
    osal_u32 ps_on_duration_record;
    osal_u32 timestamp_record;
} hal_btcoex_statistics_stru;

typedef union {
    struct {
        osal_u32 xGji_LwFd_GLBihwdC_ : 20; /* [0..11]  */
        osal_u32 xzyvwRO4SwzRlvoO_ : 12;        /* [12..31]  */
    } abort_time;

    /* Define an unsigned member */
    osal_u32 u32;
} abort_duration_union;

/*****************************************************************************
  函数声明
*****************************************************************************/
hal_btcoex_btble_status_stru *hal_btcoex_btble_status_last(osal_void);
hal_btcoex_btble_status_stru *hal_btcoex_btble_status(osal_void);
hal_btcoex_statistics_stru *hal_btcoex_statistics(osal_void);
osal_void hal_device_set_state_btcoex_notify(oal_bool_enum_uint8 wifi_work);
osal_void hal_device_btcoex_sw_init(hal_to_dmac_device_stru *hal_device_base);
osal_u32 hal_btcoex_init(const hal_to_dmac_device_stru *hal_device);
osal_u32 hal_btcoex_sw_preempt_init(hal_to_dmac_device_stru *hal_device);
osal_void hal_btcoex_sw_preempt_exit(hal_to_dmac_device_stru *hal_device);
osal_void hal_btcoex_clear_reg(osal_void);
osal_void hal_btcoex_get_ps_service_status(const hal_to_dmac_device_stru *hal_device,
    hal_btcoex_ps_status_enum_uint8 *ps_status);
osal_void hal_btcoex_get_bt_sco_status(const hal_to_dmac_device_stru *hal_device,
    oal_bool_enum_uint8 *sco_status);

osal_bool hal_btcoex_get_bt_run(osal_void);
osal_void hal_set_btcoex_hw_priority_en(oal_bool_enum_uint8 hw_prio_en);
osal_void hal_set_btcoex_priority_period(osal_u16 priority_period);
osal_void hal_set_btcoex_occupied_period(osal_u16 occupied_period);
osal_void hal_set_btcoex_abort_qos_null_seq_num(hal_to_dmac_device_stru *hal_device,
    osal_u32 qosnull_seq_num);
osal_void hal_get_btcoex_abort_qos_null_seq_num(hal_to_dmac_device_stru *hal_device,
    osal_u32 *qosnull_seq_num);
osal_void hal_set_btcoex_abort_preempt_frame_param(osal_u16 preempt_param);
osal_void hal_set_btcoex_tx_abort_preempt_type(hal_coex_hw_preempt_mode_enum_uint8 preempt_type);
osal_void hal_set_btcoex_sw_all_abort_ctrl(oal_bool_enum_uint8 sw_abort_ctrl);
osal_void hal_coex_sw_irq_set(hal_coex_sw_irq_type_enum_uint8 coex_irq_type);
osal_void hal_set_btcoex_wifi_status_notify(hal_btcoex_wifi_state_id_enum_uint8 id, osal_u32 val);
osal_void hal_device_set_state_btcoex_notify(oal_bool_enum_uint8 wifi_work);
osal_void hal_update_btcoex_btble_status(hal_btcoex_btble_status_stru *st_cur);
osal_void hal_btcoex_sync_btble_status(hal_btcoex_btble_status_msg_stru *status_msg);
osal_void hal_btcoex_get_abort_cnt(hal_to_dmac_chip_stru *hal_chip,
    osal_u32 *abort_start, osal_u32 *abort_done, osal_u32 *abort_end);
osal_u32 hal_btcoex_update_ap_beacon_count(osal_void);
osal_void hal_coex_notify_irq_en(osal_u8 intr_en);

#ifdef _PRE_BSLE_GATEWAY
osal_void hal_bsle_set_priority(osal_void);
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_BTCOEX */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
