/*
 * Copyright (c) CompanyNameMagicTag. 2023-2024. All rights reserved.
  文 件 名   : hmac_11r.c
  生成日期   : 2023年3月2日
 * 功能描述   : 11R
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_11r.h"
#include "wlan_types.h"
#include "oal_netbuf_data.h"
#include "mac_frame_common_rom.h"
#include "mac_frame.h"
#include "hmac_roam_if.h"
#include "hmac_roam_connect.h"
#include "diag_log_common.h"
#include "hmac_feature_dft.h"
#include "hmac_sme_sta.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_sta.h"
#include "hmac_chan_mgmt.h"
#include "hmac_feature_interface.h"
#include "frw_util_notifier.h"
#include "hmac_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_11R_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_11R
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_11r_vap_info_stru *g_11r_vap_info[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {
    OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL
};

/*****************************************************************************
  3 函数声明
*****************************************************************************/

/*****************************************************************************
  4 函数实现
*****************************************************************************/
OAL_STATIC hmac_11r_vap_info_stru *hmac_11r_get_vap_info(const hmac_vap_stru *hmac_vap)
{
    if (hmac_vap_id_param_check(hmac_vap->vap_id) != OSAL_TRUE) {
        return OSAL_NULL;
    }

    return g_11r_vap_info[hmac_vap->vap_id];
}

/*****************************************************************************
函 数 名  : hmac_11r_init_vap
功能描述  : 创建vap时，初始化11r vap结构体；
输入参数  : hmac_vap
输出参数  : 无
返 回 值  : 无
 *****************************************************************************/
OAL_STATIC osal_bool hmac_11r_add_vap(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_u8 vap_id;
    osal_void *mem_ptr = OSAL_NULL;
    mac_device_voe_custom_stru *mac_voe_custom_param = mac_get_pst_mac_voe_custom_param();

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    vap_id = hmac_vap->vap_id;
    if (g_11r_vap_info[vap_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] hmac_11r_add_vap mem already malloc!", vap_id);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_11r_vap_info_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_11r_add_vap mem alloc fail");
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_11r_vap_info_stru), 0, sizeof(hmac_11r_vap_info_stru));
    /* 注册特性数据结构 */
    g_11r_vap_info[vap_id] = (hmac_11r_vap_info_stru *)mem_ptr;
    g_11r_vap_info[vap_id]->enable_11r = mac_voe_custom_param->en_11r;
    g_11r_vap_info[vap_id]->enable_11r_over_ds = mac_voe_custom_param->en_11r_ds;

    return OAL_TRUE;
}

OAL_STATIC osal_bool hmac_11r_del_vap(osal_void *notify_data)
{
    osal_u8 vap_id_11r;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    vap_id_11r = hmac_vap->vap_id;
    if (g_11r_vap_info[vap_id_11r] != OSAL_NULL) {
        oal_mem_free(g_11r_vap_info[vap_id_11r], OAL_TRUE);
        g_11r_vap_info[vap_id_11r] = OSAL_NULL;
    }

    return OAL_TRUE;
}

/*
 * 函 数 名  : hmac_roam_connect_ft_ds_change_to_air_etc
 * 功能描述  : FT漫游由DS模式切换为Air方式重新连接
 * 1.日    期  : 2022年3月14日
 *   修改内容  : 新生成函数
 */
