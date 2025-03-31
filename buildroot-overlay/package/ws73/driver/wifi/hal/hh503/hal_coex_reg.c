/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Coexist interfaces of WiFi and other system.
 * Create: 2020-7-3
 */

#ifdef _PRE_WLAN_FEATURE_BTCOEX

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_coex_reg.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "wlan_mib_hcm.h"
#include "hal_ext_if.h"
#include "hal_device.h"
#include "hal_chip.h"
#include "hal_phy.h"
#include "hal_mac.h"
#include "hal_rf.h"
#include "hal_reset.h"
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "frw_util.h"
#include "wlan_msg.h"
#include "hal_soc_reg.h"
#include "hal_gp_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_COEX_REG_C

#define HAL_BTCOEX_ABORT_DONE_IRQ_MASK 15

hal_btcoex_btble_status_stru g_btble_status_last = {0}; /* 最近一次消息上报的btble状态 */
hal_btcoex_btble_status_stru g_btcoex_btble_status = {0};
hal_btcoex_statistics_stru g_btcoex_statistics = {0};

WIFI_HMAC_TCM_TEXT hal_btcoex_btble_status_stru *hal_btcoex_btble_status(osal_void)
{
    return &g_btcoex_btble_status;
}

hal_btcoex_statistics_stru *hal_btcoex_statistics(osal_void)
{
    abort_duration_union abort_time;
    abort_time.u32 = hal_gp_get_btcoex_abort_time();
    g_btcoex_statistics.xzyvwRO4SwzRlvoO_ = abort_time.abort_time.xzyvwRO4SwzRlvoO_;
    g_btcoex_statistics.xGji_LwFd_GLBihwdC_ = abort_time.abort_time.xGji_LwFd_GLBihwdC_;
    return &g_btcoex_statistics;
}

hal_btcoex_btble_status_stru *hal_btcoex_btble_status_last(osal_void)
{
    return &g_btble_status_last;
}

osal_u16 hal_btcoex_get_bt_req(osal_void)
{
    return hal_reg_read16(HH503_COEX_CTL_RB_COEX_GP_REG0_REG);
}

/*****************************************************************************
 函 数 名  : hal_device_btcoex_sw_init
 功能描述  : btcoex软件能力初始化
*****************************************************************************/
osal_void hal_device_btcoex_sw_init(hal_to_dmac_device_stru *hal_device_base)
{
    hal_device_btcoex_mgr_stru *device_btcoex_mgr = OSAL_NULL;

    device_btcoex_mgr = &(hal_device_base->device_btcoex_mgr);

    /* 1.针对m2s做定制化初始化 */
    (osal_void)memset_s(device_btcoex_mgr, sizeof(hal_device_btcoex_mgr_stru), 0, sizeof(hal_device_btcoex_mgr_stru));
    hal_device_base->device_btcoex_mgr.m2s_6slot =
        ((hal_device_get_m2s_mask() & HAL_M2S_MODE_BITMAP_6SLOT) != 0) ? OSAL_TRUE : OSAL_FALSE;
    hal_device_base->device_btcoex_mgr.m2s_ldac =
        ((hal_device_get_m2s_mask() & HAL_M2S_MODE_BITMAP_LDAC) != 0) ? OSAL_TRUE : OSAL_FALSE;

    oam_warning_log2(0, OAM_SF_COEX,
        "hal_device_btcoex_sw_init::support siso switch 6slot[%d]ldac[%d].",
        hal_device_base->device_btcoex_mgr.m2s_6slot,
        hal_device_base->device_btcoex_mgr.m2s_ldac);
}

