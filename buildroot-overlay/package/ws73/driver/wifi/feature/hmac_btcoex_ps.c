/*
 * Copyright: Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: PS处理
 * Date: 2023-01-28 15:24
 */

#ifndef __HMAC_BTCOEX_PS_C__
#define __HMAC_BTCOEX_PS_C__

#include "hmac_scan.h"
#include "hmac_fcs.h"
#include "msg_btcoex_rom.h"
#include "hmac_btcoex.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"
#include "hmac_btcoex_ps.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_BTCOEX_PS_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/*****************************************************************************
 函 数 名  : hmac_btcoex_encap_preempt_frame
 功能描述  : 根据指定的PS位封null data帧
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_encap_preempt_frame(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user)
{
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    osal_u32 qosnull_seq_num = 0;

    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_btcoex_encap_preempt_frame:: HAL_DEVICE null}", hmac_vap->vap_id);
        return;
    }

    /* 初始化dmac vap对应的preempt字段，包括写给硬件的premmpt帧地址和premmpt帧的参数 */
    (osal_void)memset_s(&(hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param),
        sizeof(hmac_vap_btcoex_null_preempt_stru), 0, sizeof(hmac_vap_btcoex_null_preempt_stru));

    switch (hmac_btcoex_get_vap_info(hmac_vap)->all_abort_preempt_type) {
        case HAL_BTCOEX_HW_POWSAVE_NOFRAME:
        case HAL_BTCOEX_HW_POWSAVE_SELFCTS:
            break;

        case HAL_BTCOEX_HW_POWSAVE_NULLDATA: {
            /* 填写帧头,其中from ds为1，to ds为0，ps=1 因此frame control的第二个字节为12 */
            mac_ieee80211_frame_stru *mac_header = (mac_ieee80211_frame_stru *) \
                hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.null_qosnull_frame;
            mac_null_data_encap(hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.null_qosnull_frame,
                WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_NODATA | 0x1100,
                hmac_user->user_mac_addr,
                mac_mib_get_station_id(hmac_vap));

            /* 22表示seq num位 */
            hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.null_qosnull_frame[22] = 0;
            /* 23表示frag位 */
            hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.null_qosnull_frame[23] = 0;
            mac_header->frame_control.power_mgmt = 1;
            break;
        }

        case HAL_BTCOEX_HW_POWSAVE_QOSNULL: {
            hmac_btcoex_qosnull_frame_stru *mac_header = (hmac_btcoex_qosnull_frame_stru *) \
                hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.null_qosnull_frame;
            mac_null_data_encap(hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.null_qosnull_frame,
                WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_QOS_NULL |
                    0x1100, hmac_user->user_mac_addr,
                mac_mib_get_station_id(hmac_vap));

            mac_header->frame_control.power_mgmt = 1;
            mac_header->qc_tid = WLAN_TIDNO_COEX_QOSNULL;
            mac_header->qc_eosp = 0;

            /* 设置seq的序列号 */
            hal_get_btcoex_abort_qos_null_seq_num(hal_device, &qosnull_seq_num);
            mac_header->sc_seq_num = (osal_u16)(qosnull_seq_num + 1);
            hal_set_btcoex_abort_qos_null_seq_num(hal_device, mac_header->sc_seq_num);

            /* 协议规定单播的QOS NULL DATA只允许normal ack 共存里面要设置0是对方会回ack */
            mac_header->qc_ack_polocy = WLAN_TX_NORMAL_ACK;
            break;
        }

        default:
            oam_error_log2(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_encap_preempt_frame::sw_preempt_type[%d] error}",
                hmac_vap->vap_id, hmac_btcoex_get_vap_info(hmac_vap)->all_abort_preempt_type);
            return;
    }

    oam_warning_log2(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_encap_preempt_frame::sw_preempt_type[%d]!}",
        hmac_vap->vap_id, hmac_btcoex_get_vap_info(hmac_vap)->all_abort_preempt_type);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_set_preempt_frame_param
 功能描述  : 配置preempt帧的参数类型，注意pst_mac_user要保证不为空
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_set_preempt_frame_param(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user)
{
    osal_u16 preempt_param_val = 0;

    switch (hmac_btcoex_get_vap_info(hmac_vap)->all_abort_preempt_type) {
        case HAL_BTCOEX_HW_POWSAVE_NOFRAME:
            /* 增加接口通用性，NOFRAME直接返回，写对应寄存器类型为0即可 */
            return;

        case HAL_BTCOEX_HW_POWSAVE_SELFCTS:
            break;

        case HAL_BTCOEX_HW_POWSAVE_NULLDATA:
            hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.cfg_coex_tx_peer_index = hmac_user->lut_index;

            /* null配置peer idx */
            preempt_param_val = preempt_param_val |
                                hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.cfg_coex_tx_peer_index;
            break;

        case HAL_BTCOEX_HW_POWSAVE_QOSNULL:
            hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.cfg_coex_tx_peer_index =  hmac_user->lut_index;
            hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.cfg_coex_tx_qos_null_tid =
                WLAN_TIDNO_COEX_QOSNULL;

            /* qos null需要配置tid */
            preempt_param_val = preempt_param_val | /* 8表示左移8位以取cfg_coex_tx_qos_null_tidde的低8位 */
                (hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.cfg_coex_tx_qos_null_tid << 8);
            /* qos null配置peer idx */
            preempt_param_val = preempt_param_val |
                                hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.cfg_coex_tx_peer_index;
            break;

        default:
            oam_error_log2(0, OAM_SF_COEX,
                "vap_id[%d] {hmac_btcoex_set_preempt_frame_param::sw_preempt_type[%d] error}", hmac_vap->vap_id,
                hmac_btcoex_get_vap_info(hmac_vap)->all_abort_preempt_type);
            return;
    }

    /* 03的self cts的时候也需要vap index; 02看软件开哪些vap里从0~4找，最先找到的vap的mac地址作为self cts的地址 */
    hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.cfg_coex_tx_vap_index = hmac_vap->hal_vap->vap_id;

    /* 三类帧都需要配置vap index， self cts的时候也需要vap index(pilot才会做，软件先写好，后续注意验证) */
    preempt_param_val = preempt_param_val |
        (hmac_btcoex_get_vap_info(hmac_vap)->null_preempt_param.cfg_coex_tx_vap_index << 12); /* 12表示左移12位 */

    hal_set_btcoex_abort_preempt_frame_param(preempt_param_val);

    oam_warning_log3(0, OAM_SF_COEX,
        "vap_id[%d] {hmac_btcoex_set_preempt_frame_param::SUCC sw_preempt_type[%d] preempt_param_val[%d]!}",
        hmac_vap->vap_id, hmac_btcoex_get_vap_info(hmac_vap)->all_abort_preempt_type, preempt_param_val);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_init_preempt
 功能描述  : preempt机制初始化
             1.单VAP的话， AP和STA三种帧都可以发送，ap和sta都默认self-cts, 后续根据调试情况来调整
             2.只要多个vap的都用self cts，sta和p2p gc同存也要用self cts，
                因为发null帧只能让一个ap不给我们发帧；
*****************************************************************************/
osal_void hmac_btcoex_init_preempt(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user)
{
    hmac_vap_btcoex_stru *hmac_vap_btcoex = hmac_btcoex_get_vap_info(hmac_vap);
    osal_s32 ret;

    if (hmac_vap_btcoex == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_btcoex_init_preempt::hmac_vap_btcoex is null.");
        return;
    }

    /* 初始化为不发preempt帧 */
    hmac_vap_btcoex->all_abort_preempt_type = HAL_BTCOEX_HW_POWSAVE_NOFRAME;

    /* encap preempt帧 */
    hmac_btcoex_encap_preempt_frame(hmac_vap, hmac_user);
    /* 配置premmpt帧的参数 */
    hmac_btcoex_set_preempt_frame_param(hmac_vap, hmac_user);
    /* 写入null qos null帧地址 */
    if ((hmac_vap_btcoex->all_abort_preempt_type == HAL_BTCOEX_HW_POWSAVE_NULLDATA) ||
        (hmac_vap_btcoex->all_abort_preempt_type == HAL_BTCOEX_HW_POWSAVE_QOSNULL)) {
        frw_msg msg_info = {0};
        btcoex_null_preempt_stru btcoex_null_preempt;

        (osal_void)memcpy_s(btcoex_null_preempt.null_qosnull_frame, WLAN_COEX_PKT_LEN,
            hmac_vap_btcoex->null_preempt_param.null_qosnull_frame, WLAN_COEX_PKT_LEN);
        btcoex_null_preempt.cfg_coex_tx_vap_index = hmac_vap_btcoex->null_preempt_param.cfg_coex_tx_vap_index;
        btcoex_null_preempt.cfg_coex_tx_qos_null_tid = hmac_vap_btcoex->null_preempt_param.cfg_coex_tx_qos_null_tid;
        btcoex_null_preempt.cfg_coex_tx_peer_index = hmac_vap_btcoex->null_preempt_param.cfg_coex_tx_peer_index;

        frw_msg_init((osal_u8 *)&btcoex_null_preempt, sizeof(btcoex_null_preempt_stru), OSAL_NULL, 0, &msg_info);
        ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_SET_BTCOEX_ABORT_NULL, &msg_info, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_CFG, "hmac_btcoex_init_preempt:send btcoex abort null to device fail[%d]", ret);
        }
    }

    /* 6. 配置芯片premmpt类型 */
    hal_set_btcoex_tx_abort_preempt_type(hmac_vap_btcoex->all_abort_preempt_type);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_update_ps_frame_param
 功能描述  : 更新ps相关参数
