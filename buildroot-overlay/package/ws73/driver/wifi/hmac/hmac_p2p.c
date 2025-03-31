/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: P2P 相关特性处理.
 */

#ifdef _PRE_WLAN_FEATURE_P2P

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_p2p.h"
#include "oal_util.h"
#include "hmac_ext_if.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "hmac_rx_filter.h"
#include "hmac_fsm.h"
#include "hmac_mgmt_bss_comm.h"
#include "mac_frame.h"
#include "hmac_user.h"
#include "hmac_mgmt_ap.h"
#include "wlan_msg.h"
#include "hmac_scan.h"
#include "hmac_feature_dft.h"
#include "oal_netbuf_data.h"
#include "hmac_tx_mgmt.h"
#include "hmac_psm_ap.h"
#include "msg_p2p_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_P2P_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 静态函数声明
*****************************************************************************/


/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
p2p_common_info_stru g_p2p_common_info = {0};

/*****************************************************************************
  4 函数实现
*****************************************************************************/
typedef struct mac_vap_state_priority {
    osal_u8 priority;
    osal_u8 vap_state;
} mac_vap_state_priority_stru;

typedef struct hmac_input_req_priority {
    osal_u8 priority;
    osal_u8 input_req;
} hmac_input_req_priority_stru;

mac_vap_state_priority_stru g_mac_vap_state_priority_table_etc[MAC_VAP_STATE_BUTT] = {
    {0, MAC_VAP_STATE_INIT},
    {2, MAC_VAP_STATE_UP},                   /* VAP UP */
    {0, MAC_VAP_STATE_PAUSE},                /* pause , for ap &sta */
    {0, MAC_VAP_STATE_LISTEN},               /* p2p0 listen */
    /* ap 独有状态 */
    {0, MAC_VAP_STATE_AP_WAIT_START},
    /* sta独有状态 */
    {0, MAC_VAP_STATE_STA_FAKE_UP},
    {10, MAC_VAP_STATE_STA_WAIT_SCAN},
    {0, MAC_VAP_STATE_STA_SCAN_COMP},
    {10, MAC_VAP_STATE_STA_JOIN_COMP},
    {10, MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2},
    {10, MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4},
    {10, MAC_VAP_STATE_STA_AUTH_COMP},
    {10, MAC_VAP_STATE_STA_WAIT_ASOC},
    {10, MAC_VAP_STATE_STA_OBSS_SCAN},
    {10, MAC_VAP_STATE_STA_BG_SCAN},
    {10, MAC_VAP_STATE_ROAMING} // MAC_VAP_STATE_ROAMING /* 漫游 */
};

hmac_input_req_priority_stru g_mac_fsm_input_type_priority_table_etc[HMAC_FSM_INPUT_TYPE_BUTT] = {
    {0, HMAC_FSM_INPUT_RX_MGMT},
    {0, HMAC_FSM_INPUT_RX_DATA},
    {0, HMAC_FSM_INPUT_TX_DATA},
    {0, HMAC_FSM_INPUT_TIMER0_OUT},
    {0, HMAC_FSM_INPUT_MISC},
    {0, HMAC_FSM_INPUT_START_REQ},
    {5, HMAC_FSM_INPUT_SCAN_REQ},
    {5, HMAC_FSM_INPUT_JOIN_REQ},
    {5, HMAC_FSM_INPUT_AUTH_REQ},
    {5, HMAC_FSM_INPUT_ASOC_REQ},
    {5, HMAC_FSM_INPUT_LISTEN_REQ}, /* P2P listen */
    {0, HMAC_FSM_INPUT_LISTEN_TIMEOUT} /* P2P listen超时 */
};

osal_u8 hmac_get_p2p_protocol_info(osal_void)
{
    return g_p2p_common_info.protocol_mode;
}

osal_u8 hmac_get_p2p_common_band_info(osal_void)
{
    return g_p2p_common_info.band_width;
}

osal_u8 hmac_get_p2p_common_enable_info(osal_void)
{
    return g_p2p_common_info.enable;
}

#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32 hmac_set_p2p_common_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    p2p_common_info_stru *p2p_common_info = (p2p_common_info_stru *)(msg->data);
    unref_param(hmac_vap);

    g_p2p_common_info.protocol_mode = p2p_common_info->protocol_mode;
    g_p2p_common_info.band_width = p2p_common_info->band_width;
    g_p2p_common_info.enable = p2p_common_info->enable;

    wifi_printf("hmac_set_p2p_common_info protocol[%d] band[%d] enable[%d]\r\n",
        g_p2p_common_info.protocol_mode, g_p2p_common_info.band_width, g_p2p_common_info.enable);
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述 : 获取Vap的P2P模式
*****************************************************************************/
wlan_p2p_mode_enum_uint8  hmac_get_p2p_mode_etc(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->p2p_mode);
}

/*****************************************************************************
 功能描述 : 删除P2P vap num的功能函数
*****************************************************************************/
void hmac_dec_p2p_num_etc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_p2p_dec_num::device[%d] null.}", hmac_vap->device_id);
        return;
    }

    if (is_p2p_mode(hmac_vap->p2p_mode)) {
        hmac_device->st_p2p_info.p2p_mode = 0;
    }
}

/*****************************************************************************
 功能描述 : add p2p vap时同步增加p2p设备的计数器
*****************************************************************************/
void hmac_inc_p2p_num_etc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_inc_p2p_num::dev[%d] null.}", hmac_vap->device_id);
        return;
    }
    if (is_p2p_mode(hmac_vap->p2p_mode)) {
        hmac_device->st_p2p_info.p2p_mode = hmac_vap->p2p_mode;
    }
}