/*****************************************************************************
 函 数 名  : hal_btcoex_init
 功能描述  : 共存初始化相关参数
******************************************************************************/
osal_u32 hal_btcoex_init(const hal_to_dmac_device_stru *hal_device)
{
    u_pa_control1 pa_control1;
    u_coex_ctrl1 coex_ctrl1;
    u_coex_ctrl2 coex_ctrl2;
    unref_param(hal_device);

    /******************** coex_ctrl1配置 *****************/
    coex_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x70);
    /* 共存RF稳定时间寄存器配置，目前配置为120us，后续RF给出后改为精确值 */
    coex_ctrl1.bits.xCt2OCvm8OwtOsRzyxmORlIm_ = 0x78;
    /* 配置mac不发preempt帧 */
    coex_ctrl1.bits.xCt2OCvm8Ow8OR8OzyvwROGwmmIGROmo_ = 0;
    coex_ctrl1.bits.x6Ngw6iWPwLPwGji_Lwu_WWSuLwLtuW_ = 0;

    /* 默认接收时不允许输出priority信号 */
    coex_ctrl1.bits.x6Ngw6iWPwQTw_Pwu_Bi_BLtwFBC_ = 1;
    /* 高优先级配置 */
    coex_ctrl1.bits.x6Ngw6iWPwQTwLPw_WCuwjGwu_BwSiFW_ = 1;    // 配置ba发送优先级
    /* 普通优先级配置 */
    coex_ctrl1.bits.x6Ngw6iWPwQTw_Pw_WCuwjGwu_BwSiFW_ = 0;    // 配置ba接收优先级
    coex_ctrl1.bits.x6Ngw6iWPwQTw_Pw_WCuwiLQW_wu_BwSiFW_ = 0; // 配置ack接收优先级
    coex_ctrl1.bits.xCt2OCvm8OAqOw8OwmsGOCRsOGwlOIv4m_ = 0;   // 配置cts接收优先级
    coex_ctrl1.bits.xCt2OCvm8OAqOR8OwmsGOCRsOGwlOIv4m_ = 0;   // 配置cts发送优先级
    coex_ctrl1.bits.x6Ngw6iWPwQTwLPw_WCuwiLQW_wu_BwSiFW_ = 0; // 配置ack发送优先级
    /* 配置硬件软件均可配置高优先级priority */
    coex_ctrl1.bits.x6Ngw6iWPwu_Bi_BLtwSiFW_ = 0x2;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x70, coex_ctrl1.u32);

    /******************** coex_ctrl2配置 *****************/
    coex_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x74);
    /* 每一帧允许的最多abort次数配置为10 */
    coex_ctrl2.bits.x6Ngw6iWPwSGPw6hL_ = BT_ABORT_RETRY_TIMES_MAX;
    /* 关闭mac逻辑的post preempt功能 */
    coex_ctrl2.bits.xCt2OCvm8OR8OzyvwROGvsROGwmmIRO4ls_ = 0;
    /* 关闭mac逻辑的preempt功能 */
    coex_ctrl2.bits.x6Ngw6iWPwLPwu_WWSuLwGihwWh_ = 0;
    /* 天线被切走时，有beacon发送，关闭tx使能 */
    coex_ctrl2.bits.xCt2OCvm8OR8OyCoOwm6Omo_ = 0;
    /* 天线被切走时，高优先级或广播帧队列有帧发送，输出tx使能 */
    coex_ctrl2.bits.x6Ngw6iWPwLPwQBu_BwRw_WRwWh_ = 1;
    /* 天线被切走时，普通发送队列有帧发送，输出tx使能 */
    coex_ctrl2.bits.xCt2OCvm8OR8OovwIO6Owm6Omo_ = 1;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x74, coex_ctrl2.u32);
    /* 屏蔽abort_done中断 */
    hal_mask_interrupt(HAL_BTCOEX_ABORT_DONE_IRQ_MASK);
    /******************** coex使能 *****************/
    pa_control1.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    pa_control1.bits.xCt2OCvm8OzyvwROmo_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_control1.u32);

    hal_coex_notify_irq_en((osal_u8)(HAL_COEX_SW_IRQ_BT));

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_btcoex_sw_preempt_init
 功能描述  : 共存sw preempt初始化相关参数