OAL_STATIC osal_u32 hmac_roam_connect_ft_ds_change_to_air_etc(hmac_vap_stru *hmac_vap, osal_void *p_param)
{
    osal_u32 ul_ret;
    hmac_roam_info_stru *roam_info = OAL_PTR_NULL;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_ft_ds_change_to_air_etc::vap null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (p_param == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_ft_ds_change_to_air_etc::param null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);

    hmac_roam_main_clear(roam_info);
    hmac_roam_main_del_timer(roam_info);
    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_SCANING);

    ul_ret = hmac_roam_connect_to_bss(roam_info, p_param);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_notify_wpas
 功能描述  : 将关联成功消息通知wpa_supplicant，以便开始4-way握手流程
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_ft_notify_wpas(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr,
    osal_u16 msg_len)
{
    osal_u8 *evt = OSAL_NULL;
    osal_u32 len;

    hmac_roam_ft_stru *ft_event = OAL_PTR_NULL;
    osal_u8 *target_bssid = OSAL_NULL;
    osal_u16 ie_offset;
    osal_s32 ret;
    frw_msg msg_info = {0};

    if (mac_get_frame_type_and_subtype(mac_hdr) == (WLAN_FC0_SUBTYPE_AUTH | WLAN_FC0_TYPE_MGT)) {
        ie_offset = OAL_AUTH_IE_OFFSET;
        /*********************************************************/
        /*              Management Frame Format                  */
        /* ------------------------------------------------------*/
        /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control| */
        /* ------------------------------------------------------*/
        /* | 2           |2       |6 |6 |6    |2               | */
        /* ------------------------------------------------------*/
        /*********************************************************/
        /* Auth的target ap bssid偏移量为mac_hdr结尾向前偏移8字节 */
        target_bssid = mac_hdr + MAC_80211_FRAME_LEN - 8;
    } else {
        ie_offset = OAL_FT_ACTION_IE_OFFSET;
        /* FT RESP ACTION帧的target ap bssid偏移量为mac_hdr结尾向后偏移8字节 */
        target_bssid = mac_hdr + MAC_80211_FRAME_LEN + 8;
    }
    if (msg_len <= ie_offset) {
        oam_error_log2(0, OAM_SF_ROAM, "hmac_roam_ft_notify_wpas::msg_len[%d] <= ie_offset[%d]",
            msg_len, ie_offset);
        return OAL_FAIL;
    }
    len = sizeof(hmac_roam_ft_stru) + msg_len - ie_offset;
    evt = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, (osal_u16)len, OAL_TRUE);
    if (evt == OSAL_NULL) {
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    (osal_void)memset_s(evt, len, 0, len);
    ft_event = (hmac_roam_ft_stru*)evt;
    /* save target bssid */
    oal_set_mac_addr(ft_event->bssid, target_bssid);
    /* save ft ie */
    ft_event->ft_ie_len = msg_len - ie_offset;
    ret = memcpy_s(evt + sizeof(hmac_roam_ft_stru), msg_len - ie_offset, mac_hdr + ie_offset, msg_len - ie_offset);
    if (ret != EOK) {
        oal_mem_free(evt, OSAL_TRUE);
        evt = OSAL_NULL;
        return OAL_FAIL;
    }

    msg_info.data = evt;
    msg_info.data_len = (osal_u16)len;
    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_FT_EVENT_STA, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    oal_mem_free(evt, OSAL_TRUE);
    evt = OSAL_NULL;
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_ft_notify_wpas, post msg fail} msg[%d] ret[%d]",
            WLAN_MSG_H2W_FT_EVENT_STA, ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC void hmac_encap_ft_req(hmac_vap_stru *hmac_vap, osal_u8 *ft_buff, const hmac_roam_info_stru *roam_info,
    osal_u16 *ft_len)
{
    osal_u8 *my_mac_addr = OSAL_NULL;
    osal_u8 *current_bssid = OSAL_NULL;
    osal_u16 app_ie_len;

    my_mac_addr = mac_mib_get_station_id(hmac_vap);
    current_bssid = hmac_vap->bssid;
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
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(ft_buff, WLAN_FC0_SUBTYPE_ACTION);
    /* Set DA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)ft_buff)->address1, current_bssid);
    /* Set SA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)ft_buff)->address2, my_mac_addr);
    /* Set SSID  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)ft_buff)->address3, current_bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(ft_buff, 0);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*                  FT Request Frame - Frame Body                        */
    /* --------------------------------------------------------------------- */
    /* | Category | Action | STA Addr |Target AP Addr | FT Req frame body  | */
    /* --------------------------------------------------------------------- */
    /* |     1    |   1    |     6    |       6       |       varibal      | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    ft_buff += MAC_80211_FRAME_LEN;
    *ft_len = MAC_80211_FRAME_LEN;

    ft_buff[0] = MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION;
    ft_buff[1] = MAC_FT_ACTION_REQUEST;
    ft_buff += 2; /* 2表示Category+Action字段长度 */
    *ft_len += 2; /* 2表示Category+Action字段长度 */

    oal_set_mac_addr(ft_buff, my_mac_addr);
    ft_buff += OAL_MAC_ADDR_LEN;
    *ft_len += OAL_MAC_ADDR_LEN;

    oal_set_mac_addr(ft_buff, roam_info->connect.bss_dscr->bssid);
    ft_buff += OAL_MAC_ADDR_LEN;
    *ft_len += OAL_MAC_ADDR_LEN;

    hmac_add_app_ie_etc((osal_void *)hmac_vap, ft_buff, &app_ie_len, OAL_APP_FT_IE);
    ft_buff += app_ie_len;
    *ft_len += app_ie_len;
}