/*****************************************************************************
 函 数 名  : hmac_p2p_check_can_enter_state_etc
 功能描述  : 检查外部输入事件是否允许执行
 输入参数  : hmac_vap_stru *hmac_vap
             hmac_fsm_input_type_enum_uint8 input_req
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_p2p_check_can_enter_state_etc(hmac_vap_stru *hmac_vap,
    hmac_fsm_input_type_enum_uint8 input_req)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_vap_stru *other_vap = OAL_PTR_NULL;
    osal_u8 vap_num, vap_idx, vap_state_priority = 0, input_priority = 0, i;

    /* 2.1 检查其他vap 状态，判断输入事件优先级是否比vap 状态优先级高
    *  如果输入事件优先级高，则可以执行输入事件 */
    hmac_device  = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_p2p_check_can_enter_state_etc::hmac_res_get_mac_dev_etc fail.device_id = %u}", hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    vap_num = hmac_device->vap_num;

    for (vap_idx = 0; vap_idx < vap_num; vap_idx++) {
        other_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (OAL_PTR_NULL == other_vap) {
            oam_warning_log1(0, OAM_SF_CFG,
                "{hmac_p2p_check_can_enter_state_etc::hmac_res_get_hmac_vap fail.vap_idx = %u}",
                hmac_device->vap_id[vap_idx]);
            continue;
        }

        if (other_vap->vap_id == hmac_vap->vap_id) {
            /* 如果检测到是自己，则继续检查其他VAP 状态 */
            continue;
        }

        /* 当GO或者AP在进行关联时，不进行扫描业务 */
        for (i = 0; i < OAL_ARRAY_SIZE(g_mac_vap_state_priority_table_etc); i++) {
            if (g_mac_vap_state_priority_table_etc[i].vap_state == other_vap->vap_state) {
                vap_state_priority = g_mac_vap_state_priority_table_etc[i].priority;
                break;
            }
        }

        if (i == OAL_ARRAY_SIZE(g_mac_vap_state_priority_table_etc)) {
            continue;
        }

        for (i = 0; i < OAL_ARRAY_SIZE(g_mac_fsm_input_type_priority_table_etc); i++) {
            if (g_mac_fsm_input_type_priority_table_etc[i].input_req == input_req) {
                input_priority = g_mac_fsm_input_type_priority_table_etc[i].priority;
                break;
            }
        }

        if (i == OAL_ARRAY_SIZE(g_mac_fsm_input_type_priority_table_etc)) {
            continue;
        }

        if ((vap_state_priority > input_priority) && (hmac_go_is_auth(other_vap))) {
            oam_warning_log2(0, OAM_SF_CFG, "vap[%d] state is [%d], can not scan.", vap_idx, other_vap->vap_state);
            return OAL_ERR_CODE_CONFIG_BUSY;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_p2p_send_listen_expired_to_host_etc
 功能描述  : 监听超时处理,通知WAL 监听超时
*****************************************************************************/
osal_u32 hmac_p2p_send_listen_expired_to_host_etc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    oal_wireless_dev_stru *wdev = OAL_PTR_NULL;
    mac_p2p_info_stru *p2p_info = OAL_PTR_NULL;
    hmac_p2p_listen_expired_stru p2p_listen_expired;
    osal_u32 ret;
    frw_msg msg_info = {0};

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_P2P, "vap_id[%d] {hmac_send_mgmt_to_host_etc::hmac_device null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    p2p_info = &hmac_device->st_p2p_info;
    if (hmac_vap->net_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{hmac_p2p_send_listen_expired_to_host_etc::net_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wdev = hmac_vap->net_device->ieee80211_ptr;

    p2p_listen_expired.st_listen_channel = p2p_info->st_listen_channel;
    p2p_listen_expired.wdev          = wdev;

    msg_info.data = (osal_u8*)&p2p_listen_expired;
    msg_info.data_len = OAL_SIZEOF(hmac_p2p_listen_expired_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_LISTEN_EXPIRED, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_p2p_send_listen_expired_to_device_etc
 功能描述  : 监听超时处理，通知DMAC 返回home 信道
 输入参数  : hmac_vap_stru *hmac_vap
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_p2p_send_listen_expired_to_device_etc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru                *hmac_device;
    osal_s32 ret1;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_P2P,
            "vap_id[%d] {hmac_p2p_send_listen_expired_to_device_etc::hmac_device null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_info_log1(0, OAM_SF_P2P,
        "vap_id[%d] {hmac_p2p_send_listen_expired_to_device_etc:: listen timeout!.}",
        hmac_vap->vap_id);

    ret1 = hmac_config_cancel_remain_on_channel(hmac_vap);
    /* 强制stop listen */
    if (OAL_TRUE == hmac_device->scan_mgmt.is_scanning) {
        hmac_device->scan_mgmt.is_scanning = OAL_FALSE;
    }

    if (OAL_UNLIKELY(OAL_SUCC != ret1)) {
        oam_warning_log2(0, OAM_SF_P2P,
            "vap_id[%d] {hmac_p2p_send_listen_expired_to_device_etc::hmac_config_cancel_remain_on_channel fail[%d].}",
            hmac_vap->vap_id, ret1);
        return ret1;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_disable_p2p_pm_etc
 功能描述  : 停止p2p noa,p2p oppps
 输入参数  : hmac_vap_stru *hmac_vap
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
osal_void hmac_disable_p2p_pm_etc(hmac_vap_stru *hmac_vap)
{
    mac_cfg_p2p_ops_param_stru      p2p_ops;
    mac_cfg_p2p_noa_param_stru      p2p_noa;
    osal_s32                        ret;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    memset_s(&p2p_noa, OAL_SIZEOF(p2p_noa), 0, OAL_SIZEOF(p2p_noa));

    msg_info.data = (osal_u8 *)&p2p_noa;
    msg_info.data_len = (osal_u16)OAL_SIZEOF(mac_cfg_p2p_noa_param_stru);
    ret = hmac_config_set_p2p_ps_noa_etc(hmac_vap, &msg_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_P2P,
            "vap_id[%d] {hmac_disable_p2p_pm_etc::hmac_config_set_p2p_ps_noa_etc disable p2p NoA fail.}",
            hmac_vap->vap_id);
    }
    memset_s(&p2p_ops, OAL_SIZEOF(p2p_ops), 0, OAL_SIZEOF(p2p_ops));

    msg_info.data = (osal_u8 *)&p2p_ops;
    msg_info.data_len = (osal_u16)OAL_SIZEOF(mac_cfg_p2p_ops_param_stru);
    ret = hmac_config_set_p2p_ps_ops_etc(hmac_vap, &msg_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_P2P,
            "vap_id[%d] {hmac_disable_p2p_pm_etc::hmac_config_set_p2p_ps_ops_etc disable p2p OppPS fail.}",
            hmac_vap->vap_id);
    }
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_presence_response
 功能描述  : 封装presence response帧
*****************************************************************************/
osal_u16  hmac_mgmt_encap_p2p_presence_rsp(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    const osal_u8 *ra, const osal_u8 *data)
{
    osal_u8        ie_len;
    osal_u8       *mac_hdr = oal_netbuf_header(netbuf);
    osal_u8       *payload_addr        = oal_netbuf_data_offset(netbuf, MAC_80211_FRAME_LEN);
    osal_u8       *payload_addr_origin = payload_addr;
    osal_u8        index;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    mac_hdr_set_frame_control(mac_hdr, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置地址1为发送presence request帧的STA */
    if (ra != OSAL_NULL) {
        oal_set_mac_addr(mac_hdr + WLAN_HDR_ADDR1_OFFSET, ra);
    }

    oal_set_mac_addr(mac_hdr + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));
    if (hmac_vap_is_in_p2p_listen()) {
        /* 设置地址3为bssid */
        oal_set_mac_addr(mac_hdr + WLAN_HDR_ADDR3_OFFSET, mac_mib_get_station_id(hmac_vap));
    } else {
        /* 设置地址3为bssid */
        oal_set_mac_addr(mac_hdr + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);
    }

    /* 设置分片序号, 管理帧为0 */
    mac_hdr_set_fragment_number(mac_hdr, 0);

    /*************************************************************************/
    /*                  presence response Frame - Frame Body                 */
    /* ---------------------------------------------------------------------- */
    /* |Category |OUI   |OUI type|OUI subtype  | Dialog Token   |Element    | */
    /* ---------------------------------------------------------------------- */
    /* |1        |3     |1       |1            |1               |variable   | */
    /* ---------------------------------------------------------------------- */
    /*************************************************************************/
    payload_addr[0] = MAC_ACTION_CATEGORY_VENDOR;
    index = P2P_GEN_ACT_OUI_OFF1;

    if (memcpy_s(&payload_addr[index], MAC_OUI_LEN, mac_frame_get_p2p_oui(), MAC_OUI_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_mgmt_encap_p2p_presence_rsp::memcpy_s error}");
    }
    index += MAC_OUI_LEN;

    /* OUI Type */
    payload_addr[index++] = MAC_OUITYPE_P2P;

    /* OUI Subtype */
    payload_addr[index++] = P2P_PRESENCE_RESP;

    /* Dialog Token */
    payload_addr[index++] = data[P2P_GEN_ACT_DIALOG_TOKEN_OFF];
    payload_addr += P2P_GEN_ACT_TAG_PARAM_OFF;

    /* Element */
    /* 填充p2p Status Attribute */
    mac_set_p2p_status(payload_addr, &ie_len, P2P_STAT_SUCCESS);
    payload_addr += ie_len;

    /* 填充p2p noa Attribute */
    if (is_p2p_ps_enabled(hmac_vap)) {
        mac_set_p2p_noa(hmac_vap, payload_addr, &ie_len);
        payload_addr += ie_len;
    } else {
        mac_set_p2p_none_noa(hmac_vap, payload_addr, &ie_len);
        payload_addr += ie_len;
    }

    return (osal_u16)((payload_addr - payload_addr_origin) + MAC_80211_FRAME_LEN);
}

/*****************************************************************************
 函 数 名  : hmac_process_p2p_presence_req
 功能描述  : 收到presence request处理
*****************************************************************************/
osal_u32 hmac_process_p2p_presence_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    oal_netbuf_stru *mgmt_buf = OSAL_NULL;
    osal_u16 mgmt_len;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 *frame_body = OSAL_NULL;
    osal_u32 ret;
    osal_u16 user_idx = 0;

    /* 获取帧头信息 */
    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)(mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info)));
    frame_body = oal_netbuf_rx_data(netbuf);

    ret = hmac_vap_find_user_by_macaddr_etc((hmac_vap), frame_hdr->address2, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_process_p2p_presence_req::no user.}", hmac_vap->vap_id);
        return ret;
    }

    /* 申请管理帧内存 */
    mgmt_buf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (mgmt_buf == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_P2P,
            "vap_id[%d] {hmac_process_p2p_presence_req::mgmt_buf null.}", hmac_vap->vap_id);

        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(mgmt_buf, OSAL_NULL);
    oal_set_netbuf_next(mgmt_buf, OSAL_NULL);

    /* 封装presence request帧 */
    mgmt_len = hmac_mgmt_encap_p2p_presence_rsp(hmac_vap, mgmt_buf, frame_hdr->address2, frame_body);
    oam_warning_log2(0, OAM_SF_P2P, "vap_id[%d] {hmac_process_p2p_presence_req::encap_p2p_presence_rsp. length=%d}",
        hmac_vap->vap_id, mgmt_len);

    /* 调用发送管理帧接口 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    tx_ctl->tx_user_idx = user_idx;
    oam_warning_log2(0, OAM_SF_P2P, "vap_id[%d] {hmac_process_p2p_presence_req::user_idx=%d}",
        hmac_vap->vap_id, user_idx);
    tx_ctl->ac = WLAN_WME_AC_MGMT;
    ret = hmac_tx_mgmt(hmac_vap, mgmt_buf, mgmt_len, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_P2P,
            "vap_id[%d] {hmac_process_p2p_presence_req::tx_mgmt failed[%d].", hmac_vap->vap_id, ret);
        oal_netbuf_free(mgmt_buf);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_p2p_listen_rx_mgmt
 功能描述  : P2P 设备接收到管理帧处理
*****************************************************************************/
osal_u8 hmac_p2p_listen_rx_mgmt(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 *frame_body = OSAL_NULL;
    osal_u16 frame_len;
    osal_u8 go_on = OSAL_TRUE;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (netbuf == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_P2P, "{hmac_p2p_listen_rx_mgmt::param null.}");
        return OSAL_FALSE;
    }

    /* 获取帧信息 */
    frame_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);
    frame_body = oal_netbuf_rx_data(netbuf);
    frame_len = (osal_u16)oal_netbuf_rx_frame_len(netbuf);

    if ((frame_hdr->frame_control.sub_type == WLAN_ACTION) && (hmac_is_p2p_action_frame(frame_body) == OSAL_TRUE)) {
        /* 判断是否是presence request action frame */
        if (hmac_is_p2p_presence_req_frame(frame_body) == OSAL_TRUE) {
            hmac_process_p2p_presence_req(hmac_vap, netbuf);
            return OSAL_FALSE;
        }
        /* 接收到p2p GO negotiation request 重设监听定时器 */
        /* 接收到provision discovery request帧则延长listen时间，避免action发送未完成即切信道 */
        if (hmac_is_p2p_go_neg_req_frame(frame_body) == OSAL_TRUE ||
            hmac_is_p2p_pd_disc_req_frame(frame_body) == OSAL_TRUE) {
            /* 延长监听时间，由于监听共用扫描接口，故延长扫描定时器 */
            if ((hmac_vap->hal_device != OSAL_NULL) &&
                (hmac_vap->hal_device->hal_scan_params.scan_timer.is_enabled == OSAL_TRUE) &&
                (hmac_vap->hal_device->hal_scan_params.scan_timer.is_registerd == OSAL_TRUE)) {
                frw_timer_stop_timer(&(hmac_vap->hal_device->hal_scan_params.scan_timer));
                frw_timer_restart_timer(&(hmac_vap->hal_device->hal_scan_params.scan_timer),
                    (hmac_vap->hal_device->hal_scan_params.scan_time), OSAL_FALSE);
            }
        }
        /* 如果是ACTION 帧，则上报 */
        return OSAL_TRUE;
    } else if (frame_hdr->frame_control.sub_type == WLAN_PROBE_REQ) {
        if (hmac_p2p_listen_filter_vap(hmac_vap) != OAL_SUCC) {
            return OSAL_FALSE;
        }
        if (hmac_p2p_listen_filter_frame(hmac_vap, frame_body, frame_len) != OAL_SUCC) {
            return OSAL_FALSE;
        }

        /* 接收到probe req 帧，返回probe response 帧 */
        hmac_ap_up_rx_probe_req(hmac_vap, netbuf, (osal_u8 *)OSAL_NULL, 0, &go_on);
    }

    return go_on;
}