******************************************************************************/
osal_u32 hal_btcoex_sw_preempt_init(hal_to_dmac_device_stru *hal_device)
{
    hal_device_btcoex_sw_preempt_stru *btcoex_sw_preempt = &(hal_device->btcoex_sw_preempt);

    (osal_void)memset_s(btcoex_sw_preempt, sizeof(hal_device_btcoex_sw_preempt_stru), 0x0,
        sizeof(hal_device_btcoex_sw_preempt_stru));

    /* 针对tplink路由器音乐下行性能普遍低，需要关闭回复CTS，提升整体性能稳定性 */
    /* 初始化软件preempt机制参数 */
    btcoex_sw_preempt->protect_coex_pri = HAL_FCS_PROTECT_COEX_PRI_OCCUPIED; /* one pkt帧优先级 */
    btcoex_sw_preempt->sw_preempt_mode.ps_on = OSAL_FALSE;                 /* 开启ps使能 */
    btcoex_sw_preempt->sw_preempt_mode.delba_on = OSAL_TRUE;              /* 初始删聚合,未使用 */
    btcoex_sw_preempt->sw_preempt_mode.reply_cts = OSAL_FALSE;            /* 初始回CTS */
    btcoex_sw_preempt->sw_preempt_mode.rsp_frame_ps = OSAL_TRUE;          /* 初始响应帧置节能位 */

    btcoex_sw_preempt->coex_pri_forbit = OSAL_FALSE;                  /* 允许配置onepkt优先级 */
    btcoex_sw_preempt->last_acl_status = OSAL_FALSE;
    btcoex_sw_preempt->ps_stop = OSAL_FALSE;                           /* 初始默认业务场景下关闭ps机制 */
    btcoex_sw_preempt->ps_pause = OSAL_FALSE;                         /* 初始默认业务场景下不暂停ps机制 */
    btcoex_sw_preempt->sw_preempt_type = HAL_BTCOEX_SW_POWSAVE_IDLE;  /* 初始ps使能 */
    btcoex_sw_preempt->sw_preempt_subtype = HAL_BTCOEX_SW_POWSAVE_SUB_ACTIVE;
    btcoex_sw_preempt->timeout_ms = BTCOEX_POWSAVE_TIMEOUT_LEVEL0; /* 初始化ps机制超时时间 */

    /* ps_event_num初始值修改为1，防止ps可能连续抛两个以上事件 */
    osal_adapt_atomic_set(&btcoex_sw_preempt->ps_event_num, 1);

    oam_warning_log3(0, OAM_SF_COEX,
                     "{hal_btcoex_sw_preempt_init::sw_preempt_mode is [0x%x], type[%d], subtype[%d].}",
                     *(osal_u8 *)(&btcoex_sw_preempt->sw_preempt_mode), btcoex_sw_preempt->sw_preempt_type,
                     btcoex_sw_preempt->sw_preempt_subtype);

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hal_btcoex_sw_preempt_exit
 功能描述  : 共存sw preempt初始化相关参数
******************************************************************************/
osal_void hal_btcoex_sw_preempt_exit(hal_to_dmac_device_stru *hal_device)
{
    hal_device_btcoex_sw_preempt_stru *btcoex_sw_preempt = &(hal_device->btcoex_sw_preempt);
    osal_adapt_atomic_set(&btcoex_sw_preempt->ps_event_num, 0);
    hal_coex_notify_irq_en(0x0);
}

osal_bool hal_btcoex_get_bt_run(osal_void)
{
    btcoex_ble_status_union ble_status_union = hal_btcoex_btble_status()->ble_status;
    if (ble_status_union.ble_status.ble_scan == OSAL_TRUE || ble_status_union.ble_status.ble_adv == OSAL_TRUE ||
        ble_status_union.ble_status.ble_init == OSAL_TRUE || ble_status_union.ble_status.ble_con == OSAL_TRUE ||
        ble_status_union.ble_status.ble_hid == OSAL_TRUE) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名 : hal_btcoex_clear_reg
 功能描述     : 清除wifi侧通知BT的状态
*****************************************************************************/
osal_void hal_btcoex_clear_reg(osal_void)
{
    hal_gp_set_btcoex_wifi_status(0);
    hal_coex_sw_irq_set(HAL_COEX_SW_IRQ_BT);
}

/*****************************************************************************
 函 数 名  : hal_btcoex_get_ps_service_status
 功能描述  : 获取当前ps业务状态
*****************************************************************************/
osal_void hal_btcoex_get_ps_service_status(const hal_to_dmac_device_stru *hal_device,
    hal_btcoex_ps_status_enum_uint8 *ps_status)
{
    btcoex_bt_status_union bt_status;
    osal_u8 cur_ps_status = HAL_BTCOEX_PS_STATUE_ACL;
    bt_status_stru *bt_status0 = OSAL_NULL;
    unref_param(hal_device);

    bt_status.bt_status_reg = hal_gp_get_btcoex_bt_status() >> BT_STATUS_OFFSET;
    bt_status0 = &(bt_status.bt_status);

    /* 是否有音乐或者数传状态 */
    if (bt_status0->bt_a2dp || bt_status0->bt_data_send || bt_status0->bt_data_rcv) {
        cur_ps_status |= HAL_BTCOEX_PS_STATUE_ACL;
    }

    /* 是否有page或者inquiry状态 */
    if (bt_status0->bt_inquiry || bt_status0->bt_page) {
        cur_ps_status |= HAL_BTCOEX_PS_STATUE_PAGE_INQ;
    }

    /* 是否有ldac状态 */
    if (bt_status0->bt_ldac) {
        cur_ps_status |= HAL_BTCOEX_PS_STATUE_LDAC;
    }

    *ps_status = (hal_btcoex_ps_status_enum_uint8)cur_ps_status;
}

/*****************************************************************************
 函 数 名  : hal_btcoex_get_bt_sco_status
 功能描述  : 获取当前sco业务状态
*****************************************************************************/
osal_void hal_btcoex_get_bt_sco_status(const hal_to_dmac_device_stru *hal_device,
    oal_bool_enum_uint8 *sco_status)
{
    osal_u16 bt_status;

    unref_param(hal_device);
    bt_status = hal_gp_get_btcoex_bt_status() >> BT_STATUS_OFFSET;
    *sco_status = (oal_bool_enum_uint8)((bt_status & BT_SCO_STATUS_MASK) >> BT_SCO_STATUS_OFFSET_LEN);
}

/*****************************************************************************
 输入参数  :硬件配置WLAN业务优先级使能位
            0：无效
            1：有效
*****************************************************************************/
osal_void hal_set_btcoex_hw_priority_en(oal_bool_enum_uint8 hw_prio_en)
{
    u_coex_priority_period coex_period;

    coex_period.u32 = hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x64);
    if (hw_prio_en) {
        coex_period.bits.xCt2OCvm8OGwlvwlRQOGmwlv4_ = 0;
    } else {
        /* 持续拉高WIFI优先级 */
        coex_period.bits.xCt2OCvm8OGwlvwlRQOGmwlv4_ = 0xFFFF;
    }

    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x64, coex_period.u32);
}