/*****************************************************************************
 函 数 名  : hmac_roam_send_ft_req
 功能描述  : 发送FT REQUEST帧
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_send_ft_req(hmac_roam_info_stru *roam_info, osal_void *param)
{
    osal_u32              ret;
    hmac_vap_stru          *hmac_vap;
    hmac_user_stru         *hmac_user;
    oal_netbuf_stru        *ft_frame;
    osal_u8              *ft_buff;
    mac_tx_ctl_stru        *tx_ctl;
    osal_u16              ft_len;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    unref_param(param);

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_INIT);
    if ((ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_send_ft_req::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap  = roam_info->hmac_vap;
    hmac_user = roam_info->hmac_user;
    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL || hmac_11r_vap_info->enable_11r != OAL_TRUE) {
        return OAL_FAIL;
    }

    ft_frame = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (ft_frame == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_send_ft_req::malloc fail.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ft_buff = (osal_u8 *)OAL_NETBUF_HEADER(ft_frame);
    memset_s(oal_netbuf_cb(ft_frame), FT_NETBUF_CB_LENTH, 0, FT_NETBUF_CB_LENTH);
    memset_s(ft_buff, MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    hmac_encap_ft_req(hmac_vap, ft_buff, roam_info, &ft_len);

    oal_netbuf_put(ft_frame, ft_len);

    /* 为填写发送描述符准备参数 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(ft_frame);
    mac_get_cb_mpdu_len(tx_ctl) = ft_len;
    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_netbuf_num(tx_ctl) = 1;

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, ft_frame, ft_len);
    if ((ret != OAL_SUCC)) {
        oal_netbuf_free(ft_frame);
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_send_ft_req::hmac_tx_mgmt_send_event_etc failed[%d].}", hmac_vap->vap_id, ret);
        return ret;
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_FT_COMP);

    /* 启动认证超时定时器 */
    hmac_roam_connect_start_timer(roam_info, ROAM_AUTH_TIME_MAX);

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_roam_process_ft_rsp
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_process_ft_rsp(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32                   ret;
    hmac_vap_stru               *hmac_vap;
    oal_netbuf_stru             *net_buffer;
    mac_rx_ctl_stru            *pst_rx_ctrl;
    osal_u8                   *mac_hdr;
    osal_u16                   auth_status;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_WAIT_FT_COMP);
    if ((ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_ft_rsp::check_state fail[%d]}", ret);
        return ret;
    }

    hmac_vap = roam_info->hmac_vap;
    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL || hmac_11r_vap_info->enable_11r != OAL_TRUE) {
        return OAL_FAIL;
    }

    net_buffer  = (oal_netbuf_stru *)p_param;
    pst_rx_ctrl    = (mac_rx_ctl_stru *)oal_netbuf_cb(net_buffer);
    mac_hdr    = (osal_u8 *)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl);

    auth_status = mac_get_ft_status(mac_hdr);
    if (auth_status != MAC_SUCCESSFUL_STATUSCODE) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_ft_rsp::status [%d], change to over air}", auth_status);
        roam_info->connect.ft_force_air = OAL_TRUE;
        roam_info->connect.ft_failed = OAL_TRUE;
        ret = hmac_roam_connect_ft_ds_change_to_air_etc(hmac_vap, roam_info->connect.bss_dscr);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_ft_rsp::over ds change to over air failed [%d]}", ret);
            return hmac_roam_connect_fail(roam_info);
        }
        return OAL_SUCC;
    }

    /* 上报FT成功消息给APP，以便APP下发新的FT_IE用于发送reassociation */
    ret = hmac_roam_ft_notify_wpas(hmac_vap, mac_hdr, pst_rx_ctrl->frame_len);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_process_ft_rsp::hmac_roam_ft_notify_wpas failed [%d]}", ret);
        return ret;
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);

    /* 启动关联超时定时器 */
    hmac_roam_connect_start_timer(roam_info, ROAM_ASSOC_TIME_MAX);
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_roam_auth_seq2_11r_process(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info,
    mac_rx_ctl_stru *rx_ctrl, osal_u8 *mac_hdr)
{
    osal_u32 ret;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || roam_info == OSAL_NULL || rx_ctrl == OSAL_NULL || mac_hdr == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_11r_vap_info->enable_11r == OAL_TRUE) {
        if (mac_get_auth_alg(mac_hdr) == WLAN_WITP_AUTH_FT) {
            /* 上报FT成功消息给APP，以便APP下发新的FT_IE用于发送reassociation */
            ret = hmac_roam_ft_notify_wpas(hmac_vap, mac_hdr, rx_ctrl->frame_len);
            if ((ret != OAL_SUCC)) {
                oam_error_log2(0, OAM_SF_ROAM,
                    "vap_id[%d] {hmac_roam_process_auth_seq2::hmac_roam_ft_notify_wpas failed[%d].}",
                    hmac_vap->vap_id, ret);
                return ret;
            }

            hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);
            /* 启动关联超时定时器 */
            hmac_roam_connect_start_timer(roam_info, ROAM_ASSOC_TIME_MAX);
            return OAL_SUCC;
        }
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_roam_ft_timeout
 功能描述  : 处理认证超时，漫游期间最多3次auth
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_ft_timeout(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32 ret;
    hmac_vap_stru *hmac_vap;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    unref_param(p_param);
    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_WAIT_FT_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_ft_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap  = roam_info->hmac_vap;
    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_11r_vap_info->enable_11r != OAL_TRUE) {
        return OAL_SUCC;
    }

    if (++roam_info->connect.ft_num >= MAX_AUTH_CNT) {
        return hmac_roam_connect_fail(roam_info);
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_INIT);

    oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_ft_timeout::change to ft over the air!}");
    roam_info->connect.ft_force_air = OAL_TRUE;
    roam_info->connect.ft_failed = OAL_TRUE;
    ret = hmac_roam_connect_ft_ds_change_to_air_etc(hmac_vap, roam_info->connect.bss_dscr);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM,
            "{hmac_roam_ft_timeout::hmac_roam_connect_ft_ds_change_to_air_etc failed[%d].}", ret);
        return hmac_roam_connect_fail(roam_info);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_ft_reassoc_etc
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_connect_ft_reassoc_etc(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru                             *roam_info;
    hmac_join_req_stru                               join_req;
    osal_u32                                       ret;
    mac_bss_dscr_stru                               *bss_dscr;
    hmac_11r_vap_info_stru  *hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);

    if (hmac_11r_vap_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    bss_dscr = roam_info->connect.bss_dscr;

    if (roam_info->hmac_user == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_mac_addr(roam_info->hmac_user->user_mac_addr, bss_dscr->bssid);
    if (memcpy_s(hmac_vap->supp_rates, bss_dscr->num_supp_rates, bss_dscr->supp_rates,
        bss_dscr->num_supp_rates) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_roam_connect_ft_reassoc_etc data::memcpy err.}");
    }
    mac_mib_set_SupportRateSetNums(hmac_vap, bss_dscr->num_supp_rates);

    if ((mac_mib_get_ft_over_ds(hmac_vap) == OAL_TRUE) && (hmac_11r_vap_info->enable_11r_over_ds == OAL_TRUE)) {
        /* 配置join参数 */
        hmac_prepare_join_req_etc(&join_req, bss_dscr);

        ret = hmac_sta_update_join_req_params_etc(hmac_vap, &join_req);
        if (ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_SCAN,
                "vap_id[%d] {hmac_roam_connect_ft_reassoc_etc::hmac_sta_update_join_req_params_etc fail[%d].}",
                hmac_vap->vap_id, ret);
            return ret;
        }
    }

    /* 发送关联请求 */
    ret = hmac_roam_send_reassoc_req(roam_info);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_connect_ft_reassoc_etc::hmac_roam_send_assoc_req failed[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_roam_reassoc_check(OAL_CONST hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = OAL_PTR_NULL;
    osal_u8 vap_id;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_reassoc_etc::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    vap_id = hmac_vap->vap_id;
    roam_info = hmac_get_roam_info(vap_id);
    if (roam_info == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_reassoc_etc::roam info null!}", vap_id);
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 漫游开关没有开时，不处理 */
    if (roam_info->enable == 0) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_reassoc_etc::roam disabled.}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    /* 主状态机为非CONNECTING状态，失败 */
    if (roam_info->main_state != ROAM_MAIN_STATE_CONNECTING) {
        oam_warning_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_reassoc_etc::roam_en_main_state=[%d] not ROAM_MAIN_STATE_CONNECTING, return.}",
            hmac_vap->vap_id,
            roam_info->main_state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    /* CONNECT状态机为非WAIT_JOIN状态，失败 */
    if (roam_info->connect.state != ROAM_CONNECT_STATE_WAIT_ASSOC_COMP) {
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_reassoc_etc::connect state[%d] error.}", hmac_vap->vap_id,
            roam_info->connect.state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_reassoc_etc
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_reassoc_etc(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru              *roam_info;
    osal_u32                        ul_ret;
    osal_u8                         vap_id;
    mac_bss_dscr_stru                *bss_dscr;
    hmac_11r_vap_info_stru  *hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);

    if (hmac_11r_vap_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_roam_reassoc_check(hmac_vap);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    vap_id = hmac_vap->vap_id;
    roam_info = hmac_get_roam_info(vap_id);
    if (mac_mib_get_ft_trainsistion(hmac_vap) == OSAL_TRUE) {
        if ((mac_mib_get_ft_over_ds(hmac_vap) == OAL_TRUE) && (hmac_11r_vap_info->enable_11r_over_ds == OAL_TRUE)) {
            /* over ds 首先pause user */
            bss_dscr = roam_info->connect.bss_dscr;
            if (bss_dscr == OAL_PTR_NULL) {
                oam_error_log1(0, OAM_SF_ROAM,
                    "vap_id[%d] {hmac_roam_reassoc_etc::bss_dscr is null.}",
                    hmac_vap->vap_id);
                return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
            }

            ul_ret = hmac_roam_pause_user_etc(hmac_vap);
            if (ul_ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ROAM,
                    "{hmac_roam_reassoc_etc::hmac_roam_reassoc_etc fail[%ld]!}", ul_ret);
                return ul_ret;
            }
        }
        oam_warning_log2(0, OAM_SF_ROAM,
                         "vap_id[%d] {hmac_roam_reassoc_etc::ft_over_ds=[%d],to send reassoc!}", vap_id,
                         mac_mib_get_ft_over_ds(hmac_vap));
        ul_ret = hmac_roam_connect_ft_reassoc_etc(hmac_vap);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ROAM,
                "vap_id[%d] {hmac_roam_reassoc_etc::hmac_roam_connect_process_ft FAIL[%d]!}", vap_id, ul_ret);
            return ul_ret;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 填充mdie
*****************************************************************************/
OAL_STATIC osal_void mac_set_md_ie_etc(hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_11r_vap_info_stru *hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);

    if (mac_mib_get_ft_trainsistion(hmac_vap) != OSAL_TRUE) {
        *ie_len = 0;
        return;
    }

    /***************************************************************************
                    ---------------------------------------------------
                    |Element ID | Length | MDID| FT Capbility & Policy|
                    ---------------------------------------------------
           Octets:  |1          | 1      | 2   |  1                   |
                    ---------------------------------------------------
    ***************************************************************************/

    *buffer = MAC_EID_MOBILITY_DOMAIN;
    *(buffer + 1) = 3;                                          /* IE内容长度3字节 */
    *(buffer + 2) = mac_mib_get_ft_mdid(hmac_vap) & 0xFF;        /* IE第2字节 */
    *(buffer + 3) = (mac_mib_get_ft_mdid(hmac_vap) >> 8) & 0xFF; /* IE第3字节. 右移8 bit */
    *(buffer + 4) = 0;                                          /* IE第4字节 */

    /***************************************************************************
                    ------------------------------------------
                    |FT over DS| RRP Capability | Reserved   |
                    ------------------------------------------
             Bits:  |1         | 1              | 6          |
                    ------------------------------------------
    ***************************************************************************/
    if (hmac_11r_vap_info->enable_11r_over_ds == OSAL_TRUE) {
        *(buffer + 4) += 1;     /* buffer偏移4字节 */
    }
    if (mac_mib_get_ft_resource_req(hmac_vap) == OSAL_TRUE) {
        *(buffer + 4) += 2;     /* buffer偏移4字节的内容设置为2 */
    }
    *ie_len = 5;    /* IE长度5字节 */
}