OAL_STATIC osal_s32 hmac_sync_p2p_info(hmac_vap_stru *hmac_vap, mac_cfg_p2p_noa_param_stru *p2p_noa)
{
    frw_msg msg_info = {0};
    osal_s32 ret;

    if (hmac_vap == OSAL_NULL || hmac_vap->init_flag != MAC_VAP_VAILD) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_sync_p2p_info:: hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    frw_msg_init((osal_u8 *)p2p_noa, sizeof(mac_cfg_p2p_noa_param_stru), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_SYNC_P2P_INFO, &msg_info, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_sync_p2p_info::sync msg failed[%d].}", ret);
    }
    return ret;
}

osal_void hmac_process_p2p_noa_ext(hmac_vap_stru *hmac_vap, const mac_cfg_p2p_ops_param_stru *p2p_ops,
    mac_cfg_p2p_noa_param_stru *p2p_noa)
{
    hal_to_dmac_vap_stru *hal_vap = hmac_vap->hal_vap;

    /* 保存GO节能参数，设置P2P ops 寄存器 */
    if ((hmac_vap->p2p_ops_param.ops_ctrl != p2p_ops->ops_ctrl) ||
        (hmac_vap->p2p_ops_param.ct_window != p2p_ops->ct_window)) {
        hmac_vap->p2p_ops_param.ops_ctrl = p2p_ops->ops_ctrl;
        hmac_vap->p2p_ops_param.ct_window = p2p_ops->ct_window;
        oam_warning_log3(0, OAM_SF_P2P, "vap_id[%d] hmac_process_p2p_noa_ext:ctrl:%d, ct_window:%d",
            hmac_vap->vap_id, hmac_vap->p2p_ops_param.ops_ctrl, hmac_vap->p2p_ops_param.ct_window);

        /* 设置P2P ops 寄存器 */
        hal_vap_set_ops(hal_vap, hmac_vap->p2p_ops_param.ops_ctrl, hmac_vap->p2p_ops_param.ct_window);
        /* 将ops参数保存到dmac_vap中 */
        (osal_void)memcpy_s(&hmac_vap->p2p_ops_param, sizeof(hmac_vap->p2p_ops_param),
            p2p_ops, sizeof(mac_cfg_p2p_ops_param_stru));
        hmac_vap_sync(hmac_vap);
    }

    /* 保存GO节能参数，设置P2P NoA 寄存器 */
    if ((hmac_vap->p2p_noa_param.count != p2p_noa->count) || (hmac_vap->p2p_noa_param.duration != p2p_noa->duration) ||
        (hmac_vap->p2p_noa_param.interval != p2p_noa->interval) ||
        (hmac_vap->p2p_noa_param.start_time != p2p_noa->start_time)) {
        (osal_void)memcpy_s(&hmac_vap->p2p_noa_param, sizeof(hmac_vap->p2p_noa_param),
            p2p_noa, sizeof(mac_cfg_p2p_noa_param_stru));
        oam_warning_log4(0, OAM_SF_P2P, "hmac_process_p2p_noa_ext:start_time:%u, duration:%u, interval:%u, count:%u",
            p2p_noa->start_time, p2p_noa->duration, p2p_noa->interval, p2p_noa->count);
        hmac_sync_p2p_info(hmac_vap, p2p_noa);
    }
}

/*****************************************************************************
 函 数 名  : hmac_process_p2p_noa
 功能描述  : beacon帧时，是否包含p2p noa参数
             解析NoA节能参数
*****************************************************************************/
osal_void hmac_process_p2p_noa(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    mac_cfg_p2p_ops_param_stru p2p_ops;
    mac_cfg_p2p_noa_param_stru p2p_noa;
    osal_u16 attr_index = 0;
    osal_u16 attr_len = 0;
    dmac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
    mac_rx_ctl_stru *rx_info = OSAL_NULL;
    osal_u16 frame_len;
    osal_u8 *payload = OSAL_NULL;
    osal_u8 *noa_attr = OSAL_NULL;
    osal_u32 ret;

    if (hmac_vap == OSAL_NULL || netbuf == OSAL_NULL) {
        return;
    }
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    rx_info = (mac_rx_ctl_stru *)(&(rx_ctrl->rx_info));
    frame_len = rx_info->frame_len - rx_info->mac_header_len; /* 帧体长度 */
    payload = oal_netbuf_rx_data(netbuf);

    (osal_void)memset_s(&p2p_ops, sizeof(p2p_ops), 0, sizeof(p2p_ops));
    (osal_void)memset_s(&p2p_noa, sizeof(p2p_noa), 0, sizeof(p2p_noa));
    /* 取得NoA attr */
    noa_attr = hmac_get_p2p_noa_attr(payload, frame_len, MAC_BEACON_OFFSET, &attr_len);
    if (noa_attr == OSAL_NULL) {
        if (is_p2p_noa_enabled(hmac_vap) || is_p2p_oppps_enabled(hmac_vap)) {
            /* 停止节能，发消息到device通知，只有在之前打开过节能开关并且现在关闭的情况下才会下发该消息 */
            ret = hmac_p2p_event_sync2device(hmac_vap, P2P_PM_AWAKE_STATE);
            oam_warning_log2(0, OAM_SF_P2P,
                "vap_id[%d] {hmac_process_p2p_noa::puc_noa_attr null. stop p2p ps ret[%d]}", hmac_vap->vap_id, ret);
           /* 停止p2p noa oppps 时恢复深睡 平台低功耗移动到平台驱动管理 */
        } else {
            return;
        }
    } else {
        /* 解析ops参数 */
        if ((noa_attr[attr_index] & BIT7) != 0) {
            p2p_ops.ops_ctrl = 1;
            p2p_ops.ct_window = (noa_attr[attr_index] & 0x7F);
        }

        if (attr_len > 2) { /* 数据长度大于2解释数据内容 */
            /* 解析NoA参数 */
            attr_index++;
            p2p_noa.count = noa_attr[attr_index++];
            p2p_noa.duration = oal_make_word32(oal_make_word16(noa_attr[attr_index], noa_attr[attr_index + 1]),
                oal_make_word16(noa_attr[attr_index + 2], noa_attr[attr_index + 3])); /* 2 index和3 index */
            attr_index += 4; /* 加上4个字节长度的duration */
            p2p_noa.interval = oal_make_word32(oal_make_word16(noa_attr[attr_index], noa_attr[attr_index + 1]),
                oal_make_word16(noa_attr[attr_index + 2], noa_attr[attr_index + 3])); /* 2 index和3 index */
            attr_index += 4; /* 加上4个字节长度的interval */
            p2p_noa.start_time = oal_make_word32(oal_make_word16(noa_attr[attr_index], noa_attr[attr_index + 1]),
                oal_make_word16(noa_attr[attr_index + 2], noa_attr[attr_index + 3])); /* 2 index和3 index */
        }
    }

    hmac_process_p2p_noa_ext(hmac_vap, &p2p_ops, &p2p_noa);
}

osal_void hmac_p2p_oppps_ctwindow_start_event(hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        ret = hmac_p2p_event_sync2device(hmac_vap, P2P_PM_AWAKE_STATE);
        if (ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_P2P,
                "vap_id[%d] {hmac_p2p_oppps_ctwindow_start_event fail[%d]}", hmac_vap->vap_id, ret);
        }
    } else {
        /* 恢复发送 */
        hmac_p2p_handle_ps(hmac_vap, OSAL_FALSE);
    }
}

OAL_STATIC osal_void hmac_p2p_client_register(const hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device,
    hmac_user_stru *hmac_user)
{
    hmac_user_pause(hmac_user);
    /* 告知MAC用户进入节能模式 */
    hal_tx_enable_peer_sta_ps_ctrl(hmac_user->lut_index);
    hmac_device->mac_vap_id = hmac_vap->vap_id; /* 记录睡眠时vap id */

    return;
}

/*****************************************************************************
 函 数 名  : hmac_p2p_handle_ps
 功能描述  : 处理p2p节能状态
*****************************************************************************/
osal_void hmac_p2p_handle_ps(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 pause)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 mac_vap_id_old = hmac_device->mac_vap_id;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{hmac_p2p_handle_ps::hmac_vap null.}");
        return;
    }

    /* dbac在运行,直接return */
    if ((mac_is_dbac_running(hmac_device) == OSAL_TRUE)) {
        return;
    }

    if (hmac_device->st_p2p_info.p2p_ps_pause == pause) {
        return;
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
        if (hmac_user == OSAL_NULL) {
            oam_warning_log2(0, OAM_SF_P2P, "vap_id[%d] {hmac_p2p_handle_ps::hmac_user[%d] null.}", hmac_vap->vap_id,
                hmac_vap->assoc_vap_id);
            return;
        }

        /* P2P CLIENT 芯片节能针对P2P CLIENT 注册，而不能对P2P DEVICE 注册 */
        if (pause == OSAL_TRUE) {
            hmac_p2p_client_register(hmac_vap, hmac_device, hmac_user);
        } else {
            /* 恢复该用户的硬件队列的发送 */
            hal_tx_disable_peer_sta_ps_ctrl(hmac_user->lut_index);
            hmac_user_resume(hmac_user); /* 恢复user */
            /* 将所有的缓存帧发送出去 */
            hmac_psm_queue_flush(hmac_vap, hmac_user);
        }
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* P2P GO 芯片节能需要通过命令配置 */
        if (pause == OSAL_TRUE) {
            hmac_ap_pause_all_user(hmac_vap);
            hmac_device->mac_vap_id = hmac_vap->vap_id; /* 记录睡眠时vap id */
        } else {
            hmac_ap_resume_all_user(hmac_vap);
        }
    }

    /* 记录目前p2p节能状态 */
    hmac_device->st_p2p_info.p2p_ps_pause = pause;

    /* 如果休眠vap_id或节能状态发生变化，则同步 */
    if (mac_vap_id_old != hmac_device->mac_vap_id) {
        hmac_device_sync(hmac_device);
    }

    hmac_send_p2p_state_to_device(hmac_vap, pause, hmac_vap->vap_mode);
    return;
}