/*****************************************************************************
 函 数 名  : hal_set_btcoex_priority_period
 输入参数  :MAC处于priority的时间，从软件配置这个寄存器开始计时，单位us。
*****************************************************************************/
osal_void hal_set_btcoex_priority_period(osal_u16 priority_period)
{
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x64, priority_period);
}

/*****************************************************************************
 输入参数  :MAC处于occupied的时间，从软件配置这个寄存器开始计时，单位us。
*****************************************************************************/
osal_void hal_set_btcoex_occupied_period(osal_u16 occupied_period)
{
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x68, occupied_period);
}

/*****************************************************************************
 函 数 名  : hal_set_btcoex_abort_qos_null_seq_nume
*****************************************************************************/
osal_void hal_set_btcoex_abort_qos_null_seq_num(hal_to_dmac_device_stru *hal_device,
    osal_u32 qosnull_seq_num)
{
    unref_param(hal_device);
    unref_param(qosnull_seq_num);
    oam_info_log0(0, OAM_SF_CFG, "hal_set_btcoex_abort_qos_null_seq_num::empty function");
}

/*****************************************************************************
 函 数 名  : hal_get_btcoex_abort_qos_null_seq_num
*****************************************************************************/
osal_void hal_get_btcoex_abort_qos_null_seq_num(hal_to_dmac_device_stru *hal_device,
    osal_u32 *qosnull_seq_num)
{
    unref_param(hal_device);
    unref_param(qosnull_seq_num);
    oam_info_log0(0, OAM_SF_CFG, "hal_get_btcoex_abort_qos_null_seq_num::empty function");
}