*****************************************************************************/
static osal_void hmac_btcoex_update_ps_frame_param(hal_to_dmac_device_stru *hal_device,
    hal_chip_stru *hal_chip, mac_btcoex_ps_frame_info_stru *ps_param)
{
    unref_param(hal_chip);

    if (hal_device->btcoex_sw_preempt.coex_pri_forbit == OSAL_FALSE) {
        ps_param->protect_coex_pri = hal_device->btcoex_sw_preempt.protect_coex_pri;
    } else {
        ps_param->protect_coex_pri = HAL_FCS_PROTECT_COEX_PRI_NORMAL;
    }

    /* 这里等效fcs_service_type条件判断，这个变量耦合太多，建议删除 */
    /* 非self-CTS duration参数也会统一设置但实际不会使用 */
    if (hal_btcoex_btble_status()->bt_status.bt_status.bt_ldac == OSAL_TRUE) {
        ps_param->duration = MAC_FCS_CTS_MAX_BTCOEX_LDAC_DURATION;
        ps_param->timeout = MAC_FCS_DEFAULT_PROTECT_TIME_OUT;
        ps_param->wait_timeout = MAC_ONE_PACKET_TIME_OUT;
    } else {
        ps_param->duration = MAC_FCS_CTS_MAX_BTCOEX_NOR_DURATION;
        ps_param->timeout = MAC_FCS_DEFAULT_PROTECT_TIME_OUT3;
        ps_param->wait_timeout = MAC_ONE_PACKET_TIME_OUT3;
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_set_ps_flag
 功能描述  : BTCOEX同步ps状态标志给device侧
*****************************************************************************/
osal_void hmac_btcoex_set_ps_flag(hal_to_dmac_device_stru *hal_device)
{
    osal_s32 ret;
    mac_btcoex_ps_flag_info_stru ps_flags = {0};
    frw_msg msg = {0};

    if (hal_device == OSAL_NULL) {
        return;
    }

    if ((hal_device->btcoex_sw_preempt.ps_stop == OSAL_TRUE) ||
        (hal_device->btcoex_sw_preempt.sw_preempt_mode.ps_on == OSAL_FALSE) ||
        (hal_device->btcoex_sw_preempt.ps_pause == OSAL_TRUE)) {
        ps_flags.one_pkt_en = OSAL_FALSE;
    } else {
        ps_flags.one_pkt_en = OSAL_TRUE;
    }

    ps_flags.dis_abort = OSAL_TRUE;
    ps_flags.reply_cts = hal_device->btcoex_sw_preempt.sw_preempt_mode.reply_cts;
    ps_flags.rsp_frame_ps = hal_device->btcoex_sw_preempt.sw_preempt_mode.rsp_frame_ps;

    oam_info_log3(0, OAM_SF_COEX, "{hmac_btcoex_set_ps_flag:: enable=%d, cts=%d, rsp_ps=%d}",
        ps_flags.one_pkt_en, ps_flags.reply_cts, ps_flags.rsp_frame_ps);

    frw_msg_init((osal_u8 *)&ps_flags, sizeof(ps_flags), OSAL_NULL, 0, &msg);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_SET_BTCOEX_PS_FLAG, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_set_ps_flag send msg fail, ret=%d}", ret);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_et_vap_ps_frame
 功能描述  : BTCOEX同步ps frame内容给device侧
*****************************************************************************/
osal_void hmac_btcoex_set_vap_ps_frame(hmac_vap_stru *hmac_vap, osal_u32 enable)
{
    osal_s32 ret;
    mac_btcoex_ps_frame_info_stru ps_frame = {0};
    frw_msg msg = {0};
    hal_one_packet_cfg_stru one_packet_cfg_tmp = {0};
    hmac_user_stru *hmac_user = OSAL_NULL;
    hal_chip_stru *hal_chip = hal_get_chip_stru();
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    unref_param(enable);
    if ((hmac_vap == OSAL_NULL) || (hmac_vap->hal_vap == OSAL_NULL) ||
        (hal_chip == OSAL_NULL) || (hmac_vap->hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_COEX, "{hmac_btcoex_set_vap_ps_frame:: null pointer}");
        return;
    }

    hal_device = hmac_vap->hal_device;

    /* user_nums只能表示接入。最好判断设备已经接入并完成认证 */
    ps_frame.is_vap_param = OAL_TRUE;
    ps_frame.is_valid = OAL_TRUE;
    if (!is_sta(hmac_vap) || (hmac_vap->user_nums == 0)) {
        ps_frame.is_valid = OAL_FALSE;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OSAL_NULL) {
        ps_frame.is_valid = OAL_FALSE;
    }

    if (ps_frame.is_valid == OAL_TRUE) {
        ps_frame.protect_cnt     = mac_fcs_get_protect_cnt(hmac_vap);
        ps_frame.cfg_one_pkt_tx_vap_index = hmac_vap->hal_vap->vap_id;
        ps_frame.cfg_one_pkt_tx_peer_index = hmac_user->lut_index;

        /* data_rate和tx_mode信息最终方案需要由算法模块提供 */
        ps_frame.tx_data_rate    = hmac_fcs_set_prot_datarate(hmac_vap);
        one_packet_cfg_tmp.tx_data_rate = ps_frame.tx_data_rate;
        ps_frame.tx_mode         = hmac_fcs_get_prot_mode(hmac_vap, &one_packet_cfg_tmp);

        if (hmac_btcoex_get_vap_info(hmac_vap)->ps_type == HAL_FCS_PROTECT_TYPE_SELF_CTS) {
            ps_frame.protect_type = HAL_FCS_PROTECT_TYPE_SELF_CTS;
        } else {
            ps_frame.protect_type = HAL_FCS_PROTECT_TYPE_NULL_DATA;
        }
    }

    // device相关的ps frame参数
    ps_frame.is_dev_param = OAL_TRUE;
    hmac_btcoex_update_ps_frame_param(hal_device, hal_chip, &ps_frame);

    oam_info_log3(0, OAM_SF_COEX, "{hmac_btcoex_set_vap_ps_frame:: valid=%d, vap_id=%d, lut_index=%d}",
        ps_frame.is_valid, ps_frame.cfg_one_pkt_tx_vap_index, ps_frame.cfg_one_pkt_tx_peer_index);

    frw_msg_init((osal_u8 *)&ps_frame, sizeof(ps_frame), OSAL_NULL, 0, &msg);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_SET_BTCOEX_PS_FRAME, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_set_vap_ps_frame send msg fail, ret=%d}", ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_set_dev_ps_frame
 功能描述  : BTCOEX同步ps frame内容给device侧
*****************************************************************************/
osal_void hmac_btcoex_set_dev_ps_frame(hal_to_dmac_device_stru *hal_device)
{
    osal_s32 ret;
    hal_chip_stru *hal_chip = hal_get_chip_stru();
    mac_btcoex_ps_frame_info_stru ps_frame = {0};
    frw_msg msg = {0};

    // device相关的ps frame参数
    ps_frame.is_dev_param = OAL_TRUE;
    hmac_btcoex_update_ps_frame_param(hal_device, hal_chip, &ps_frame);

    oam_info_log2(0, OAM_SF_COEX, "{hmac_btcoex_set_dev_ps_frame:: coex_pri=%d duration=%d}",
        ps_frame.protect_coex_pri, ps_frame.duration);

    frw_msg_init((osal_u8 *)&ps_frame, sizeof(ps_frame), OSAL_NULL, 0, &msg);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_SET_BTCOEX_PS_FRAME, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_set_dev_ps_frame send msg fail, ret=%d}",
            ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_switch_ps
 功能描述  : BTCOEX通知device进行ps状态切换
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_switch_ps(hal_to_dmac_device_stru *hal_device,
    hmac_btcoex_ps_switch_enum_uint8 status)
{
    osal_s32 ret;
    frw_msg msg = {0};

    unref_param(hal_device);
    frw_msg_init((osal_u8 *)&status, sizeof(status), OSAL_NULL, 0, &msg);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_BTCOEX_SIWTCH_PS, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_COEX, "{hmac_btcoex_switch_ps send msg fail, status=%u ret=%d}",
            status, ret);
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_preempt_sub_type_notify
 功能描述  : 外部触发，切换sw preempt sub type
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_btcoex_set_sw_preempt_type(hal_to_dmac_device_stru *hal_device,
    hal_coex_sw_preempt_subtype_uint8 type)
{
    hal_device->btcoex_sw_preempt.sw_preempt_type = type;

    if ((type == HAL_BTCOEX_SW_POWSAVE_SCAN_BEGIN) ||
        (type == HAL_BTCOEX_SW_POWSAVE_SCAN_WAIT) ||
        (type == HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT) ||
        (type == HAL_BTCOEX_SW_POWSAVE_SCAN_END)) {
        /* 通知device侧ps状态被打断 */
        hmac_btcoex_switch_ps(hal_device, HMAC_BTCOEX_PS_ABORT);
    }

    return OSAL_SUCCESS;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_ps_timeout_update_time
 功能描述  : ps机制更新timeout超时时间，音乐数传，page inquiry下超时时间配置需要不同
*****************************************************************************/
osal_void hmac_btcoex_ps_timeout_update_time(hal_to_dmac_device_stru *hal_device)
{
    hal_btcoex_ps_status_enum_uint8 ps_status = HAL_BTCOEX_PS_STATUE_BUTT;

    /* 获取当前ps业务状态 */
    hal_btcoex_get_ps_service_status(hal_device, &ps_status);

    switch (ps_status) {
        case HAL_BTCOEX_PS_STATUE_ACL:
            hal_device->btcoex_sw_preempt.timeout_ms = BTCOEX_POWSAVE_TIMEOUT_LEVEL0;
            break;

        case HAL_BTCOEX_PS_STATUE_LDAC:
        case HAL_BTCOEX_PS_STATUE_LDAC_ACL:
        case HAL_BTCOEX_PS_STATUE_PAGE_INQ:
            hal_device->btcoex_sw_preempt.timeout_ms = BTCOEX_POWSAVE_TIMEOUT_LEVEL1;
            break;

        case HAL_BTCOEX_PS_STATUE_PAGE_ACL:
        case HAL_BTCOEX_PS_STATUE_LDAC_PAGE:
        case HAL_BTCOEX_PS_STATUE_TRIPLE:
            hal_device->btcoex_sw_preempt.timeout_ms = BTCOEX_POWSAVE_TIMEOUT_LEVEL2;
            break;

        default:
            oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_ps_timeout_update_time::ps_status[%d] error.}",
                ps_status);
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_ps_stop_check_and_notify
 功能描述  : 获取ps是否禁止标记
             (1)dbac启动和停止 (2)单ap模式 (3)sco电话启动和结束 (4)5G工作
             其中单ap模式，5g工作逻辑在关联和去关联时候也要单独判断
*****************************************************************************/
osal_void hmac_btcoex_ps_stop_check_and_notify(hal_to_dmac_device_stru *hal_device)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    oal_bool_enum_uint8 sco_status = OSAL_FALSE;
    oal_bool_enum_uint8 ps_stop = OSAL_FALSE; /* 初始是打开ps */
    osal_u8 vap_idx, up_vap_num, up_ap_num, up_sta_num, up_2g_num, up_5g_num;
    osal_u8             mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    up_ap_num = up_sta_num = up_2g_num = up_5g_num = 0;

    if (hal_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_ps_stop_check_and_notify: hal_device is null ptr.}");
        return;
    }

    /* 1.电话场景 */
    hal_btcoex_get_bt_sco_status(hal_device, &sco_status);
    if ((sco_status == OSAL_TRUE) || (mac_is_dbac_running(hmac_device) == OSAL_TRUE)) {
        ps_stop = OSAL_TRUE;
    }

    /* Hal Device处于work状态vap个数 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_idx = 0; vap_idx < up_vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            continue;
        }

        /* 关联用户数不为0 */
        if (hmac_vap->user_nums != 0) {
            (is_ap(hmac_vap)) ? (up_ap_num++) : (up_sta_num++);
            (mac_btcoex_check_valid_vap(hmac_vap) == OSAL_TRUE) ? (up_2g_num++) : (up_5g_num++);
        }
    }

    /* 3.单ap模式 4.5g模式 5.siso c1 */
    if ((up_ap_num != 0 && up_sta_num == 0) || (up_5g_num != 0 && up_2g_num == 0) ||
        ((hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ONE) &&
        (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO))) {
        ps_stop = OSAL_TRUE;
    }

    /* 刷新ps能力 */
    hal_device->btcoex_sw_preempt.ps_stop = ps_stop;
    hmac_btcoex_set_ps_flag(hal_device);

    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_PS_STOP, (osal_u32)ps_stop);

    hal_coex_sw_irq_set(HAL_COEX_SW_IRQ_BT);

    oam_warning_log_alter(0, OAM_SF_COEX,
        "{hmac_btcoex_ps_stop_check_and_notify::ap[%d]sta[%d]5g[%d]2g[%d]sco[%d]dbac[%d]chain[%d]m2s[%d] ps_stop[%d]!}",
        /* 9代表9后面有9个待打印参数 */
        9, up_ap_num, up_sta_num, up_5g_num, up_2g_num, sco_status,
        mac_is_dbac_running(hmac_device), hal_device->cfg_cap_info->phy_chain,
        hal_device->hal_m2s_fsm.oal_fsm.cur_state,
        hal_device->btcoex_sw_preempt.ps_stop);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_ps_pause_check_and_notify
 功能描述  : 获取ps是否暂停标记，单wifi侧
             (1)漫游过程中需要暂停