/* 检查是否只包含11B 速率，11b速率集:0x82 = 1Mbps, 0x84 = 2Mbps, 0x8B = 5.5Mbps, 0x96 = 11Mbps */
/*****************************************************************************
 函 数 名  : hmac_p2p_is_11b_rate
 功能描述  : 检查是否为11b 速率
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_11b_rate(osal_u8 rate)
{
    rate &= (osal_u8)(~BIT7);
    /* 2，4，11，22表示当rate等于这些值时，是11b的速率 */
    if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22)) {
        return OSAL_TRUE;
    } else {
        return OSAL_FALSE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_is_p2p_action_frame
 功能描述  : 过滤P2P action帧
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_p2p_action_frame(const osal_u8 *data)
{
    oal_bool_enum_uint8       ret;

    /* 获取帧体指针 */
    /* Category */
    switch (data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_PUBLIC: {
            /* Action */
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
                case MAC_PUB_VENDOR_SPECIFIC: {
                    ret =  OSAL_TRUE;
                }
                    break;
                default: {
                    ret =  OSAL_FALSE;
                }
                    break;
            }
        }
            break;

        case MAC_ACTION_CATEGORY_VENDOR: {
            ret =  OSAL_TRUE;
        }
            break;

        default: {
            ret =  OSAL_FALSE;
        }
            break;
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_is_p2p_pd_disc_req_frame
 功能描述  : 过滤P2P action帧
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_p2p_pd_disc_req_frame(const osal_u8 *data)
{
    if ((data[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_PUBLIC) &&
        (data[P2P_PUB_ACT_OUI_OFF1] == WFA_OUI_BYTE1) &&
        (data[P2P_PUB_ACT_OUI_OFF2] == WFA_OUI_BYTE2) &&
        (data[P2P_PUB_ACT_OUI_OFF3] == WFA_OUI_BYTE3) &&
        (WFA_P2P_V1_0 == data[P2P_PUB_ACT_OUI_TYPE_OFF]) &&
        (data[P2P_PUB_ACT_OUI_SUBTYPE_OFF] == P2P_PAF_PD_REQ)) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_p2p_is_only_11b_rates
 功能描述  : 检查是否只支持11b 速率
*****************************************************************************/
oal_bool_enum_uint8 hmac_p2p_is_only_11b_rates(osal_u8 *frame_body, osal_u16 frame_len)
{
    osal_u8      *ie = OSAL_NULL;
    osal_u8       ie_len;
    osal_u8       rate;
    osal_u32      loop;
    osal_u32      ie_index;
    osal_u8      *ie_array[2]; /* 2表示ie_array的长度 */

    /* 检查是否只包含11B 速率集，11b速率集:0x82 = 1Mbps, 0x84 = 2Mbps, 0x8B = 5.5Mbps, 0x96 = 11Mbps */
    ie_array[0] = mac_find_ie_etc(MAC_EID_RATES, frame_body, frame_len);
    ie_array[1] = mac_find_ie_etc(MAC_EID_XRATES, frame_body, frame_len);
    for (ie_index = 0; ie_index < oal_array_size(ie_array); ie_index++) {
        if (ie_array[ie_index] == OSAL_NULL) {
            continue;
        }

        ie = ie_array[ie_index];
        ie_len = *(ie + 1);

        for (loop = 0; loop < ie_len; loop++) {
            rate = *(ie + MAC_IE_HDR_LEN + loop);
            if (hmac_is_11b_rate(rate) == OSAL_FALSE) {
                return OSAL_FALSE;
            }
        }
    }
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_is_p2p_presence_req_frame
 功能描述  : 是否是P2P presence reqest action帧
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_p2p_presence_req_frame(const osal_u8 *data)
{
    if ((data[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_VENDOR) &&
        (data[P2P_GEN_ACT_OUI_SUBTYPE_OFF] == P2P_PRESENCE_REQ)) {
        return OSAL_TRUE;
    } else {
        return OSAL_FALSE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_p2p_listen_filter_frame
 功能描述  : 检查收到的probe request是不是要
*****************************************************************************/
osal_u32  hmac_p2p_listen_filter_frame(const hmac_vap_stru *hmac_vap, osal_u8 *frame_body, osal_u16 frame_len)
{
    osal_u8 *p2p_ie = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_p2p_listen_filter_frame:: hmac_vap is null!!!}");
    }
    /* 如果接收到的probe req 帧只包含11b 速率， 则返回 */
    if (hmac_p2p_is_only_11b_rates(frame_body, frame_len) == OSAL_TRUE) {
        return OAL_FAIL;
    }

    /* 如果接收到的probe req 帧不包含SSID "DIRECT-" ， 则返回 */
    if (!is_p2p_wildcard_ssid(&frame_body[MAC_IE_HDR_LEN], frame_body[1])) {
        return OAL_FAIL;
    }

    /* 如果接收到的probe req 帧不包含P2P_IE， 则返回 */
    p2p_ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, frame_body, frame_len);
    if (p2p_ie == OSAL_NULL) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_p2p_listen_filter_vap
 功能描述  : 检查vap状态是否要处理probe request
*****************************************************************************/
osal_u32  hmac_p2p_listen_filter_vap(const hmac_vap_stru *hmac_vap)
{
    /* 只有P2P 处于Listen状态且当前VAP信道与P2P0 Listen Channel一致的情况下才可以回复Probe resp帧 */
    if ((!is_legacy_vap(hmac_vap)) && hmac_vap_is_in_p2p_listen()
        && (hmac_vap->channel.chan_number == hmac_vap->p2p_listen_channel)) {
        return OAL_SUCC;
    }

    return OAL_FAIL;
}

/*****************************************************************************
 函 数 名  : hmac_is_p2p_go_neg_req_frame
 功能描述  : 是否是P2P GO negotiation request action帧
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_p2p_go_neg_req_frame(const osal_u8 *data)
{
    if ((data[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_PUBLIC) &&
        (data[P2P_PUB_ACT_OUI_OFF1] == WFA_OUI_BYTE1) &&
        (data[P2P_PUB_ACT_OUI_OFF2] == WFA_OUI_BYTE2) &&
        (data[P2P_PUB_ACT_OUI_OFF3] == WFA_OUI_BYTE3) &&
        (WFA_P2P_V1_0 == data[P2P_PUB_ACT_OUI_TYPE_OFF]) &&
        (data[P2P_PUB_ACT_OUI_SUBTYPE_OFF] == P2P_PAF_GON_REQ)) {
        return OSAL_TRUE;
    } else {
        return OSAL_FALSE;
    }
}
/*****************************************************************************
 函 数 名  : mac_set_p2p0_ssid_ie
 功能描述  : 设置p2p0的BSSID IE
*****************************************************************************/
osal_void  mac_set_p2p0_ssid_ie(const hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len, osal_u16 frm_type)
{
    osal_u8     ssid_len;

    if (is_legacy_vap(hmac_vap)) {
        return;
    }

    /***************************************************************************
                    ----------------------------
                    |Element ID | Length | SSID|
                    ----------------------------
           Octets:  |1          | 1      | 0~32|
                    ----------------------------
    ***************************************************************************/
    /***************************************************************************
      A SSID  field  of length 0 is  used  within Probe
      Request management frames to indicate the wildcard SSID.
    ***************************************************************************/
    /* 只有beacon会隐藏ssid */
    if ((hmac_vap->cap_flag.hide_ssid) && (frm_type == WLAN_FC0_SUBTYPE_BEACON)) {
        /* ssid ie */
        *buffer = MAC_EID_SSID;
        /* ssid len */
        *(buffer + 1) = 0;
        *ie_len = MAC_IE_HDR_LEN;
        return;
    }

    *buffer = MAC_EID_SSID;
    ssid_len = (osal_u8)strlen((osal_char *)HMAC_P2P_WILDCARD_SSID);   /* 不包含'\0' */

    *(buffer + 1) = ssid_len;

    if (memcpy_s(buffer + MAC_IE_HDR_LEN, ssid_len, HMAC_P2P_WILDCARD_SSID, ssid_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_set_p2p0_ssid_ie::memcpy_s error}");
    }
    *ie_len = ssid_len + MAC_IE_HDR_LEN;
}

/*****************************************************************************
 函 数 名  : mac_set_p2p_noa
 功能描述  : 填充p2p noa信息
*****************************************************************************/
osal_void mac_set_p2p_noa(hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    osal_u8 index = MAC_IE_HDR_LEN;
    osal_u8 ct_window = 0;
    osal_u32 start_time, duration, interval;
    osal_u16 noa_descr_num = is_p2p_noa_enabled(hmac_vap) ? P2P_NOA_DESC_NUM : 0;
    /* 13表示ie中Notice of Absence Descriptor位所占长度为13，2表示Index和 CTWindow/OppPS2所占的2字节 */
    osal_u16 attr_len = (osal_u16)((noa_descr_num * 13) + 2);

    /* p2p noa Element                                                       */
    /* --------------------------------------------------------------------- */
    /* |1          |1          |3     | 1        | 1            | 2        | */
    /* --------------------------------------------------------------------- */
    /* |Tag number |Tag length |OUI   | OUI Type | Attribute ID | Length   | */
    /* --------------------------------------------------------------------- */
    /* |  1    |       1        | 13                                       | */
    /* --------------------------------------------------------------------- */
    /* | Index | CTWindow/OppPS |Notice of Absence Descriptor              | */
    /* --------------------------------------------------------------------- */
    /* Tag number */
    buffer[0] = MAC_EID_P2P;

    /* Tag length */ /* 13表示ie中Notice of Absence Descriptor位所占长度为13，2表示Index和 CTWindow/OppPS2所占的2字节 */
    buffer[1] = (osal_u8)(P2P_OUI_LEN + P2P_ATTR_HDR_LEN + (noa_descr_num * 13) + 2);

    /* OUI */
    if (memcpy_s(&buffer[index], MAC_OUI_LEN, mac_frame_get_p2p_oui(), MAC_OUI_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_mgmt_encap_p2p_presence_rsp::memcpy_s error}");
    }
    index += MAC_OUI_LEN;

    /* OUI Type */
    buffer[index++] = MAC_OUITYPE_P2P;

    /* Notice of Absence attribute */
    /* Attribute ID */
    buffer[index++] = NOTICE_OF_ABSENCE;

    /* Length */
    buffer[index++] = attr_len & 0x00FF;
    buffer[index++] = attr_len >> 8; /* 8表示右移8位，以获得attr_len的高8位 */

    /* Index */
    buffer[index++] = 0;

    /* CTWindow and Opportunity parameters field */
    if (is_p2p_oppps_enabled(hmac_vap)) {
        ct_window = hmac_vap->p2p_ops_param.ct_window;
        ct_window |= BIT7;
    }
    buffer[index++] = ct_window;

    if (!is_p2p_noa_enabled(hmac_vap)) {
        *ie_len = MAC_IE_HDR_LEN + P2P_OUI_LEN + P2P_ATTR_HDR_LEN + 2; /* 2表示ie中Index和 CTWindow/OppPS2所占的2字节 */
        return;
    }
    /* Notice of Absence Descriptor */
    buffer[index++] = hmac_vap->p2p_noa_param.count;

    duration = hmac_vap->p2p_noa_param.duration;
    buffer[index++] = duration & 0x000000FF; /* 取低8位 */
    buffer[index++] = (duration >> 8) & 0x000000FF; /* 8表示右移8位后取低8位 */
    buffer[index++] = (duration >> 16) & 0x000000FF; /* 16表示右移16位后取低8位 */
    buffer[index++] = (duration >> 24); /* 24表示右移24位后取低8位 */

    interval = hmac_vap->p2p_noa_param.interval;
    buffer[index++] = interval & 0x000000FF; /* 取低8位 */
    buffer[index++] = (interval >> 8) & 0x000000FF; /* 8表示右移8位后取低8位 */
    buffer[index++] = (interval >> 16) & 0x000000FF; /* 16表示右移16位后取低8位 */
    buffer[index++] = (interval >> 24); /* 24表示右移24位后取低8位 */

    start_time = hmac_vap->p2p_noa_param.start_time;
    buffer[index++] = start_time & 0x000000FF; /* 取低8位 */
    buffer[index++] = (start_time >> 8) & 0x000000FF; /* 8表示右移8位后取低8位 */
    buffer[index++] = (start_time >> 16) & 0x000000FF; /* 16表示右移16位后取低8位 */
    buffer[index++] = (start_time >> 24); /* 24表示右移24位后取低8位 */

    /* 13表示ie中Notice of Absence Descriptor位所占长度为13， 2表示ie中Index和 CTWindow/OppPS2所占的2字节 */
    *ie_len = MAC_IE_HDR_LEN + P2P_OUI_LEN + P2P_ATTR_HDR_LEN + (P2P_NOA_DESC_NUM * 13) + 2;
}

/*****************************************************************************
 函 数 名  : mac_set_p2p_none_noa
 功能描述  : 填充p2p noa信息不包含NoA description
*****************************************************************************/
osal_void mac_set_p2p_none_noa(const hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    osal_u8       index;

    /* p2p noa Element                                                       */
    /* --------------------------------------------------------------------- */
    /* |1          |1          |3     | 1        | 1            | 2        | */
    /* --------------------------------------------------------------------- */
    /* |Tag number |Tag length |OUI   | OUI Type | Attribute ID | Length   | */
    /* --------------------------------------------------------------------- */
    /* |  1    |       1        | 13                                       | */
    /* --------------------------------------------------------------------- */
    /* | Index | CTWindow/OppPS |Notice of Absence Descriptor              | */
    /* --------------------------------------------------------------------- */
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{mac_set_p2p_none_noa:: hmac_vap is null!!!}");
    }
    /* Tag number */
    buffer[0] = MAC_EID_P2P;

    /* Tag length */
    buffer[1] = P2P_OUI_LEN + P2P_ATTR_HDR_LEN + 2; /* 2表示ie中Index和 CTWindow/OppPS2所占的2字节 */
    index = MAC_IE_HDR_LEN;

    /* OUI */
    if (memcpy_s(&buffer[index], MAC_OUI_LEN, mac_frame_get_p2p_oui(), MAC_OUI_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_set_p2p_none_noa::memcpy_s error}");
    }
    index += MAC_OUI_LEN;

    /* OUI Type */
    buffer[index++] = MAC_OUITYPE_P2P;

    /* Notice of Absence attribute */
    /* Attribute ID */
    buffer[index++] = NOTICE_OF_ABSENCE;

    /* Length */
    buffer[index++] = 2; /* 2表示Length位的第1字节设置为2 */
    buffer[index++] = 0;

    /* Index */
    buffer[index++] = 0;
    /* Set CTWindow and Opportunity parameters field to 0 */
    buffer[index++] = 0;

    *ie_len = MAC_IE_HDR_LEN + P2P_OUI_LEN + P2P_ATTR_HDR_LEN + 2; /* 2表示ie中Index和 CTWindow/OppPS2所占的2字节 */
}

/*****************************************************************************
 函 数 名  : mac_set_p2p_status
 功能描述  : 填充p2p status信息
*****************************************************************************/
osal_void mac_set_p2p_status(osal_u8 *buffer, osal_u8 *ie_len, p2p_status_code_t status)
{
    osal_u8       index;
    osal_u16      attr_len;

    /* ------------------------------------------- */
    /* p2p Status Attribute                       */
    /* ------------------------------------------- */
    /* | 1            | 2        | 1             | */
    /* ------------------------------------------- */
    /* |Attribute ID  | Length   | Status Code   | */
    /* ------------------------------------------- */
    /* Tag number */
    buffer[0] = MAC_EID_P2P;

    /* Tag length */
    buffer[1] = P2P_OUI_LEN + P2P_ATTR_HDR_LEN + 1;
    index = MAC_IE_HDR_LEN;

    /* OUI */
    if (memcpy_s(&buffer[index], MAC_OUI_LEN, mac_frame_get_p2p_oui(), MAC_OUI_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_set_p2p_status::memcpy_s error}");
    }
    index += MAC_OUI_LEN;

    /* OUI Type */
    buffer[index++] = MAC_OUITYPE_P2P;

    /* Attribute ID */
    buffer[index++] = P2P_STATUS;

    /* Length */
    attr_len = 1;
    buffer[index++] = attr_len & 0x00FF;
    buffer[index++] = attr_len >> 8; /* 8表示右移8位后取低8位 */

    buffer[index++] = (osal_u8)status;

    *ie_len = index;
}
/*****************************************************************************
 函 数 名  : hmac_is_p2p_ie
 功能描述  : 是否是P2P IE
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_p2p_ie(const osal_u8 *data)
{
    /* 0表示data指针指向的第0位， 2表示第2位， 3表示第3位，用来判断是否是p2p IE */
    if ((data[0] == MAC_EID_P2P) && (data[2] == WFA_OUI_BYTE1) && (data[3] == WFA_OUI_BYTE2) &&
        /* 4表示data指针指向的第4位， 5表示第5位 */
        (data[4] == WFA_OUI_BYTE3) && (data[5] == WFA_P2P_V1_0)) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}


/*****************************************************************************
 函 数 名  : hmac_get_p2p_attr
 功能描述  : 从beacon帧中查找P2P NoA attr
*****************************************************************************/
osal_u8 *hmac_get_p2p_noa_attr(osal_u8 *frame_body, osal_u16 rx_len, osal_u16 tag_param_offset, osal_u16 *attr_len)
{
    osal_u16  idx;
    osal_u16  num_bytes;
    osal_u32  index_byte;
    osal_u8  *data         = NULL;
    idx = tag_param_offset;

    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* --------------------------------------------------------------------- */
    /* |Timestamp |BeaconInt |CapInfo |SSID |SupRates |DSParSet |TAG       | */
    /* --------------------------------------------------------------------- */
    /* |8         |2         |2       |2-34 |3-10     |3        |???       | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    while (idx < (rx_len - WLAN_HDR_FCS_LENGTH)) {
        if (hmac_is_p2p_ie(frame_body + idx) != OSAL_TRUE) {
            /* Move to the next IE. */
            idx += (MAC_IE_HDR_LEN + frame_body[idx + 1]);
            continue;
        }

        /* 找到P2P IE */
        index_byte = 0;
        /* get the P2P IE len */
        if (frame_body[idx + 1] < P2P_OUI_LEN) {
            oam_warning_log1(0, OAM_SF_P2P, "hmac_get_p2p_noa_attr:: invalid p2p ie len[%d]", frame_body[idx + 1]);
            return OSAL_NULL;
        }
        num_bytes = frame_body[idx + 1] - P2P_OUI_LEN;
        data = frame_body + idx + MAC_IE_HDR_LEN + P2P_OUI_LEN;

        while (index_byte < num_bytes) {
            if (data[index_byte] == NOTICE_OF_ABSENCE) { /* 找到NoA属性 */
                *attr_len = get_attr_len(data + index_byte);
                return (osal_u8*)(frame_body + idx + index_byte + MAC_IE_HDR_LEN + P2P_OUI_LEN + P2P_ATTR_HDR_LEN + 1);
            }
            index_byte += get_attr_len(data + index_byte) + P2P_ATTR_HDR_LEN;
        }

        /* Move to the next IE. */
        idx += (MAC_IE_HDR_LEN + frame_body[idx + 1]);
    }
    return OSAL_NULL;
}

osal_u32 hmac_p2p_event_sync2device(hmac_vap_stru *hmac_vap, osal_u8 type)
{
    osal_s32 ret;
    frw_msg msg_info = {0};
    msg_info.data = (osal_u8 *)&type;
    msg_info.data_len = (osal_u16)sizeof(osal_u8);

    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_PM_POST, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_p2p_event_sync2device::frw_send_msg_to_device failed[%d].}",
            hmac_vap->vap_id, ret);
    }
    return (osal_u32)ret;
}