/*****************************************************************************
 函 数 名  : hal_set_btcoex_abort_premmpt_frame_param
 功能描述  : 配置premmpt帧参数
*****************************************************************************/
osal_void hal_set_btcoex_abort_preempt_frame_param(osal_u16 preempt_param)
{
    osal_u8 cfg_coex_tx_peer_index = preempt_param & 0x1f;
    osal_u8 cfg_coex_tx_qos_null_tid = (preempt_param >> 8) & 0xf;
    osal_u8 cfg_coex_tx_vap_index = (preempt_param >> 12) & 0xf;
    u_coex_ctrl2 coex_ctrl2;

    coex_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x74);
    coex_ctrl2.bits.cfg_coex_tx_peer_index = cfg_coex_tx_peer_index;
    coex_ctrl2.bits.cfg_coex_tx_qos_null_tid = cfg_coex_tx_qos_null_tid;
    coex_ctrl2.bits.cfg_coex_tx_vap_index = cfg_coex_tx_vap_index;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x74, coex_ctrl2.u32);
}

/*****************************************************************************
 功能描述  : 配置mac的preempt帧类型，默认是00，不发帧，这个是发帧总开关
*****************************************************************************/
osal_void hal_set_btcoex_tx_abort_preempt_type(hal_coex_hw_preempt_mode_enum_uint8 preempt_type)
{
    u_coex_ctrl1 coex_ctrl1;

    coex_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x70);
    if (preempt_type <= HAL_BTCOEX_HW_POWSAVE_QOSNULL) {
        coex_ctrl1.bits.x6Ngw6iWPwLPwGji_Lwu_WWSuLwLtuW_ = preempt_type;
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x70, coex_ctrl1.u32);
    }
}

/*****************************************************************************
 功能描述  : MAC硬件对于Abort模块使能标志位，无效时屏蔽Abort模块所有输入输出状态
*****************************************************************************/
osal_void hal_set_btcoex_sw_all_abort_ctrl(oal_bool_enum_uint8 sw_abort_ctrl)
{
    hal_reg_write_bits_by_msg(HH503_MAC_CTRL0_BANK_BASE_0x0, 11, 1, sw_abort_ctrl, OSAL_TRUE); // 11,cfg_coex_abort_en
}

/* 软中断通知bsle */
osal_void hal_coex_sw_irq_set(hal_coex_sw_irq_type_enum_uint8 coex_irq_type)
{
    hal_reg_write16(HH503_COEX_CTL_RB_INT_SET_WL_REG, coex_irq_type);
}

/* WIFI通过写共存通用寄存器，通知蓝牙工作状态 */
osal_void hal_set_btcoex_wifi_status_notify(hal_btcoex_wifi_state_id_enum_uint8 id, osal_u32 val)
{
    wifi_status_union coex_wifi_status;

    coex_wifi_status.u32 = hal_gp_get_btcoex_wifi_status();

    if (id == HAL_BTCOEX_WIFI_STATE_ON) {
        coex_wifi_status.wifi_status.wlan_on = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_CHAN_NUM) {
        coex_wifi_status.wifi_status.wlan_chan_num = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_BAND_WIDTH) {
        coex_wifi_status.wifi_status.wlan_bandwidth = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_SLEEP) {
        coex_wifi_status.wifi_status.wlan_sleep = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_JOINING) {
        coex_wifi_status.wifi_status.wlan_joining = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_SCAN) {
        coex_wifi_status.wifi_status.wlan_scan = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_CONN) {
        coex_wifi_status.wifi_status.wlan_connect = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_P2P_SCAN) {
        coex_wifi_status.wifi_status.wlan_p2p_scan = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_P2P_CONN) {
        coex_wifi_status.wifi_status.wlan_p2p_conn = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_PS_STOP) {
        coex_wifi_status.wifi_status.wlan_ps_stop = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_WORK_MODE) {
        coex_wifi_status.wifi_status.wlan_dbac = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_DIFF_CHANNEL) {
        coex_wifi_status.wifi_status.wlan_diff_chan = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_PROTOCOL_2G11BGN) {
        coex_wifi_status.wifi_status.wlan_pro_11bgn = val;
    } else if (id == HAL_BTCOEX_WIFI_STATE_PROTOCOL_2G11AX) {
        coex_wifi_status.wifi_status.wlan_pro_11ax = val;
    } else if (id == HAL_BTCOEX_WIFI_BUSY_STATUS) {
        coex_wifi_status.wifi_status.wlan_busy = val;
    } else {
        return;
    }

    hal_gp_set_btcoex_wifi_status(coex_wifi_status.u32);
}