OAL_STATIC osal_void mac_set_rde_ie_etc(osal_u8 *buffer, osal_u8 *ie_len)
{
    /*********************************************************************************************
              -------------------------------------------------------------------
              |Element ID | Length | RDE Identifier | Res Dscr Cnt| Status Code |
              -------------------------------------------------------------------
     Octets:  |1          | 1      | 1              |  1          |   2         |
              -------------------------------------------------------------------
    ***********************************************************************************************/
    *buffer = MAC_EID_RDE;
    *(buffer + 1) = 4;  /* IE内容长度4字节 */
    *(buffer + 2) = 0;  /* IE第2字节 */
    *(buffer + 3) = 1;  /* IE第3字节 */
    *(buffer + 4) = 0;  /* IE第4字节 */

    *ie_len = 6;        /* IE长度6字节 */
}

OAL_STATIC osal_void mac_set_tspec_ie_etc(osal_u8 *buffer, osal_u8 *ie_len, osal_u8 tid)
{
    osal_u8 len = 0;
    mac_ts_info_stru *ts_info;

    /***********************************************************************************************
              --------------------------------------------------------------------------------------
              |Element ID|Length|TS Info|Nominal MSDU Size|Max MSDU Size|Min Serv Itvl|Max Serv Itvl|
              ---------------------------------------------------------------------------------------
     Octets:  |1         | 1(55)| 3     |  2              |   2         |4            |4            |
              ---------------------------------------------------------------------------------------
              | Inactivity Itvl | Suspension Itvl | Serv Start Time |Min Data Rate | Mean Data Rate |
              ---------------------------------------------------------------------------------------
     Octets:  |4                | 4               | 4               |4             |  4             |
              ---------------------------------------------------------------------------------------
              |Peak Data Rate|Burst Size|Delay Bound|Min PHY Rate|Surplus BW Allowance  |Medium Time|
              ---------------------------------------------------------------------------------------
     Octets:  |4             |4         | 4         | 4          |  2                   |2          |
              ---------------------------------------------------------------------------------------

    ***********************************************************************************************/

    *buffer = MAC_EID_TSPEC;
    *(buffer + 1) = 55; /* IE内容长度55字节 */
    len += 2;           /* buffer偏移2字节 */

    memset_s(buffer + len, *(buffer + 1), 0, *(buffer + 1));

    ts_info = (mac_ts_info_stru *)(buffer + len); // TS Info
    len += 3;           /* buffer偏移3字节 */

    ts_info->tsid = tid;
    ts_info->direction = 3; // 3 for bidirection
    ts_info->apsd = 1;
    ts_info->user_prio = tid;

    *(osal_u16 *)(buffer + len) = 0x812c; // Nominal MSDU Size
    len += 28;          /* buffer偏移28字节 */

    *(osal_u32 *)(buffer + len) = 0x0001D4C0; // Mean Data Rate
    len += 16;          /* buffer偏移16字节 */

    *(osal_u16 *)(buffer + len) = 0x3000; // Surplus BW Allowance

    *ie_len = 57;       /* IE长度57字节 */
}