osal_u32 hmac_send_p2p_state_to_device(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 pause, wlan_vap_mode_enum_uint8 mode)
{
    osal_s32 ret;
    frw_msg msg_info = {0};
    p2p_info_report_stru p2p_info_report = {0};
    p2p_info_report.pause = pause;
    p2p_info_report.vap_mode = mode;

    msg_info.data = (osal_u8 *)&p2p_info_report;
    msg_info.data_len = (osal_u16)sizeof(p2p_info_report);

    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_P2P_STATE, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_send_p2p_state_to_device::return err code[%d]!}", ret);
    }
    return (osal_u32)ret;
}

osal_s32 hmac_p2p_opps_info_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 vap_mode = *(osal_u8 *)msg->data;

    if (osal_unlikely(hmac_vap->hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_P2P, "{hmac_p2p_opps_info_event::hmac_vap->hal_device is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 记录暂停的信道，tbtt中断后切回 */
        hmac_vap->hal_device->hal_scan_params.home_channel = hmac_vap->channel;
    } else {
        if (hmac_vap->p2p_ops_param.pause_ops == OSAL_FALSE) {
            /* 记录暂停的信道，tbtt中断后切回 */
            hmac_vap->hal_device->hal_scan_params.home_channel = hmac_vap->channel;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_get_p2p_status_etc
 功能描述  : 判断p2p 是否为该状态
 输入参数  : osal_u32 p2p_status
             hmac_cfgp2p_status_enum_uint32 status
 输出参数  : 无
 返 回 值  : osal_u8
*****************************************************************************/
osal_u8 hmac_get_p2p_status_etc(osal_u32 p2p_status, hmac_cfgp2p_status_enum_uint32 status)
{
    if (p2p_status & BIT(status)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_set_p2p_status_etc
 功能描述  : 设置p2p 为对应状态
 输入参数  : osal_u32 p2p_status
             hmac_cfgp2p_status_enum_uint32 status
 输出参数  : 无
 返 回 值  : osal_u8
*****************************************************************************/
osal_void hmac_set_p2p_status_etc(osal_u32 *pul_p2p_status, hmac_cfgp2p_status_enum_uint32 status)
{
    *pul_p2p_status |= ((osal_u32)BIT(status));
}

/*****************************************************************************
 函 数 名  : hmac_config_set_p2p_ps_ops_etc
 功能描述  : 设置P2P OPS 节能
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_p2p_ps_ops_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    mac_cfg_p2p_ops_param_stru *p2p_ops;
    p2p_ops = (mac_cfg_p2p_ops_param_stru *)msg->data;
    oam_info_log3(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_p2p_ps_ops_etc:ctrl:%d, ct_window:%d}",
                  hmac_vap->vap_id, p2p_ops->ops_ctrl, p2p_ops->ct_window);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/

    ret = hmac_config_set_p2p_ps_ops(hmac_vap, p2p_ops);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_p2p_ps_ops_etc::hmac_config_set_p2p_ps_ops failed[%d].}",
            hmac_vap->vap_id, ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_p2p_ps_noa_etc
 功能描述  : 设置P2P NOA 节能
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_p2p_ps_noa_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    mac_cfg_p2p_noa_param_stru  *p2p_noa;
    p2p_noa = (mac_cfg_p2p_noa_param_stru *)msg->data;
    oam_info_log4(0, OAM_SF_CFG, "{hmac_config_set_p2p_ps_noa_etc:start_time:%d, duration:%d, interval:%d, count:%d}",
        p2p_noa->start_time, p2p_noa->duration, p2p_noa->interval, p2p_noa->count);
    /* ms to us */
    p2p_noa->start_time *= 1000; /* 1ms == 1000us */
    p2p_noa->duration   *= 1000; /* 1ms == 1000us */
    p2p_noa->interval   *= 1000; /* 1ms == 1000us */
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/

    ret = hmac_config_set_p2p_ps_noa(hmac_vap, p2p_noa);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_p2p_ps_noa_etc::hmac_config_set_p2p_ps_noa failed[%d].}",
            hmac_vap->vap_id, ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_p2p_ps_stat
 功能描述  : 设置P2P 节能统计
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_p2p_ps_stat(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_p2p_stat_param_stru  *p2p_stat;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    p2p_stat = (mac_cfg_p2p_stat_param_stru *)msg->data;
    /* 获取hal_device 结构体 */
    hal_device = hmac_vap->hal_device;

    if (p2p_stat->p2p_statistics_ctrl == 0) {
        /* 清除统计值 */
        hal_clear_irq_stat(hal_device);
    } else if (p2p_stat->p2p_statistics_ctrl == 1) {
        /* 打印统计值 */
        hal_show_irq_info(hal_device, 0);
    } else {
        /* 错误控制命令 */
        oam_warning_log1(0, 0, "vap_id[%d] hmac_config_set_p2p_ps_stat:: wrong p2p ps ctrl vale", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_find_p2p_listen_channel_etc
 功能描述  : 查找wpa_supplicant 下发的IE 中的P2P IE中的listen channel
 输入参数  : osal_u8 *param   wpa_supplicant 下发的ie
             osal_u16 len      wpa_supplicant 下发的ie 长度
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_find_p2p_listen_channel_etc(hmac_vap_stru *hmac_vap, osal_u16 len,
    osal_u8 *param, osal_u8 *p2p_listen_channel)
{
    osal_u8 *p2p_ie = OAL_PTR_NULL;
    osal_u8 *listen_channel_ie = OAL_PTR_NULL;

    /* 查找P2P IE信息 */
    p2p_ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, param, (osal_s32)len);
    if (p2p_ie == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_find_p2p_listen_channel_etc::p2p ie is null.}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 长度校验 */
    if (p2p_ie[1] < MAC_P2P_MIN_IE_LEN) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_find_p2p_listen_channel_etc::invalid p2p ie len[%d].}",
                         hmac_vap->vap_id, p2p_ie[1]);
        return OAL_FAIL;
    }

    /* 查找P2P Listen channel信息 */
    listen_channel_ie = hmac_find_p2p_attribute_etc(MAC_P2P_ATTRIBUTE_LISTEN_CHAN,
        p2p_ie + P2P_IE_HDR_LEN, (p2p_ie[1] - MAC_P2P_MIN_IE_LEN));
    if (listen_channel_ie == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_find_p2p_listen_channel_etc::p2p listen channel ie is null.}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* listen channel长度校验，大端 */
    if ((osal_s32)((listen_channel_ie[2] << 8) + listen_channel_ie[1]) !=  /* 2进制高位左移8位切换为10进制 */
            MAC_P2P_LISTEN_CHN_ATTR_LEN) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_find_p2p_listen_channel_etc::invalid p2p listen channel ie len[%d].}",
            hmac_vap->vap_id, (osal_s32)((listen_channel_ie[2] << 8) +   /* 2进制高位左移8位切换为10进制 */
            listen_channel_ie[1]));
        return OAL_FAIL;
    }

    /* 获取P2P Listen channel信息 */
    if (p2p_listen_channel != OAL_PTR_NULL) {
        *p2p_listen_channel = listen_channel_ie[2 + MAC_P2P_LISTEN_CHN_ATTR_LEN]; /* 2进制高位左移8位切换为10进制 */
    }
    oam_info_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_find_p2p_listen_channel_etc::CHANNEL[%d].}",
                  hmac_vap->vap_id, listen_channel_ie[2 + MAC_P2P_LISTEN_CHN_ATTR_LEN]); /* 2个字节IE头 */

    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 设置P2P OPS 节能