*****************************************************************************/
OSAL_STATIC osal_bool hmac_btcoex_ps_pause_check_and_notify(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    oal_bool_enum_uint8 ps_pause = OSAL_FALSE; /* 初始是不暂停ps */
    hal_to_dmac_device_stru *hal_device = ((hmac_vap_stru *)notify_data)->hal_device;
    osal_u8 vap_idx;
    osal_u8 up_vap_num;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    /* Hal Device处于work状态vap个数 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_idx = 0; vap_idx < up_vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            continue;
        }

        /* 1.处于漫游过程中需要暂停ps */
        if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
            ps_pause = OSAL_TRUE;
        }
    }

    /* 刷新ps能力 */
    hal_device->btcoex_sw_preempt.ps_pause = ps_pause;
    hmac_btcoex_set_ps_flag(hal_device);

    oam_warning_log2(0, OAM_SF_COEX,
        "{hmac_btcoex_ps_pause_check_and_notify::ps_pause[%d]up_vap_num[%d]!}",
        hal_device->btcoex_sw_preempt.ps_pause, up_vap_num);
    return OSAL_TRUE;
}

OSAL_STATIC osal_void hmac_btcoex_pow_save_scan_state(hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *h2d_device)
{
    switch (h2d_device->btcoex_sw_preempt.sw_preempt_type) {
        case HAL_BTCOEX_SW_POWSAVE_SCAN_BEGIN:
            /* 如果扫描一开始被ps打断，此时ps来恢复继续扫描 */
            hmac_scan_begin(hmac_device, h2d_device);
            break;

        case HAL_BTCOEX_SW_POWSAVE_SCAN_WAIT:
            /* 如果扫描临时回home channel继续工作被打断，此时ps来恢复继续回home channel工作 */
            hmac_scan_switch_home_channel_work(hmac_device, h2d_device);
            oam_warning_log0(0, OAM_SF_COEX,
                "{hmac_btcoex_pow_save_scan_state::hmac_scan_switch_home_channel_work start.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_SCAN_END:
            /* 如果扫描结束被打断，此时ps来恢复继续扫描 */
            oam_warning_log0(0, OAM_SF_COEX,
                "{hmac_btcoex_pow_save_scan_state::hmac_scan_handle_switch_channel_back start.}");
            hmac_scan_prepare_end(hmac_device, h2d_device);
            break;

        case HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT:
            /* abort状态一定是，在save状态时候scan baort，类似于ps=0要恢复ps=1的配置，因为scan
             * abort提前resume了，此处不需要处理  */
            oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_pow_save_scan_state::scan is already abort and resume.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_IDLE:
            /* 可能是状态1事件没有及时处理下半部，又来了状态0，此时处于扫描，恢复交给扫描来做即可 */
            oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_pow_save_scan_state::HAL_BTCOEX_SW_POWSAVE_IDLE.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_WORK:
            /* 如果扫描期间来了1又来了0，此时恢复交给扫描自己恢复,ps=0不做处理 */
            break;

        default:
            oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_pow_save_scan_state::sw_preempt_type[%d] error.}",
                h2d_device->btcoex_sw_preempt.sw_preempt_type);
    }

    return;
}