OAL_STATIC osal_void hmac_11r_set_md_ie(hmac_vap_stru *hmac_vap, osal_u8 **buffer, osal_u8 *ie_len)
{
    wlan_wme_ac_type_enum_uint8 aci;
    wlan_wme_ac_type_enum_uint8 target_ac;
    osal_u8 tid;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || buffer == OSAL_NULL || ie_len == OSAL_NULL) {
        return;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL || hmac_11r_vap_info->enable_11r != OAL_TRUE) {
        return;
    }

    if (hmac_vap->reassoc_flag == OAL_FALSE) {
        mac_set_md_ie_etc(hmac_vap, *buffer, ie_len);
        *buffer += *ie_len;
    } else {
        /* Reasoc中包含RIC-Req */
        for (aci = WLAN_WME_AC_BE; aci < WLAN_WME_AC_BUTT; aci++) {
            if (mac_mib_get_qap_edca_table_mandatory(hmac_vap, aci) != 0) {
                target_ac = aci;
                tid = wlan_wme_ac_to_tid(target_ac);
                mac_set_rde_ie_etc(*buffer, ie_len);
                *buffer += *ie_len;

                mac_set_tspec_ie_etc(*buffer, ie_len, tid);
                *buffer += *ie_len;
            }
        }
    }

    return;
}