输入参数  : hmac_vap_stru *mac_vap
          osal_u8 len
          osal_u8 *param
返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_p2p_ps_ops(hmac_vap_stru *hmac_vap, mac_cfg_p2p_ops_param_stru *p2p_ops)
{
    hal_to_dmac_vap_stru *hal_vap;

    if (p2p_ops == OSAL_NULL) {
        return OAL_FAIL;
    }

    /*  sanity check, CTwindow 7bit, max value not beyond 127 */
    if (p2p_ops->ct_window < 127) {
        /* wpa_supplicant只设置ctwindow size,保存dmac,不写寄存器 */
        hmac_vap->p2p_ops_param.ct_window = p2p_ops->ct_window;
        return OAL_SUCC;
    }
    /*  sanity check, opps ctrl 1bit, max value not beyond 2 */
    if (p2p_ops->ops_ctrl < 2) {
        if ((p2p_ops->ops_ctrl == 0) && is_p2p_oppps_enabled(hmac_vap)) {
            /* 恢复发送 */
            hmac_p2p_handle_ps(hmac_vap, OSAL_FALSE);
        }
        hmac_vap->p2p_ops_param.ops_ctrl = p2p_ops->ops_ctrl;
    } else {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] hmac_config_set_p2p_ps_ops:invalid ops ctrl value[%d]",
            hmac_vap->vap_id, p2p_ops->ops_ctrl);
        return OAL_FAIL;
    }
    oam_warning_log4(0, OAM_SF_CFG, "vap_id[%d] hmac_config_set_p2p_ps_ops:ctrl[%d] ct_window[%d] vap state[%d]",
        hmac_vap->vap_id, hmac_vap->p2p_ops_param.ops_ctrl, hmac_vap->p2p_ops_param.ct_window, hmac_vap->vap_state);
    hal_vap = hmac_vap->hal_vap;
    /* 设置P2P ops 寄存器 */
    hal_vap_set_ops(hal_vap, hmac_vap->p2p_ops_param.ops_ctrl, hmac_vap->p2p_ops_param.ct_window);
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 设置P2P NOA 节能
输入参数  : hmac_vap_stru *mac_vap
          osal_u8 len
          osal_u8 *param