OSAL_STATIC osal_void hmac_btcoex_pow_save_other_state(hal_to_dmac_device_stru *h2d_device)
{
    switch (h2d_device->btcoex_sw_preempt.sw_preempt_type) {
        case HAL_BTCOEX_SW_POWSAVE_PSM_END:
            /* 先执行了低功耗恢复，不要做操作 */
            oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_pow_save_other_state:: powerdown already resume.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_IDLE:
            /* 可能是低功耗ps=1事件才开始执行，此时读取寄存器状态=0，当前是0状态，就按照0来处理即可,
             * 会连续来两个0的事件处理
             */
            oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_pow_save_other_state:: current is normal!.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT:
            /* 强制scan abort恢复了，此处不需要恢复 */
            oam_warning_log0(0, OAM_SF_COEX,
                "{hmac_btcoex_pow_save_other_state:: work state scan abort already resume.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_WORK:
            /* 恢复发送和接收 */
            if (h2d_device->btcoex_sw_preempt.sw_preempt_subtype == HAL_BTCOEX_SW_POWSAVE_SUB_IDLE) {
                h2d_device->btcoex_sw_preempt.sw_preempt_subtype = HAL_BTCOEX_SW_POWSAVE_SUB_ACTIVE;
                oam_warning_log0(0, OAM_SF_COEX,
                    "{hmac_btcoex_pow_save_other_state:: work HAL_BTCOEX_SW_POWSAVE_SUB_IDLE.}");
            } else if (h2d_device->btcoex_sw_preempt.sw_preempt_subtype == HAL_BTCOEX_SW_POWSAVE_SUB_SCAN) {
                h2d_device->btcoex_sw_preempt.sw_preempt_subtype = HAL_BTCOEX_SW_POWSAVE_SUB_ACTIVE;
                oam_warning_log0(0, OAM_SF_COEX,
                    "{hmac_btcoex_pow_save_other_state:: work HAL_BTCOEX_SW_POWSAVE_SUB_SCAN.}");
            }
            break;

        default:
            oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_pow_save_other_state::sw_preempt_type[%d] error.}",
                h2d_device->btcoex_sw_preempt.sw_preempt_type);
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_pow_save_callback
 功能描述  : 软件preempt机制下恢复节能定时处理函数
*****************************************************************************/
osal_u32 hmac_btcoex_pow_save_callback(osal_void *arg)
{
    hal_to_dmac_device_stru *h2d_device = (hal_to_dmac_device_stru *)arg;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (h2d_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_pow_save_callback::h2d_device is null.}");
        return OAL_FAIL;
    }
    /* 在scan状态时候，扫描不进入低功耗，此时不用担心低功耗，如果扫描来时 */
    if (h2d_device->hal_dev_fsm.oal_fsm.cur_state == HAL_DEVICE_SCAN_STATE) {
        hmac_btcoex_pow_save_scan_state(hmac_device, h2d_device);
    } else {
        hmac_btcoex_pow_save_other_state(h2d_device);
    }
    hmac_btcoex_switch_ps(h2d_device, HMAC_BTCOEX_PS_TIMOUT); /* 通知device侧处理ps超时 */
    /* preempt机制置为NONE形式 */
    h2d_device->btcoex_sw_preempt.sw_preempt_type = HAL_BTCOEX_SW_POWSAVE_TIMEOUT;
    h2d_device->btcoex_sw_preempt.ps_timeout_cnt++;

    oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_pow_save_callback::time is up.}");

    return OAL_SUCC;
}