/*****************************************************************************
 功能描述 : 初始化Fast BSS Trisition信息
*****************************************************************************/
OAL_STATIC osal_u32 hmac_mib_init_ft_cfg_etc(hmac_vap_stru *hmac_vap, const osal_u8 *mde)
{
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_mib_init_ft_cfg_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->mib_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_mib_init_ft_cfg_etc::pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_mib_get_authentication_mode(hmac_vap) != WLAN_WITP_AUTH_FT) {
        mac_mib_set_ft_trainsistion(hmac_vap, OSAL_FALSE);
        return OAL_SUCC;
    }

    if ((mde == OAL_PTR_NULL) || (mde[0] != MAC_EID_MOBILITY_DOMAIN) || (mde[1] != 3)) { /* MDIE长度不是3字节 */
        mac_mib_set_ft_trainsistion(hmac_vap, OSAL_FALSE);
        return OAL_SUCC;
    }

    mac_mib_set_ft_trainsistion(hmac_vap, OSAL_TRUE);
    mac_mib_set_ft_mdid(hmac_vap, (osal_u16)((mde[3] << 8) | mde[2]));        /* MDIE第3字节左移8 bit,或上MDIE第2字节 */
    mac_mib_set_ft_over_ds(hmac_vap, ((mde[4] & BIT0) != 0) ? OSAL_TRUE : OSAL_FALSE);        /* MDIE第4字节 */
    mac_mib_set_ft_resource_req(hmac_vap, ((mde[4] & BIT1) != 0) ? OSAL_TRUE : OSAL_FALSE);   /* MDIE第4字节 */

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 获取当前的md id
*****************************************************************************/
OAL_STATIC osal_u32 mac_mib_get_md_id_etc(hmac_vap_stru *hmac_vap, osal_u16 *mdid)
{
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_mib_init_ft_cfg_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->mib_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_mib_init_ft_cfg_etc::pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_mib_get_ft_trainsistion(hmac_vap) == OAL_FALSE) {
        return OAL_FAIL;
    }

    *mdid = mac_mib_get_ft_mdid(hmac_vap);
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_set_11r_md_ft_over_ds(hmac_vap_stru *hmac_vap, osal_u8 *ies, osal_u32 ie_len)
{
    osal_u8 *ie = OSAL_NULL;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || !is_legacy_sta(hmac_vap)) {
        return;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL || hmac_11r_vap_info->enable_11r != OSAL_TRUE) {
        return;
    }

    ie = mac_find_ie_etc(MAC_EID_MOBILITY_DOMAIN, ies, (osal_s32)ie_len);
    if ((ie != OAL_PTR_NULL)) {
        *(ie + FT_CAPABILITY_OFFSET) = 0;
        if (hmac_11r_vap_info->enable_11r_over_ds == OSAL_TRUE) {
            *(ie + FT_CAPABILITY_OFFSET) += 1; // 1表示Fast Bss Transition over ds能力
        }
        if (mac_mib_get_ft_resource_req(hmac_vap) == OSAL_TRUE) {
            *(ie + FT_CAPABILITY_OFFSET) += 2; // 2表示Resource Request Protocol Capability能力
        }
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_ft_ies_etc
 功能描述  : 配置ft ies
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_set_ft_ies_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg80211_ft_ies_stru           *mac_ft_ies;
    oal_app_ie_stru                     ft_ie;
    osal_u32                          ul_ret;
    osal_u16                          md_id;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_ft_ies_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_11r_vap_info->enable_11r != OAL_TRUE) {
        return OAL_SUCC;
    }

    mac_ft_ies = (mac_cfg80211_ft_ies_stru *)msg->data;
    ul_ret = mac_mib_get_md_id_etc(hmac_vap, &md_id);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_ft_ies_etc::get_md_id fail[%d].}",
                       hmac_vap->vap_id, ul_ret);
        return (osal_s32)ul_ret;
    }

    if (md_id != mac_ft_ies->mdid) {
        oam_error_log3(0, OAM_SF_CFG,
                       "vap_id[%d] {hmac_config_set_ft_ies_etc::UNEXPECTED mdid[%d/%d].}",
                       hmac_vap->vap_id, mac_ft_ies->mdid, md_id);
        return OAL_FAIL;
    }

    ft_ie.app_ie_type   = OAL_APP_FT_IE;
    ft_ie.ie_len        = mac_ft_ies->len;
    if (memcpy_s(ft_ie.ie, sizeof(ft_ie.ie), mac_ft_ies->ie, mac_ft_ies->len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_ft_ies_etc::memcpy_s error}");
    }

    hmac_set_11r_md_ft_over_ds(hmac_vap, ft_ie.ie, ft_ie.ie_len);

    ul_ret = hmac_config_set_app_ie_to_vap_etc(hmac_vap, &ft_ie, OAL_APP_FT_IE);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_ft_ies_etc::set_app_ie FAIL[%d].}",
                       hmac_vap->vap_id, ul_ret);
        return (osal_s32)ul_ret;
    }

    oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_ft_ies_etc::set_app_ie OK LEN[%d].}",
                     hmac_vap->vap_id, mac_ft_ies->len);

    hmac_roam_reassoc_etc(hmac_vap);

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_11r_set_authentication_mode(hmac_vap_stru *hmac_vap, const mac_conn_security_stru *conn_sec)
{
    osal_u32 akm_suite;
    osal_u8 akm_type;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || conn_sec == OSAL_NULL) {
        return;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return;
    }

    if (hmac_11r_vap_info->enable_11r != OAL_TRUE) {
        return;
    }

    akm_suite = OAL_NTOH_32(conn_sec->crypto.akm_suite[0]);
    akm_type  = akm_suite & 0xFF;
    if ((akm_type == WLAN_AUTH_SUITE_FT_1X) || (akm_type == WLAN_AUTH_SUITE_FT_PSK) ||
        (akm_type == WLAN_AUTH_SUITE_FT_SHA256)) {
        mac_mib_set_authentication_mode(hmac_vap, WLAN_WITP_AUTH_FT);
    }

    return;
}

OAL_STATIC osal_void hmac_11r_mib_init_cfg(hmac_vap_stru *hmac_vap, const mac_conn_security_stru *conn_sec,
    mac_bss_dscr_stru *bss_dscr)
{
    osal_u8 *mde = OSAL_NULL;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || conn_sec == OSAL_NULL || bss_dscr == OSAL_NULL) {
        return;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return;
    }

    if (hmac_11r_vap_info->enable_11r != OAL_TRUE) {
        return;
    }

    mac_mib_set_ft_trainsistion(hmac_vap, OSAL_FALSE);
    mde = mac_find_ie_etc(MAC_EID_MOBILITY_DOMAIN, bss_dscr->mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET,
        (osal_s32)(bss_dscr->mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET));
    if (mde != OAL_PTR_NULL) {
        hmac_mib_init_ft_cfg_etc(hmac_vap, mde);
    }

    return;
}