返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_p2p_ps_noa(hmac_vap_stru *hmac_vap, mac_cfg_p2p_noa_param_stru *p2p_noa)
{
    osal_u32 current_tsf_lo;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (is_p2p_go(hmac_vap)) {
#ifdef _PRE_WLAN_FEATURE_DBAC
        if (mac_is_dbac_running(hmac_device)) {
            /* dbac运行中 go的noa由dbac接管 */
            return OAL_SUCC;
        }
#endif

        /* 如果是GO, interval设为beacon interval，需要乘以1024转换 */
        p2p_noa->interval = (osal_u32)hmac_device->beacon_interval * 1024;
    }

    /* 保存参数用于encap probe rsp,beacon */
    if (p2p_noa->count != 0) {
        hmac_vap->p2p_noa_param.count = p2p_noa->count;
        hmac_vap->p2p_noa_param.duration = p2p_noa->duration;
        hmac_vap->p2p_noa_param.interval = p2p_noa->interval;
        hal_vap_tsf_get_32bit(hmac_vap->hal_vap, &current_tsf_lo);
        p2p_noa->start_time += current_tsf_lo;
        hmac_vap->p2p_noa_param.start_time = p2p_noa->start_time;
    } else {
        if (is_p2p_noa_enabled(hmac_vap)) {
            /* 恢复发送 */
            hmac_p2p_handle_ps(hmac_vap, OSAL_FALSE);
        }

        (osal_void)memset_s(&(hmac_vap->p2p_noa_param), sizeof(mac_cfg_p2p_noa_param_stru),
            0, sizeof(mac_cfg_p2p_noa_param_stru));
    }
    oam_warning_log4(0, OAM_SF_CFG,
        "hmac_config_set_p2p_ps_noa:start_time:%d, duration:%d, interval:%d, count:%d\r\n",
        hmac_vap->p2p_noa_param.start_time, hmac_vap->p2p_noa_param.duration, hmac_vap->p2p_noa_param.interval,
        hmac_vap->p2p_noa_param.count);
    /* 设置P2P noa 寄存器 */
    hal_vap_set_noa(hmac_vap->hal_vap, hmac_vap->p2p_noa_param.start_time, hmac_vap->p2p_noa_param.duration,
                    hmac_vap->p2p_noa_param.interval, hmac_vap->p2p_noa_param.count);
    return OAL_SUCC;
}

osal_s32 hmac_config_set_p2p_noa_adapt(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_p2p_noa_param_stru *p2p_noa_param;

    /* 保存节能参数，用于encap beacon,probe response */
    p2p_noa_param = (mac_cfg_p2p_noa_param_stru *)msg->data;
    if (p2p_noa_param == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_WIFI_BEACON, "{hmac_config_set_p2p_noa_adapt::msg null.vap_id = %u}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_WIFI_BEACON, "{hmac_config_set_p2p_noa_adapt::mac_res_get_hmac_vap fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    (osal_void)memcpy_s(&hmac_vap->p2p_noa_param, sizeof(hmac_vap->p2p_noa_param),
        p2p_noa_param, sizeof(mac_cfg_p2p_noa_param_stru));
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置wifi 返回指定信道
 输入参数  : hmac_vap_stru *mac_vap
           osal_u8 len
           osal_u8 *param
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_cancel_remain_on_channel(hmac_vap_stru *hmac_vap)
{
    mac_scan_req_stru *scan_req_params;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 1.判断当前是否正在扫描，如果不是在扫描，直接返回 */
    if (hmac_device->curr_scan_state != MAC_SCAN_STATE_RUNNING) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_config_cancel_remain_on_channel::scan is not running.hmac_vap state[%d]}",
            hmac_vap->vap_id, hmac_vap->vap_state);
        return OAL_SUCC;
    }

    scan_req_params = &(hmac_device->scan_params);

    /* 2.判断当前扫描的vap是不是p2p的，如果不是，直接返回，如果是，判断是否在监听，不是直接返回 */
    if ((hmac_vap->vap_id != scan_req_params->vap_id) || (scan_req_params->scan_func != MAC_SCAN_FUNC_P2P_LISTEN)) {
        oam_warning_log2(0, OAM_SF_SCAN,
                         "vap_id[%d] {hmac_config_cancel_remain_on_channel::other vap is scanning, scan_func[%d].}",
                         hmac_vap->vap_id, scan_req_params->scan_func);
        return OAL_SUCC;
    }

    /* 调用扫描完成 */
    oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_config_cancel_remain_on_channel::stop scan.}", hmac_vap->vap_id);

    hmac_scan_abort(hmac_device);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_p2p_listen_comp_cb_etc
 功能描述  : P2P 监听超时处理函数
 输入参数  : void *p_arg
 输出参数  : 无
 返 回 值  : osal_void
*****************************************************************************/
osal_void hmac_p2p_listen_comp_cb_etc(void *p_arg)
{
    hmac_vap_stru                      *hmac_vap;
    hmac_device_stru                   *hmac_device;
    hmac_scan_record_stru              *scan_record;

    scan_record = (hmac_scan_record_stru *)p_arg;

    /* 判断listen完成时的状态 */
    if (MAC_SCAN_SUCCESS != scan_record->scan_rsp_status) {
        oam_warning_log1(0, OAM_SF_P2P, "{hmac_p2p_listen_comp_cb_etc::listen failed, listen rsp status: %d.}",
            scan_record->scan_rsp_status);
    }

    hmac_vap   = mac_res_get_hmac_vap(scan_record->vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_P2P, "{hmac_p2p_listen_comp_cb_etc::hmac_vap is null:vap_id %d.}",
            scan_record->vap_id);
        return;
    }

    hmac_device = hmac_res_get_mac_dev_etc(scan_record->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_P2P, "{hmac_p2p_listen_comp_cb_etc::hmac_device is null:vap_id %d.}",
            scan_record->device_id);
        return;
    }

    /* 解决wifi 驱动不上报监听结束问题。
      此处上报ull_cookie不加1，因为在邋remain_on_channel下发时刻便已+1 */
    if (scan_record->ull_cookie == hmac_device->st_p2p_info.ull_last_roc_id) {
        /* 状态机调用: hmac_p2p_listen_timeout_etc */
        if ((hmac_vap->p2p_mode == WLAN_P2P_GO_MODE) &&
            (hmac_fsm_call_func_ap_etc(hmac_vap, HMAC_FSM_INPUT_LISTEN_TIMEOUT, hmac_vap) != OAL_SUCC)) {
            oam_warning_log0(0, OAM_SF_P2P, "{hmac_p2p_listen_comp_cb_etc::hmac_fsm_call_func_ap_etc fail.}");
        } else if (hmac_fsm_call_func_sta_etc(hmac_vap, HMAC_FSM_INPUT_LISTEN_TIMEOUT,
            hmac_vap) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_P2P, "{hmac_p2p_listen_comp_cb_etc::hmac_fsm_call_func_sta_etc fail.}");
        }
    } else {
        oam_warning_log4(0, OAM_SF_P2P,
            "vap_id[%d] {ignore listen complete.scan_report_cookie[%x], current_listen_cookie[%x],ull_last_roc_id[%x]}",
            hmac_vap->vap_id, scan_record->ull_cookie,
            hmac_device->scan_params.cookie, hmac_device->st_p2p_info.ull_last_roc_id);
    }
    return;
}
/*****************************************************************************
 函 数 名  : hmac_mgmt_tx_roc_comp_cb
 功能描述  : P2P 监听超时处理函数
 输入参数  : void *p_arg
 输出参数  : 无
 返 回 值  : osal_void
*****************************************************************************/
osal_void hmac_mgmt_tx_roc_comp_cb(void *p_arg)
{
    hmac_vap_stru                      *hmac_vap;
    hmac_device_stru                   *hmac_device;
    hmac_scan_record_stru              *scan_record;

    scan_record = (hmac_scan_record_stru *)p_arg;

    /* 判断listen完成时的状态 */
    if (MAC_SCAN_SUCCESS != scan_record->scan_rsp_status) {
        oam_warning_log1(0, OAM_SF_P2P, "{hmac_mgmt_tx_roc_comp_cb::listen failed, listen rsp status: %d.}",
            scan_record->scan_rsp_status);
    }

    hmac_vap   = mac_res_get_hmac_vap(scan_record->vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_P2P, "{hmac_mgmt_tx_roc_comp_cb::hmac_vap is null:vap_id %d.}",
            scan_record->vap_id);
        return;
    }

    hmac_device = hmac_res_get_mac_dev_etc(scan_record->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_P2P, "vap_id[%d] {hmac_mgmt_tx_roc_comp_cb::hmac_device is null:vap_id %d.}",
            hmac_vap->vap_id, scan_record->device_id);
        return;
    }

    /* 由于P2P0 和P2P_CL 共用vap 结构体，监听超时，返回监听前保存的状态 */
    /* p2p listen同步vap状态与背景扫描冲突，不同步到dmac */
    hmac_vap_state_change_etc(hmac_vap, hmac_device->st_p2p_info.last_vap_state);

    oam_warning_log1(0, OAM_SF_P2P, "vap_id[%d] {hmac_mgmt_tx_roc_comp_cb}", hmac_vap->vap_id);
}

/*****************************************************************************
 函 数 名  : hmac_cfg80211_prepare_listen_req_param
 功能描述  : 准备p2p监听请求的对应参数
 输入参数  : mac_scan_req_stru *scan_params,        扫描参数
             osal_s8 *param,                       p2p监听参数
 输出参数  : 无
 返 回 值  : osal_void
*****************************************************************************/
OAL_STATIC osal_void hmac_cfg80211_prepare_listen_req_param(mac_scan_req_stru *scan_params, osal_s8 *param)
{
    mac_remain_on_channel_param_stru *remain_on_channel;
    mac_channel_stru                 *channel_tmp;

    remain_on_channel = (mac_remain_on_channel_param_stru *)param;

    memset_s(scan_params, OAL_SIZEOF(mac_scan_req_stru), 0, OAL_SIZEOF(mac_scan_req_stru));

    /* 设置监听信道信息到扫描参数中 */
    scan_params->channel_list[0].band           = remain_on_channel->band;
    scan_params->channel_list[0].en_bandwidth   = remain_on_channel->listen_channel_type;
    scan_params->channel_list[0].chan_number    = remain_on_channel->listen_channel;
    scan_params->channel_list[0].chan_idx       = 0;
    channel_tmp = &(scan_params->channel_list[0]);
    if (hmac_get_channel_idx_from_num_etc(channel_tmp->band, channel_tmp->chan_number,
        &(channel_tmp->chan_idx)) != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_P2P,
            "{hmac_cfg80211_prepare_listen_req_param::hmac_get_channel_idx_from_num_etc fail.band[%u]  channel[%u]}",
            channel_tmp->band, channel_tmp->chan_number);
    }

    /* 设置其它监听参数 */
    scan_params->max_scan_count_per_channel      = 1;
    scan_params->channel_nums = 1;
    scan_params->scan_func       = MAC_SCAN_FUNC_P2P_LISTEN;
    scan_params->scan_time    = (osal_u16)remain_on_channel->listen_duration;
    if (IEEE80211_ROC_TYPE_MGMT_TX == remain_on_channel->roc_type) {
        scan_params->fn_cb         = hmac_mgmt_tx_roc_comp_cb;
    } else {
        scan_params->fn_cb         = hmac_p2p_listen_comp_cb_etc;
    }
    scan_params->cookie      = remain_on_channel->cookie;
    scan_params->is_p2p0_scan = OAL_TRUE;
    scan_params->p2p0_listen_channel = remain_on_channel->listen_channel;

    return;
}