osal_void hmac_btcoex_ps_on_proc(hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *h2d_device, hal_chip_stru *hal_chip)
{
    unref_param(hmac_device);
    unref_param(hal_chip);

    /* preempt机制置为软件形式 */
    h2d_device->btcoex_sw_preempt.sw_preempt_type = HAL_BTCOEX_SW_POWSAVE_WORK;

    /* 在scan状态时候 */
    if (h2d_device->hal_dev_fsm.oal_fsm.cur_state == HAL_DEVICE_SCAN_STATE) {
        /* 在扫描执行过程中，不需要处理，扫描时对端已经处于节能状态并在tx
         pause状态，扫描结束时需要考虑是不是由ps来恢复，低功耗是200ms，
         即使扫描由ps来恢复，也来得及，ps等于0肯定在低功耗前来到或者恢复之后，不会被低功耗打断
        */
        h2d_device->btcoex_sw_preempt.sw_preempt_subtype = HAL_BTCOEX_SW_POWSAVE_SUB_SCAN;
    } else {
        /* sta模式可能一开始还没关联上,处于非up状态,后续启动成up状态,ps=0时候恢复不生效,
         * 直接判断不处理即可,如果处于低功耗状态(主要是work状态下的awake子状态,
         * 收到ps中断, 仍然对端在睡眠状态),wifi不需要特殊处理,执行pause不需要发ps帧,
         * 等低功耗自己来处理ps状态.sta vap进行发送pause,并通知对端缓存数据暂停发送数据,
         * 处于idle状态,空发一帧也没关系 */
        /* 1.先默认为active状态，能保证低功耗饿死时，中断ps=1也能唤醒 */
        h2d_device->btcoex_sw_preempt.sw_preempt_subtype = HAL_BTCOEX_SW_POWSAVE_SUB_ACTIVE;
    }

    /* 状态变迁,启动定时器完成ps操作，防止扫描置状态之后，出现wifi不醒来 */
    if (h2d_device->btcoex_powersave_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(h2d_device->btcoex_powersave_timer));
    }

    /* ps机制启动时，需要根据当前状态，刷新超时定时器时间 */
    hmac_btcoex_ps_timeout_update_time(h2d_device);
    frw_create_timer_entry(&(h2d_device->btcoex_powersave_timer), hmac_btcoex_pow_save_callback,
        h2d_device->btcoex_sw_preempt.timeout_ms, (osal_void *)h2d_device, OSAL_FALSE);
}