OAL_STATIC osal_void hmac_11r_encap_auth_req_ft_ie(hmac_vap_stru *hmac_vap, osal_u8 *mgmt_frame, osal_u16 *auth_req_len)
{
    osal_u16 app_ie_len;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || mgmt_frame == OSAL_NULL || auth_req_len == OSAL_NULL) {
        return;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return;
    }

    if (hmac_11r_vap_info->enable_11r == OAL_TRUE && (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) &&
        (mac_mib_get_ft_trainsistion(hmac_vap) == OAL_TRUE)) {
        /* FT System */
        mgmt_frame[MAC_80211_FRAME_LEN] = 0x02;
        mgmt_frame[MAC_80211_FRAME_LEN + 1] = 0x00;
        mgmt_frame += *auth_req_len;

        hmac_add_app_ie_etc((osal_void *)hmac_vap, mgmt_frame, &app_ie_len, OAL_APP_FT_IE);
        *auth_req_len += app_ie_len;
        mgmt_frame += app_ie_len;
    }

    return;
}

OAL_STATIC osal_u32 hmac_11r_process_assoc_rsp(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
    osal_u32 ret;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || roam_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_11r_vap_info->enable_11r != OAL_TRUE) {
        return OAL_CONTINUE;
    }

    /* 信道跟随检查 */
    ret = hmac_check_ap_channel_follow_sta(hmac_vap, &hmac_vap->channel);
    if (ret == OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_11r_process_assoc_rsp:: ap channel change to %d}",
            hmac_vap->vap_id, hmac_vap->channel.chan_number);
    }

    if (mac_mib_get_privacyinvoked(hmac_vap) != OAL_TRUE) {
        return OAL_CONTINUE;
    }

    if (mac_mib_get_ft_trainsistion(hmac_vap) == OAL_TRUE) {
        /* FT情况下，漫游成功 */
        hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_HANDSHAKING);
        hmac_roam_connect_succ(roam_info, OAL_PTR_NULL);
        return OAL_SUCC;
    }

    return OAL_CONTINUE;
}

OAL_STATIC osal_u32 hmac_11r_roam_connect_start_etc(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info,
    mac_bss_dscr_stru *bss_dscr)
{
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || roam_info == OSAL_NULL || bss_dscr == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_11r_vap_info->enable_11r == OAL_TRUE) {
        if ((mac_mib_get_ft_trainsistion(hmac_vap) == OAL_TRUE) &&
            (mac_mib_get_ft_over_ds(hmac_vap) == OAL_TRUE) &&
            (hmac_11r_vap_info->enable_11r_over_ds == OAL_TRUE) &&
            (roam_info->connect.ft_force_air != OAL_TRUE)) {
            return hmac_roam_connect_fsm_action_etc(roam_info, ROAM_CONNECT_FSM_EVENT_FT_OVER_DS,
                (osal_void *)bss_dscr);
        }
    }

    return OAL_CONTINUE;
}

OAL_STATIC osal_void hmac_11r_roam_connect_to_bss(hmac_vap_stru *hmac_vap, const hmac_roam_info_stru *roam_info,
    oal_bool_enum_uint8 *need_to_stop_user)
{
    hmac_11r_vap_info_stru  *hmac_11r_vap_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || roam_info == OSAL_NULL || need_to_stop_user == OSAL_NULL) {
        return;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return;
    }

    if ((mac_mib_get_ft_trainsistion(hmac_vap) == OAL_TRUE) && (mac_mib_get_ft_over_ds(hmac_vap) == OAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_11r_roam_connect_to_bss::11r_over_ds = [%d]}",
            hmac_11r_vap_info->enable_11r_over_ds);
        /* modify仅第一次使用ds方式 */
        if ((hmac_11r_vap_info->enable_11r_over_ds == OAL_TRUE) && (roam_info->connect.ft_force_air != OAL_TRUE)) {
            *need_to_stop_user = OSAL_FALSE;
        }
    }

    return;
}

OAL_STATIC osal_void hmac_11r_roam_change_app_ie(hmac_vap_stru *hmac_vap, app_ie_type_uint8 *app_ie_type)
{
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || app_ie_type == OSAL_NULL) {
        return;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return;
    }

    if (hmac_11r_vap_info->enable_11r == OAL_TRUE) {
        if (mac_mib_get_ft_trainsistion(hmac_vap) == OAL_TRUE) {
            *app_ie_type = OAL_APP_FT_IE;
        }
    }

    return;
}

OAL_STATIC osal_void hmac_11r_roam_connect_fsm_init(
    hmac_roam_fsm_func hmac_roam_connect_fsm_func[][ROAM_CONNECT_FSM_EVENT_TYPE_BUTT])
{
    if (hmac_roam_connect_fsm_func == OSAL_NULL) {
        return;
    }

    hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_INIT][ROAM_CONNECT_FSM_EVENT_FT_OVER_DS]           =
        hmac_roam_send_ft_req;
    hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_FT_COMP][ROAM_CONNECT_FSM_EVENT_MGMT_RX]      =
        hmac_roam_process_ft_rsp;
    hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_FT_COMP][ROAM_CONNECT_FSM_EVENT_TIMEOUT]      =
        hmac_roam_ft_timeout;
}

OAL_STATIC osal_void hmac_get_11r_cap(hmac_vap_stru *hmac_vap, osal_s32 *cap_value)
{
    osal_u8 vap_id;
    osal_u32 val = 0;

    if (hmac_vap == OSAL_NULL || cap_value == OSAL_NULL) {
        return;
    }
    val = (osal_u32)(*cap_value);

    if (!is_legacy_sta(hmac_vap)) {
        return;
    }

    vap_id = hmac_vap->vap_id;
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return;
    }

    if (g_11r_vap_info[vap_id] != OSAL_NULL && g_11r_vap_info[vap_id]->enable_11r == OAL_TRUE) {
        val |=  BIT(WAL_WIFI_FEATURE_SUPPORT_11R);
        *cap_value = (osal_s32)val;
    }

    return;
}

