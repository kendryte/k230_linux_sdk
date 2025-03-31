/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * 文 件 名   : hmac_smps.c
 * 生成日期   :  2014年4月17日
 * 功能描述   : SMPS hmac接口
 */

#ifdef _PRE_WLAN_FEATURE_SMPS

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_smps.h"
#include "hmac_config.h"
#include "mac_ie.h"
#ifdef _PRE_WLAN_FEATURE_M2S
#include "hmac_m2s.h"
#endif
#include "hmac_feature_dft.h"
#include "hmac_alg_notify.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_SMPS_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述 : 更新smps信息
*****************************************************************************/
osal_u32 hmac_smps_update_user_status(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    wlan_mib_mimo_power_save_enum_uint8 user_smps_mode;
    osal_u8 old_avail_num_spatial_stream;

    if ((hmac_vap == OSAL_NULL) || (hmac_user == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_SMPS,
            "hmac_smps_update_user_status: NULL PTR hmac_vap is [%p] and mac_user is [%p].",
            (uintptr_t)hmac_vap, (uintptr_t)hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    user_smps_mode = (wlan_mib_mimo_power_save_enum_uint8)hmac_user->ht_hdl.sm_power_save;
    old_avail_num_spatial_stream = hmac_user->avail_num_spatial_stream;

    switch (user_smps_mode) {
        case WLAN_MIB_MIMO_POWER_SAVE_STATIC:
            hmac_user_set_sm_power_save(hmac_user, WLAN_MIB_MIMO_POWER_SAVE_STATIC);
            hmac_user_set_avail_num_spatial_stream_etc(hmac_user, WLAN_SINGLE_NSS);
            break;
        case WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC:
            oam_warning_log1(0, OAM_SF_SMPS, "vap_id[%d] {hmac_smps_update_user_status:smps_mode update DYNAMIC!}",
                hmac_vap->vap_id);
            hmac_user_set_sm_power_save(hmac_user, WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC);
            hmac_user_set_avail_num_spatial_stream_etc(hmac_user, osal_min(hmac_vap->vap_rx_nss, WLAN_DOUBLE_NSS));
            break;
        case WLAN_MIB_MIMO_POWER_SAVE_MIMO:
            hmac_user_set_sm_power_save(hmac_user, WLAN_MIB_MIMO_POWER_SAVE_MIMO);
            hmac_user_set_avail_num_spatial_stream_etc(hmac_user, osal_min(hmac_vap->vap_rx_nss, WLAN_DOUBLE_NSS));
            break;
        default:
            oam_warning_log2(0, OAM_SF_SMPS,
                "vap_id[%d] {hmac_smps_update_user_status: user_smps_mode mode[%d] fail!}",
                hmac_vap->vap_id, user_smps_mode);
            return OAL_FAIL;
    }

    if (old_avail_num_spatial_stream != hmac_user->avail_num_spatial_stream) {
        /* 空间流更新 则同步至device */
        hmac_user_sync(hmac_user);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_smps_set_mac_mode
 功能描述  : 转换为MAC的smps mode
*****************************************************************************/
osal_u8 hmac_smps_get_hal_mode(osal_u8 dev_smps_mode)
{
    hal_smps_mode_enum_uint8 hal_smps_mode = HAL_SMPS_MODE_DYNAMIC;

    if (dev_smps_mode == WLAN_MIB_MIMO_POWER_SAVE_MIMO) {
        hal_smps_mode = HAL_SMPS_MODE_DISABLE;
    } else if (dev_smps_mode == WLAN_MIB_MIMO_POWER_SAVE_STATIC) {
        hal_smps_mode = HAL_SMPS_MODE_STATIC;
    }

    return hal_smps_mode;
}

/*****************************************************************************
 函 数 名  : hmac_smps_encap_action
 功能描述  : dmac组action帧
*****************************************************************************/
osal_void hmac_smps_encap_action(const hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    osal_u16 *len, wlan_mib_mimo_power_save_enum_uint8  smps_mode, oal_bool_enum_uint8 bool_code)
{
    osal_u16 index;
    mac_ieee80211_frame_stru *mac_header = OSAL_NULL;
    mac_smps_action_mgt_stru *payload_addr = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL || netbuf == OSAL_NULL || len == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_smps_encap_action::param null.}");
        return;
    }

    mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);
    payload_addr = (mac_smps_action_mgt_stru *)oal_netbuf_data_offset(netbuf, MAC_80211_FRAME_LEN);

    *len = 0;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /* 设置 Frame Control field */
    mac_hdr_set_frame_control((osal_u8 *)mac_header,
        WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /*  power management bit is never sent by an AP */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_header->frame_control.power_mgmt = bool_code;
    }

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number((osal_u8 *)mac_header, 0);

    /* 设置 address1(接收端): AP MAC地址 (BSSID) */
    oal_set_mac_addr((osal_u8 *)mac_header + WLAN_HDR_ADDR1_OFFSET, hmac_vap->bssid);

    /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr((osal_u8 *)mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr((osal_u8 *)mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    /*************************************************************************/
    /*                  SMPS Management frame - Frame Body                   */
    /* ---------------------------------------------------------------------- */
    /* |Category |HT Action |SMPS Control field|                             */
    /* ---------------------------------------------------------------------- */
    /* |1        |1         |1                 |                             */
    /* ---------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    index = MAC_80211_FRAME_LEN;

    payload_addr->category = MAC_ACTION_CATEGORY_HT;            /* HT Category */
    payload_addr->action = MAC_HT_ACTION_SMPS;               /* HT Action */

    index += MAC_IE_HDR_LEN;

    /* SMPS Control field */
    /* Bit0 --- enable/disable smps   Bit1 --- SMPS MODE */
    payload_addr->sm_ctl = 0;                                /* Element ID */

    /* Bit0始终为1表示enable SMPS; Bit1取MAC smps MODE */
    if (smps_mode == WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC) {
        payload_addr->sm_ctl = BIT1 | BIT0;
    } else if (smps_mode == WLAN_MIB_MIMO_POWER_SAVE_STATIC) {
        payload_addr->sm_ctl = BIT0;
    }

    index++;

    *len = index;

    return;
}

/*****************************************************************************
 函 数 名  : hmac_smps_send_action
 功能描述  : 发送action帧
*****************************************************************************/
osal_void hmac_smps_send_action(hmac_vap_stru *hmac_vap,
    wlan_mib_mimo_power_save_enum_uint8  smps_mode, oal_bool_enum_uint8 bool_code)
{
    oal_netbuf_stru                      *mgmt_buf = OSAL_NULL;
    osal_u16                            mgmt_len;
    mac_tx_ctl_stru                      *tx_ctl;
    osal_u32                            ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_smps_send_action::param null.}");
        return;
    }

    /* 申请管理帧内存 */
    mgmt_buf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (mgmt_buf == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SMPS, "vap_id[%d] {hmac_smps_send_action::mgmt_buf null.}", hmac_vap->vap_id);
        return;
    }

    oal_set_netbuf_prev(mgmt_buf, OSAL_NULL);
    oal_set_netbuf_next(mgmt_buf, OSAL_NULL);

    hmac_smps_encap_action(hmac_vap, mgmt_buf, &mgmt_len, smps_mode, bool_code);
    if (mgmt_len == 0) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        oam_error_log1(0, OAM_SF_SMPS, "vap_id[%d] {hmac_smps_send_action::mgmt_len is zero.}", hmac_vap->vap_id);
        oal_netbuf_free(mgmt_buf);
        return;
    }

    /* 调用发送管理帧接口 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    tx_ctl->tx_user_idx    = hmac_vap->assoc_vap_id;
    tx_ctl->ac             = WLAN_WME_AC_MGMT;
    tx_ctl->frame_type     = WLAN_CB_FRAME_TYPE_ACTION;
    tx_ctl->frame_subtype  = WLAN_ACTION_SMPS_FRAME_SUBTYPE;

    ret = hmac_tx_mgmt(hmac_vap, mgmt_buf, mgmt_len, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, 0, "vap_id[%d] {hmac_smps_send_action::hmac_tx_mgmt failed[%d].",
            hmac_vap->vap_id, ret);
        oal_netbuf_free(mgmt_buf);
        return;
    }
}

/*****************************************************************************
 函 数 名  : hmac_check_smps_field
 功能描述  :解析帧中SMPS能力位
*****************************************************************************/
osal_u32 hmac_check_smps_field(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u32 msg_len,
    hmac_user_stru *hmac_user)
{
    osal_u8 *ie = OSAL_NULL;
    wlan_mib_mimo_power_save_enum_uint8 smps;

    if (osal_unlikely(hmac_vap == OSAL_NULL || payload == OSAL_NULL || hmac_user == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_check_smps_field::param null.}");
        return OAL_FAIL;
    }

    ie = mac_find_ie_etc(MAC_EID_HT_CAP, payload, (osal_s32)msg_len);
    if (ie == OSAL_NULL) {
        return OAL_FAIL;
    }
    /* 2表示右移2位 */
    smps = hmac_ie_proc_sm_power_save_field_etc(hmac_user, (ie[MAC_IE_HDR_LEN] & (BIT3 | BIT2)) >> 2);
    if (smps != hmac_user->ht_hdl.sm_power_save) {
        hmac_user->ht_hdl.sm_power_save = smps;
        hmac_smps_update_user_status(hmac_vap, hmac_user);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_rx_smps_frame
 功能描述  : 从空口接收smps action帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_rx_smps_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, const osal_u8 *frame_payload)
{
    wlan_mib_mimo_power_save_enum_uint8     user_smps_mode;

    if (osal_unlikely(hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL || frame_payload == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_mgmt_rx_smps_frame::param null.}");
        return OAL_FAIL;
    }

    if (is_vap_single_nss(hmac_vap) || is_user_single_nss(hmac_user)) {
        return OAL_SUCC;
    }

    /* 更新STA的sm_power_save field, 获取enable bit */
    if ((frame_payload[MAC_ACTION_OFFSET_ACTION + 1] & BIT0) == 0) {
        user_smps_mode = WLAN_MIB_MIMO_POWER_SAVE_MIMO;
    } else {
        /* 如果SMPS enable,则配置为相应模式(不考虑动态状态更新，动态只支持配置命令配置) */
        if ((frame_payload[MAC_ACTION_OFFSET_ACTION + 1] & BIT1) == 0) {
            /* 静态SMPS */
            user_smps_mode = WLAN_MIB_MIMO_POWER_SAVE_STATIC;
        } else { /* 动态SMPS */
            user_smps_mode = WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC;
        }
    }

    oam_warning_log4(0, OAM_SF_SMPS, "vap_id[%d] {hmac_mgmt_rx_smps_frame::user[%d] smps mode[%d] change to[%d]!}",
        hmac_vap->vap_id, hmac_user->assoc_id, hmac_user->ht_hdl.sm_power_save, user_smps_mode);

    /* 用户更新的smps能力不能超过本vap的能力 */
    if (user_smps_mode > mac_mib_get_smps(hmac_vap)) {
        oam_warning_log4(0, OAM_SF_SMPS,
            "vap_id[%d] {hmac_mgmt_rx_smps_frame::user[%d] new smps mode[%d] > vap smps mode[%d]!}", hmac_vap->vap_id,
            hmac_user->assoc_id, user_smps_mode, mac_mib_get_smps(hmac_vap));
        return OAL_FAIL;
    }

    /* 如果user的SMPS状态发生改变，需要做user和vap状态更新 */
    if (user_smps_mode != hmac_user->ht_hdl.sm_power_save) {
        /* 更新user的smps能力 */
        hmac_user->ht_hdl.sm_power_save = user_smps_mode;

        hmac_smps_update_user_status(hmac_vap, hmac_user);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_get_smps_mode
 功能描述  : 设置获取user的smps模式,这里是AP与STA协商出来支持的最大能力;
             如果AP不支持,STA支持,则取STA能力; AP支持，STA不支持, STA则不支持
*****************************************************************************/
osal_u8 hmac_user_get_smps_mode(const hmac_vap_stru  *hmac_vap, const hmac_user_stru *hmac_user)
{
    wlan_mib_mimo_power_save_enum_uint8 vap_smps;
    wlan_mib_mimo_power_save_enum_uint8 user_smps;

    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        return WLAN_MIB_MIMO_POWER_SAVE_MIMO;
    }

    if (hmac_user->ht_hdl.ht_capable != OSAL_TRUE) {
        return WLAN_MIB_MIMO_POWER_SAVE_MIMO;
    }

    vap_smps  = mac_mib_get_smps(hmac_vap);
    user_smps = (wlan_mib_mimo_power_save_enum_uint8)hmac_user->ht_hdl.sm_power_save;

    return (osal_u8)((vap_smps >= user_smps) ? user_smps : vap_smps);
}

/*****************************************************************************
 函 数 名  :  hmac_config_set_smps_mode
 功能描述  : 设置SMPS模式
*****************************************************************************/
osal_s32 hmac_config_set_smps_mode(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    struct osal_list_head *entry;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_vap_stru *hmac_vap_tmp = OAL_PTR_NULL;
    wlan_mib_mimo_power_save_enum_uint8 smps_mode;
    wlan_nss_enum_uint8 spatial_stream_num;
    osal_u8 vap_idx, old_avail_num_spatial_stream;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    hal_smps_mode_enum_uint8 hal_smps_mode;
    osal_u32 ret;

    smps_mode = *((wlan_mib_mimo_power_save_enum_uint8 *)msg->data);
    if ((smps_mode < WLAN_MIB_MIMO_POWER_SAVE_STATIC) || (smps_mode > WLAN_MIB_MIMO_POWER_SAVE_MIMO)) {
        oam_warning_log1(0, OAM_SF_SMPS, "{hmac_config_set_smps_mode::smps_mode[%d] beyond scope.}", smps_mode);
        return OAL_FAIL;
    }

    /* 获取device,hmac_vap为配置vap */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SMPS, "{hmac_config_set_smps_mode::hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 确认device是否可以配置新的smps mode */
    ret = hmac_device_find_smps_mode_en(hmac_device, smps_mode);
    if (ret != OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_SMPS, "{hmac_config_set_smps_mode::device not support changing smps mode.}");
        return OAL_FAIL;
    }

    /* 根据SMPS mode确认采用单流还是双流来发送，通知算法 */
    spatial_stream_num = (smps_mode == WLAN_MIB_MIMO_POWER_SAVE_STATIC) ? WLAN_SINGLE_NSS : WLAN_DOUBLE_NSS;

    /* 遍历device下所有业务vap，刷新所有vap的SMPS能力 */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_tmp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap_tmp == OAL_PTR_NULL ||
            mac_mib_get_high_throughput_option_implemented(hmac_vap_tmp) != OAL_TRUE) {
            continue;
        }

        /* device能力变化，需要修改vap的空间流能力 */
        /* 改变vap下空间流个数能力(给新接入user使用) */
        hmac_vap_set_rx_nss_etc(hmac_vap_tmp, spatial_stream_num);

        hmac_vap_tmp->hal_device->cfg_cap_info->nss_num = spatial_stream_num;

        /* 改变hal_device下空间流个数，防止关联时m2s改变smps能力位 */
        ((dmac_vap_stru *)mac_vap)->hal_device->cfg_cap_info.nss_num = avail_nss;

        /* 设置mib项 */
        mac_mib_set_smps(hmac_vap_tmp, smps_mode);

        /* 遍历vap下所有user，发送SMPS action帧 */
        osal_list_for_each(entry, &(hmac_vap->mac_user_list_head)) {
            hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
            /* 非ht模式用户不发action帧 */
            if (hmac_user->cur_protocol_mode != WLAN_HT_MODE && hmac_user->cur_protocol_mode != WLAN_HT_ONLY_MODE &&
                hmac_user->cur_protocol_mode != WLAN_HT_11G_MODE) {
                continue;
            }
            old_avail_num_spatial_stream = hmac_user->avail_num_spatial_stream;
            /* 刷新user smps和空间流能力 */
            hmac_user_set_sm_power_save(hmac_user, smps_mode);
            hmac_user_set_avail_num_spatial_stream_etc(hmac_user, spatial_stream_num);

            if (hmac_vap_tmp->vap_mode == WLAN_VAP_MODE_BSS_STA) {
                hmac_smps_send_action(hmac_vap, smps_mode, OSAL_FALSE);
                /* 如何保证对端收到了此帧，可能需要起一个定时器，多发几帧，作为优化点 */
            }
            if (old_avail_num_spatial_stream != hmac_user->avail_num_spatial_stream) {
                /* 空间流更新 则同步至device */
                hmac_user_sync(hmac_user);
            }
        }
    }
    /* 配置device SMPS软件能力 */
    hmac_device->mac_smps_mode = smps_mode;

    /* 获取需要切换的SMPS mode */
    hal_smps_mode = hmac_smps_get_hal_mode(hmac_device->mac_smps_mode);
    hal_set_smps_mode(hal_smps_mode);

    oam_warning_log1(0, OAM_SF_SMPS, "{hmac_config_set_smps_mode::mode%d.}", hmac_device->mac_smps_mode);

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