osal_void hmac_btcoex_ps_off_scan_state_proc(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *h2d_device)
{
    switch (h2d_device->btcoex_sw_preempt.sw_preempt_type) {
        case HAL_BTCOEX_SW_POWSAVE_SCAN_BEGIN:
            /* 如果扫描一开始被ps打断，此时ps来恢复继续扫描 */
            hmac_scan_begin(hmac_device, h2d_device);
            break;

        case HAL_BTCOEX_SW_POWSAVE_SCAN_WAIT:
            /* 如果扫描临时回home channel继续工作被打断，此时ps来恢复继续回home channel工作 */
            hmac_scan_switch_home_channel_work(hmac_device, h2d_device);
            oam_warning_log0(0, OAM_SF_COEX,
                "{hmac_btcoex_ps_off_scan_state_proc::hmac_scan_switch_home_channel_work start.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_SCAN_END:
            /* 如果扫描结束被打断，此时ps来恢复继续扫描 */
            oam_warning_log0(0, OAM_SF_COEX,
                "{hmac_btcoex_ps_off_scan_state_proc::hmac_scan_handle_switch_channel_back start.}");
            hmac_scan_prepare_end(hmac_device, h2d_device);
            break;

        case HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT:
            /* abort状态一定是，在save状态时候scan baort，类似于ps=0要恢复ps=1的配置，因为scan
             * abort提前resume了，此处不需要处理  */
            oam_warning_log0(0, OAM_SF_COEX,
                "{hmac_btcoex_ps_off_scan_state_proc::scan is already abort and resume.}");

            /* abort已经强制置为恢复，此次不保护，vap已经vap了不需要做操作 */
            /* 恢复发送和接收 */
            break;

        case HAL_BTCOEX_SW_POWSAVE_IDLE:
            /* 可能是状态1事件没有及时处理下半部，又来了状态0，此时处于扫描，恢复交给扫描来做即可 */
            oam_warning_log0(0, OAM_SF_COEX,
                "{hmac_btcoex_ps_off_scan_state_proc::HAL_BTCOEX_SW_POWSAVE_IDLE.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_WORK:
            /* 如果扫描期间来了1又来了0，此时恢复交给扫描自己恢复,ps=0不做处理 */
            break;

        case HAL_BTCOEX_SW_POWSAVE_TIMEOUT:
            /* time is up， 或者低功耗已经提前恢复，属于正常 */
            break;

        default:
            oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_ps_off_scan_state_proc::sw_preempt_type[%d] error.}",
                h2d_device->btcoex_sw_preempt.sw_preempt_type);
    }
}