OAL_STATIC osal_u32 hmac_sta_up_rx_action_ft_transition(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    hmac_roam_info_stru *roam_info = OSAL_NULL;
    hmac_11r_vap_info_stru *hmac_11r_vap_info = OSAL_NULL;

    if (netbuf == OSAL_NULL || hmac_vap == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    if (roam_info == OAL_PTR_NULL) {
        return OAL_CONTINUE;
    }

    /* 漫游开关没有开时，不处理管理帧接收 */
    if (roam_info->enable == 0) {
        return OAL_CONTINUE;
    }

    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(*netbuf); /* 获取帧头信息 */
    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    data = oal_netbuf_rx_data(*netbuf);

    if (!is_legacy_sta(hmac_vap)) {
        return OAL_CONTINUE;
    }

    hmac_11r_vap_info = hmac_11r_get_vap_info(hmac_vap);
    if (hmac_11r_vap_info == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    if (hmac_11r_vap_info->enable_11r != OAL_TRUE) {
        return OAL_CONTINUE;
    }

    if (frame_hdr->frame_control.type != WLAN_MANAGEMENT || frame_hdr->frame_control.sub_type != WLAN_ACTION) {
        return OAL_CONTINUE;
    }

    if ((data[MAC_ACTION_OFFSET_CATEGORY]) != MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION ||
        data[MAC_ACTION_OFFSET_ACTION] != MAC_FT_ACTION_RESPONSE) {
        return OAL_CONTINUE;
    }

    hmac_roam_connect_fsm_action_etc(roam_info, ROAM_CONNECT_FSM_EVENT_MGMT_RX, (osal_void *)*netbuf);

    return OAL_SUCC;
}

hmac_netbuf_hook_stru g_11r_netbuf_hook = {
    .hooknum = HMAC_FRAME_MGMT_RX_EVENT_FEATURE,
    .priority = HMAC_HOOK_PRI_HIGHEST,
    .hook_func = hmac_sta_up_rx_action_ft_transition,
};

osal_u32 hmac_11r_init(osal_void)
{
    osal_u32 ret;

    /* 注册监听 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_11r_add_vap);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_11r_del_vap);
    /* 注册消息 */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_FT_IES, hmac_config_set_ft_ies_etc);
    /* 注册对外接口 */
    hmac_feature_hook_register(HMAC_FHOOK_11R_SET_MD_IE, hmac_11r_set_md_ie);
    hmac_feature_hook_register(HMAC_FHOOK_11R_SET_AUTHENTICATION_MODE, hmac_11r_set_authentication_mode);
    hmac_feature_hook_register(HMAC_FHOOK_11R_MIB_INIT_CFG, hmac_11r_mib_init_cfg);
    hmac_feature_hook_register(HMAC_FHOOK_11R_ENCAP_AUTH_REQ_FT_IE, hmac_11r_encap_auth_req_ft_ie);
    hmac_feature_hook_register(HMAC_FHOOK_11R_AUTH_SEQ2_PROCESS, hmac_roam_auth_seq2_11r_process);
    hmac_feature_hook_register(HMAC_FHOOK_11R_PROCESS_ASSOC_RSP, hmac_11r_process_assoc_rsp);
    hmac_feature_hook_register(HMAC_FHOOK_11R_ROAM_CONNECT_START_ETC, hmac_11r_roam_connect_start_etc);
    hmac_feature_hook_register(HMAC_FHOOK_11R_ROAM_CONNECT_TO_BSS, hmac_11r_roam_connect_to_bss);
    hmac_feature_hook_register(HMAC_FHOOK_11R_ROAM_CHANGE_APP_IE, hmac_11r_roam_change_app_ie);
    hmac_feature_hook_register(HMAC_FHOOK_11R_ROAM_CONNECT_FSM_INIT, hmac_11r_roam_connect_fsm_init);
    hmac_feature_hook_register(HMAC_FHOOK_11R_GET_11R_CAP, hmac_get_11r_cap);
    hmac_feature_hook_register(HMAC_FHOOK_11R_SET_MD_FT_OVER_DS, hmac_set_11r_md_ft_over_ds);
    /* 注册转发Hook */
    ret = hmac_register_netbuf_hook(&g_11r_netbuf_hook);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_11k_init:: MGMT RX IN register_netbuf_hooks error!");
        return ret;
    }

    return OAL_SUCC;
}

osal_void hmac_11r_deinit(osal_void)
{
    /* 去注册监听 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_11r_add_vap);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_11r_del_vap);
    /* 去注册消息 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_FT_IES);
    /* 去注册对外接口 */
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_SET_MD_IE);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_SET_AUTHENTICATION_MODE);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_MIB_INIT_CFG);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_ENCAP_AUTH_REQ_FT_IE);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_AUTH_SEQ2_PROCESS);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_PROCESS_ASSOC_RSP);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_ROAM_CONNECT_START_ETC);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_ROAM_CONNECT_TO_BSS);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_ROAM_CHANGE_APP_IE);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_ROAM_CONNECT_FSM_INIT);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_GET_11R_CAP);
    hmac_feature_hook_unregister(HMAC_FHOOK_11R_SET_MD_FT_OVER_DS);
    /* 去注册转发Hook */
    hmac_unregister_netbuf_hook(&g_11r_netbuf_hook);

    return;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