/*****************************************************************************
 函 数 名  : hmac_p2p_listen_timeout_etc
 功能描述  : P2P_DEVICE 监听超时
 输入参数  : hmac_vap_stru *hmac_vap_sta
             osal_void *p_param
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_p2p_listen_timeout_etc(hmac_vap_stru *hmac_vap, osal_void *p_param)
{
    hmac_device_stru                   *hmac_device;
    hmac_scan_record_stru              *scan_record;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_P2P, "{hmac_p2p_listen_timeout_etc::hmac_res_get_mac_dev_etc fail.device_id[%u]!}",
            hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_info_log3(0, OAM_SF_P2P,
        "vap_id[%d] {hmac_p2p_listen_timeout_etc::current hmac_vap channel is [%d] state[%d]}",
        hmac_vap->vap_id,
        hmac_vap->channel.chan_number,
        hmac_vap->vap_state);

    oam_info_log3(0, OAM_SF_P2P,
        "vap_id[%d] {hmac_p2p_listen_timeout_etc::next hmac_vap channel is [%d] state[%d]}",
        hmac_vap->vap_id,
        hmac_vap->channel.chan_number,
        hmac_device->st_p2p_info.last_vap_state);

    /* 由于P2P0 和P2P_CL 共用vap 结构体，监听超时，返回监听前保存的状态 */
    /* p2p listen同步vap状态与背景扫描冲突，不同步到dmac */
    hmac_vap_state_change_etc(hmac_vap, hmac_device->st_p2p_info.last_vap_state);

    scan_record = &(hmac_device->scan_mgmt.scan_record_mgmt);
    if (scan_record->ull_cookie == hmac_device->st_p2p_info.ull_last_roc_id) {
        /* 3.1 抛事件到WAL ，上报监听结束 */
        hmac_p2p_send_listen_expired_to_host_etc(hmac_vap);
    }

    /* 3.2 抛事件到DMAC ，返回监听信道 */
    hmac_p2p_send_listen_expired_to_device_etc(hmac_vap);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_p2p_remain_on_channel_etc
 功能描述  : 设置device 到指定信道监听，并设置监听超时定时器
             如果是从up 状态进入listen ，则返回up
             如果是从scan complete 状态进入，则返回scan complete
 输入参数  : hmac_vap_stru *hmac_vap_sta
             osal_void *p_param
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_p2p_remain_on_channel_etc(hmac_vap_stru *hmac_vap_sta, osal_void *p_param)
{
    hmac_device_stru                    *hmac_device;
    mac_remain_on_channel_param_stru    *remain_on_channel;
    mac_scan_req_stru                    scan_params;
    osal_u32                           ul_ret;

    remain_on_channel = (mac_remain_on_channel_param_stru *)p_param;

    hmac_device  = hmac_res_get_mac_dev_etc(hmac_vap_sta->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_device[%d](%p) null!}", hmac_vap_sta->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置P2P 监听超时定时器,
        P2P处于Listen状态下无法下发监听命令，导致Listen状态下不能重新出发监听，无法与商用设备关联 */
    if (MAC_VAP_STATE_LISTEN == hmac_vap_sta->vap_state) {
        hmac_p2p_send_listen_expired_to_host_etc(hmac_vap_sta);
        oam_warning_log2(0, OAM_SF_P2P,
            "vap_id[%d] {listen nested, send remain on channel expired to host!curr_state[%d]}",
            hmac_vap_sta->vap_id,
            hmac_vap_sta->vap_state);
    }

    /* 修改P2P_DEVICE 状态为监听状态 */
    /* P2P监听状态与背景扫描时DMAC的pause状态冲突，P2P_LISTEN状态不同步到DMAC去，DMAC不再使用P2P_LISTEN状态 */
    hmac_vap_state_change_etc(hmac_vap_sta, MAC_VAP_STATE_LISTEN);
    oam_info_log4(0, OAM_SF_P2P,
        "{hmac_p2p_remain_on_channel_etc : get in listen state!last_state %d, channel %d, duration %d, curr_state %d}",
        hmac_device->st_p2p_info.last_vap_state,
        remain_on_channel->listen_channel,
        remain_on_channel->listen_duration,
        hmac_vap_sta->vap_state);

    /* 准备监听参数 */
    hmac_cfg80211_prepare_listen_req_param(&scan_params, (osal_s8 *)remain_on_channel);

    /* 调用扫描入口，准备进行监听动作，不管监听动作执行成功或失败，都返回监听成功 */
    /* 状态机调用: hmac_scan_proc_scan_req_event_etc */
    ul_ret = hmac_fsm_call_func_sta_etc(hmac_vap_sta, HMAC_FSM_INPUT_SCAN_REQ, (osal_void *)(&scan_params));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_p2p_remain_on_channel_etc::hmac_fsm_call_func_sta_etc fail[%d].}",
            ul_ret);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_remain_on_channel_etc
 功能描述  : 保持在指定信道
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
 如果允许进入监听状态，则设置其他设备节能，并停止发送
*****************************************************************************/
osal_s32 hmac_config_remain_on_channel_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_remain_on_channel_param_stru *remain_on_channel = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    osal_u32 ret;

    /* 1.1 判断入参 */
    if (hmac_vap == OAL_PTR_NULL || msg->data == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_P2P, "{hmac_config_remain_on_channel_etc null ptr:vap=%p; param=%p}",
            (uintptr_t)hmac_vap, (uintptr_t)(msg->data));
        return OAL_ERR_CODE_PTR_NULL;
    }

    remain_on_channel = (mac_remain_on_channel_param_stru *)msg->data;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_remain_on_channel_etc::mac_device[%p]!}", hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1.2 检查是否能进入监听状态 */
    ret = hmac_p2p_check_can_enter_state_etc(hmac_vap, HMAC_FSM_INPUT_LISTEN_REQ);
    if (ret != OAL_SUCC) {
        /* 不能进入监听状态，返回设备忙 */
        oam_warning_log2(0, OAM_SF_P2P, "vap_id[%d] {hmac_config_remain_on_channel_etc fail,device busy: ul_ret=%d}",
            hmac_vap->vap_id, ret);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* 1.3 获取home 信道和信道类型。如果返回主信道为0，表示没有设备处于up 状态，监听后不需要返回主信道 */

    /* 保存内核下发的监听信道信息，用于监听超时或取消监听时返回 */
    hmac_device->st_p2p_info.st_listen_channel = remain_on_channel->listen_channel_para;

    /* 由于p2p0和 p2p cl 共用一个VAP 结构，故在进入监听时，需要保存之前的状态，便于监听结束时返回 */
    if (hmac_vap->vap_state != MAC_VAP_STATE_LISTEN) {
        hmac_device->st_p2p_info.last_vap_state = hmac_vap->vap_state;
    }
    remain_on_channel->last_vap_state = hmac_device->st_p2p_info.last_vap_state;

    oam_info_log4(0, OAM_SF_P2P, "vap_id[%d] {listen_channel=%d, current_channel=%d, last_state=%d}",
        hmac_vap->vap_id, remain_on_channel->listen_channel,
        hmac_vap->channel.chan_number, hmac_device->st_p2p_info.last_vap_state);

    /* 在Miracast链接后 关闭P2P监听流程 */
    /* 由WPA配置监听时间一般为300ms以上 */
    if ((hmac_vap->vap_state == MAC_VAP_STATE_UP) && (remain_on_channel->listen_duration > 300)) {
        remain_on_channel->listen_duration = 20;  /* Miracast链接后,不再执行长监听命令,将监听时间改为20ms */
    }

    /* 状态机调用:  hmac_config_remain_on_channel_etc */
    if (hmac_vap->p2p_mode == WLAN_P2P_GO_MODE) {
        ret = hmac_fsm_call_func_ap_etc(hmac_vap, HMAC_FSM_INPUT_LISTEN_REQ, (osal_void *)(remain_on_channel));
    } else {
        ret = hmac_fsm_call_func_sta_etc(hmac_vap, HMAC_FSM_INPUT_LISTEN_REQ, (osal_void *)(remain_on_channel));
    }
    if (ret != OAL_SUCC) {
        /* DMAC 设置切换信道失败 */
        oam_warning_log2(0, OAM_SF_P2P, "vap_id[%d] {hmac_config_remain_on_channel_etc fail: ul_ret=%d}",
                         hmac_vap->vap_id, ret);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    oam_info_log4(0, OAM_SF_P2P,
        "vap_state[%d] {hmac_config_remain_on_channel_etc succ: cookie=%llu, duration=%d, band=%d}",
        hmac_vap->vap_state, remain_on_channel->cookie, remain_on_channel->listen_duration,
        remain_on_channel->band);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_cancel_remain_on_channel_etc
 功能描述  : 停止保持在指定信道
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_cancel_remain_on_channel_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (hmac_vap->vap_state == MAC_VAP_STATE_LISTEN) {
        hmac_p2p_listen_timeout_etc(hmac_vap, OSAL_NULL);
    } else {
        hmac_p2p_send_listen_expired_to_host_etc(hmac_vap);
    }
    return OAL_SUCC;
}

oal_module_symbol(hmac_get_p2p_status_etc);
oal_module_symbol(hmac_set_p2p_status_etc);


#ifdef __cplusplus
#if __cplusplus
}
#endif

#endif /* _PRE_WLAN_FEATURE_P2P */
#endif