osal_void hmac_btcoex_ps_off_no_scan_state_proc(hal_to_dmac_device_stru *h2d_device)
{
    switch (h2d_device->btcoex_sw_preempt.sw_preempt_type) {
        case HAL_BTCOEX_SW_POWSAVE_PSM_END:
            /* 先执行了低功耗恢复，不要做操作 */
            oam_warning_log0(0, OAM_SF_COEX,
                "{hmac_btcoex_ps_off_no_scan_state_proc:: powerdown has already resume.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_IDLE:
            /* 可能是低功耗ps=1事件才开始执行，此时读取寄存器状态=0，
               当前是0状态，就按照0来处理即可，会连续来两个0的事件处理 */
            oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_ps_off_no_scan_state_proc:: current is normal!.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT:
            /* 强制scan abort恢复了，此处不需要恢复 */
            oam_warning_log0(0, OAM_SF_COEX,
                "{hmac_btcoex_ps_off_no_scan_state_proc:: work state scan abort already resume.}");
            break;

        case HAL_BTCOEX_SW_POWSAVE_WORK:
            /* 恢复发送和接收 */
            if (h2d_device->btcoex_sw_preempt.sw_preempt_subtype == HAL_BTCOEX_SW_POWSAVE_SUB_IDLE) {
                h2d_device->btcoex_sw_preempt.sw_preempt_subtype = HAL_BTCOEX_SW_POWSAVE_SUB_ACTIVE;
                oam_warning_log0(0, OAM_SF_COEX,
                    "{hmac_btcoex_ps_off_no_scan_state_proc:: work HAL_BTCOEX_SW_POWSAVE_SUB_IDLE.}");
            } else if (h2d_device->btcoex_sw_preempt.sw_preempt_subtype == HAL_BTCOEX_SW_POWSAVE_SUB_SCAN) {
                h2d_device->btcoex_sw_preempt.sw_preempt_subtype = HAL_BTCOEX_SW_POWSAVE_SUB_ACTIVE;
                oam_warning_log0(0, OAM_SF_COEX,
                    "{hmac_btcoex_ps_off_no_scan_state_proc:: work HAL_BTCOEX_SW_POWSAVE_SUB_SCAN.}");
            }
            break;

        case HAL_BTCOEX_SW_POWSAVE_TIMEOUT:
            /* time is up， 或者低功耗已经提前恢复，属于正常 */
            break;

        default:
            oam_warning_log1(0, OAM_SF_COEX,
                "{hmac_btcoex_ps_off_no_scan_state_proc::sw_preempt_type[%d] error.}",
                h2d_device->btcoex_sw_preempt.sw_preempt_type);
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_one_pkt_type_and_duration_update
 功能描述  : one pkt帧类型刷新，兼容性设备只能识别self-cts帧
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_one_pkt_type_and_duration_update(const mac_fcs_mgr_stru *fcs_mgr,
    hal_one_packet_cfg_stru *one_packet_cfg, hmac_vap_stru *hmac_vap)
{
    /* 针对sta模式下才刷新帧类型 */
    if (!is_sta(hmac_vap)) {
        return;
    }

    /* 如果是非null帧类型，也即是self-cts类型，需要刷新duration */
    if (hmac_btcoex_get_vap_info(hmac_vap)->ps_type == HAL_FCS_PROTECT_TYPE_SELF_CTS) {
        /* 刷新帧类型和duration时间 */
        one_packet_cfg->protect_type = HAL_FCS_PROTECT_TYPE_SELF_CTS;

        if (fcs_mgr->fcs_service_type == HAL_FCS_SERVICE_TYPE_BTCOEX_LDAC) {
            one_packet_cfg->duration = MAC_FCS_CTS_MAX_BTCOEX_LDAC_DURATION;
        } else {
            one_packet_cfg->duration = MAC_FCS_CTS_MAX_BTCOEX_NOR_DURATION;
        }
    }
}

osal_u32 hmac_btcoex_ps_init(osal_void)
{
    /* notify注册 */
    frw_util_notifier_register_with_priority(WLAN_UTIL_NOTIFIER_EVENT_ROAM_ACTIVE,
        hmac_btcoex_ps_pause_check_and_notify, WLAN_UTIL_NOTIFIER_PRIORITY_MAX);
    frw_util_notifier_register_with_priority(WLAN_UTIL_NOTIFIER_EVENT_ROAM_DEACTIVE,
        hmac_btcoex_ps_pause_check_and_notify, WLAN_UTIL_NOTIFIER_PRIORITY_MAX);
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_PS_SET_SW_PREEMPT_TYPE, hmac_btcoex_set_sw_preempt_type);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_PS_ONE_PKT_TYPE_UPDATE, hmac_btcoex_one_pkt_type_and_duration_update);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_PS_STOP_CHECK, hmac_btcoex_ps_stop_check_and_notify);
    return OAL_SUCC;
}

osal_void hmac_btcoex_ps_deinit(osal_void)
{
    /* notify去注册 */
    frw_util_notifier_unregister_with_priority(WLAN_UTIL_NOTIFIER_EVENT_ROAM_ACTIVE,
        hmac_btcoex_ps_pause_check_and_notify, WLAN_UTIL_NOTIFIER_PRIORITY_MAX);
    frw_util_notifier_unregister_with_priority(WLAN_UTIL_NOTIFIER_EVENT_ROAM_DEACTIVE,
        hmac_btcoex_ps_pause_check_and_notify, WLAN_UTIL_NOTIFIER_PRIORITY_MAX);
    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_PS_SET_SW_PREEMPT_TYPE);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_PS_ONE_PKT_TYPE_UPDATE);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_PS_STOP_CHECK);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