/*****************************************************************************
 功能描述  : wifi处于工作或者非工作状态通知蓝牙，便于蓝牙调整业务速率
*****************************************************************************/
osal_void hal_device_set_state_btcoex_notify(oal_bool_enum_uint8 wifi_work)
{
    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_SLEEP, (osal_u32)wifi_work);
    hal_coex_sw_irq_set(HAL_COEX_SW_IRQ_BT);
}

osal_void hal_update_btcoex_btble_status(hal_btcoex_btble_status_stru *st_cur)
{
    st_cur->ble_status.ble_status_reg = hal_gp_get_btcoex_bt_status() & BLE_STATUS_MASK;
    st_cur->action_req.ble_req_reg = hal_btcoex_get_bt_req();
    st_cur->bt_status.bt_status.bt_on = st_cur->ble_status.ble_status.ble_on;
}

/*****************************************************************************
 函 数 名  : hal_btcoex_sync_btble_status
 功能描述  : 同步btble到hal数据结构中
******************************************************************************/
osal_void hal_btcoex_sync_btble_status(hal_btcoex_btble_status_msg_stru *status_msg)
{
    hal_btcoex_btble_status_stru *status = hal_btcoex_btble_status();
    status->ble_status.ble_status = status_msg->ble_status;
    status->action_req.ble_req = status_msg->action_req;
    /* 在与bsle共存时，bt_on表示整个蓝牙系统是否开启 */
    status->bt_status.bt_status.bt_on = status_msg->ble_status.ble_on;
    oam_info_log3(0, OAM_SF_COEX, "hal_btcoex_sync_btble_status::ble:0x%04x, req:0x%04x, bt:0x%04x.\r\n",
        status->ble_status.ble_status_reg, status->action_req.ble_req_reg, status->bt_status.bt_status_reg);
}

/*****************************************************************************
 函 数 名  : hal_btcoex_get_abort_cnt
 功能描述  : 读取btcoex abort统计
******************************************************************************/
osal_void hal_btcoex_get_abort_cnt(hal_to_dmac_chip_stru *hal_chip,
    osal_u32 *abort_start, osal_u32 *abort_done, osal_u32 *abort_end)
{
    u_rpt_coex_1 coex_1;
    u_rpt_coex_2 coex_2;

    coex_1.u32 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0xC4);
    coex_2.u32 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0xC8);

    *abort_done = coex_1.bits.xwGROCvm8OzyvwRO4vomOCoR_;
    *abort_start = coex_1.bits.xwGROCvm8OzyvwROsRzwROCoR_;
    *abort_end = coex_2.bits.xwGROCvm8OzyvwROmo4OCoR_;
}

/*****************************************************************************
 功能描述  : 共存APUT统计BEACON帧发送个数
******************************************************************************/
osal_u32 hal_btcoex_update_ap_beacon_count(osal_void)
{
    u_rpt_tx_bcn_count counts;
    counts.u32 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x30);
    return counts.bits.xR8OymzCvoOCvSoR_;
}
#if defined (_PRE_WLAN_FEATURE_BTCOEX) && defined (_PRE_BSLE_GATEWAY)
osal_void hal_bsle_set_priority(osal_void)
{
    u_cfg_force_priority cfg_force_priority;

    cfg_force_priority.u16 = hal_reg_read16(HH503_COEX_CTL_RB_CFG_FORCE_PRIORITY_REG);
    cfg_force_priority.bits.cfg_force_bsle_prio_higher = 1;
    hal_reg_write16(HH503_COEX_CTL_RB_CFG_FORCE_PRIORITY_REG, cfg_force_priority.u16);
}
#endif
osal_void hal_coex_notify_irq_en(osal_u8 intr_en)
{
    hal_reg_write16(HH503_COEX_CTL_RB_INT_EN_WL_REG, intr_en & 0x1);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
