/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description:
 * Date: 2020-07-07
 */
#include "hmac_slp.h"
#include "mac_resource_ext.h"
#include "mac_ie.h"
#include "mac_device_ext.h"
#include "oal_mem_hcm.h"
#include "oal_netbuf_data.h"
#include "hmac_tx_mgmt.h"
#include "hmac_user.h"
#include "hmac_11i.h"
#include "msg_slp_rom.h"
#include "hmac_hook.h"
#include "hmac_ccpriv.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SLP_C
#undef  THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

osal_bool g_slp_switch = OSAL_FALSE;
osal_u8 frame_data[WLAN_SHORT_NETBUF_SIZE];
osal_u8 g_gtk_idx[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT] = {
    WLAN_SLP_GTK_IDX_INVAILD, WLAN_SLP_GTK_IDX_INVAILD, WLAN_SLP_GTK_IDX_INVAILD, WLAN_SLP_GTK_IDX_INVAILD
};

/*****************************************************************************
 函 数 名  : hmac_get_slp_switch_status
 功能描述  : 获取SLP特性是否开启
*****************************************************************************/
OSAL_STATIC osal_bool hmac_get_slp_switch_status(osal_void)
{
    return g_slp_switch;
}

/*****************************************************************************
 函 数 名  : hmac_set_slp_gtk_idx
 功能描述  : 设置vap的gtk密钥索引（用于添加或者删除key索引）
*****************************************************************************/
OSAL_STATIC osal_bool hmac_slp_add_gtk_idx(osal_void *notify_data)
{
    hmac_add_gtk_notify_stru *add_gtk_data = (hmac_add_gtk_notify_stru *)notify_data;
    wlan_ciper_protocol_type_enum_uint8 cipher_type = add_gtk_data->cipher_type;
    osal_u8 vap_id = add_gtk_data->vap_id;
    osal_u8 gtk_idx = add_gtk_data->key_idx;

    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        oam_error_log1(0, OAM_SF_SLP, "{hmac_slp_add_gtk_idx:: vap id[%d] is error}", vap_id);
        return OSAL_FALSE;
    }

    if ((cipher_type == WLAN_80211_CIPHER_SUITE_BIP) ||
        (cipher_type == WLAN_80211_CIPHER_SUITE_BIP_GMAC_128) ||
        (cipher_type == WLAN_80211_CIPHER_SUITE_BIP_GMAC_256) ||
        (cipher_type == WLAN_80211_CIPHER_SUITE_BIP_CMAC_256)) {
        return OSAL_TRUE;
    }

    oam_warning_log3(0, OAM_SF_SLP, "{hmac_slp_add_gtk_idx:: set vap[%d] new gtk[%d] old gtk[%d]}", vap_id, gtk_idx,
        g_gtk_idx[vap_id]);

    g_gtk_idx[vap_id] = gtk_idx;

    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_slp_del_gtk_idx(osal_void *notify_data)
{
    hmac_del_gtk_notify_stru *del_gtk_data = (hmac_add_gtk_notify_stru *)notify_data;
    wlan_ciper_protocol_type_enum_uint8 cipher_type = del_gtk_data->cipher_type;
    osal_u8 vap_id = del_gtk_data->vap_id;

    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        oam_error_log1(0, OAM_SF_SLP, "{hmac_slp_del_gtk_idx:: vap id[%d] is error}", vap_id);
        return OSAL_FALSE;
    }

    if ((cipher_type == WLAN_80211_CIPHER_SUITE_BIP) ||
        (cipher_type == WLAN_80211_CIPHER_SUITE_BIP_GMAC_128) ||
        (cipher_type == WLAN_80211_CIPHER_SUITE_BIP_GMAC_256) ||
        (cipher_type == WLAN_80211_CIPHER_SUITE_BIP_CMAC_256)) {
        return OSAL_TRUE;
    }
    oam_warning_log2(0, OAM_SF_SLP, "{hmac_slp_del_gtk_idx::vap[%d] gtk[%d]}", vap_id, g_gtk_idx[vap_id]);

    g_gtk_idx[vap_id] = WLAN_SLP_GTK_IDX_INVAILD;

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_set_slp_gtk_idx
 功能描述  : 获取vap的gtk密钥索引
*****************************************************************************/
OSAL_STATIC osal_void hmac_get_slp_gtk_idx(osal_u8 vap_id, osal_u8 *gtk_idx)
{
    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        oam_error_log1(0, OAM_SF_SLP, "{hmac_get_slp_gtk_idx:: vap id[%d] is error}", vap_id);
        return;
    }

    *gtk_idx = g_gtk_idx[vap_id];
    oam_warning_log2(0, OAM_SF_SLP, "{hmac_get_slp_gtk_idx:: get vap[%d] gtk[%d]}", vap_id, *gtk_idx);
}

/*****************************************************************************
 函 数 名  : hmac_slp_encap_cts
 功能描述  : 组ba request帧
*****************************************************************************/
OSAL_STATIC osal_u16 hmac_slp_encap_cts(const hmac_vap_stru *hmac_vap, osal_u8 *frame_buf,
    osal_u8 *mac_addr, osal_u16 duration)
{
    osal_u8 *mac_hdr = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || frame_buf == OSAL_NULL || mac_addr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SLP, "{hmac_slp_encap_cts:: ptr is null}");
        return 0;
    }

    mac_hdr = frame_buf + OAL_MAX_CB_LEN;

    /*************************************************************************/
    /* Head: Frame Control+Duration+DA                                       */
    /*                     SLP CTS Frame Format                     */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|          DA                           |FCS|  */
    /* |             |        |                                       |   |  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |          6                            |4  |  */
    /* --------------------------------------------------------------------  */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(mac_hdr, (osal_u16)WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_CTL | WLAN_FC0_SUBTYPE_CTS);

    mac_hdr_set_duration(mac_hdr, duration);

    /* 4表示将指针定位到DA,Set DA to the address of the STA requesting authentication */
    oal_set_mac_addr(mac_hdr + WLAN_HDR_ADDR1_OFFSET, mac_addr);

    return WLAN_MAX_CTS_LEN;
}

/*****************************************************************************
 函 数 名  : hmac_slp_cts_set_tx_ctrl
 功能描述  : 设置cts帧的tx_ctrl
*****************************************************************************/
OSAL_STATIC osal_void hmac_slp_cts_set_tx_ctrl(const hmac_vap_stru *hmac_vap, mac_tx_ctl_stru *tx_ctl)
{
    /* 避免节能，将用户索引置为INVAILD             */
    tx_ctl->tx_user_idx = MAC_INVALID_USER_ID;
    tx_ctl->ac  = WLAN_WME_AC_MGMT;

    tx_ctl->retried_num = 0;
    tx_ctl->frame_header_length = WLAN_MAX_CTS_LEN;
    tx_ctl->netbuf_num = 1;
    tx_ctl->mpdu_payload_len = 0;
    tx_ctl->duration_hw_bypass = 1;
    tx_ctl->slp_frame_id = MAC_SLP_CTS_FRAME;

    /* set mpdu num & seq num */
    tx_ctl->mpdu_num = 1;
    tx_ctl->msdu_num = 1;
    tx_ctl->tx_vap_index = hmac_vap->vap_id;
    if (hmac_vap->hal_vap != OSAL_NULL) {
        tx_ctl->tx_hal_vap_index = hmac_vap->hal_vap->vap_id;
    }

    tx_ctl->frame_type = WLAN_CB_FRAME_TYPE_MGMT;
}

/*****************************************************************************
 函 数 名  : hmac_slp_send_cts
 功能描述  : 发送cts帧（host主要做封装操作）
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_slp_send_cts(hmac_vap_stru *hmac_vap, osal_u16 duration, osal_u8 *cst_frame)
{
    osal_u16         cts_len;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)cst_frame;

    cts_len = hmac_slp_encap_cts(hmac_vap, cst_frame, mac_mib_get_station_id(hmac_vap), duration);
    if (cts_len == 0) {
        oam_error_log1(0, OAM_SF_SLP, "vap_id[%d] {hmac_slp_send_cts::cts_len=0.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    hmac_slp_cts_set_tx_ctrl(hmac_vap, tx_ctl);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_slp_encap_rts
 功能描述  : 组rts帧
*****************************************************************************/
OSAL_STATIC osal_u16 hmac_slp_encap_rts(const hmac_vap_stru *hmac_vap, osal_u8 *frame_buf,
    osal_u8 *mac_addr, osal_u16 duration)
{
    osal_u8 *mac_hdr = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || frame_buf == OSAL_NULL || mac_addr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SLP, "{hmac_slp_encap_rts:: ptr is null}");
        return 0;
    }

    mac_hdr = frame_buf + OAL_MAX_CB_LEN;

    /*************************************************************************/
    /* Head: Frame Control+Duration+DA+RA                                    */
    /*                     SLP RTS Frame Format                              */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|          DA          |       RA       |FCS|  */
    /* |             |        |                      |                |   |  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |           6          |        6       |4  |  */
    /* --------------------------------------------------------------------  */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(mac_hdr, (osal_u16)WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_CTL | WLAN_FC0_SUBTYPE_RTS);

    mac_hdr_set_duration(mac_hdr, duration);

    /* 4表示将指针定位到DA,Set DA to the address of the STA requesting authentication */
    oal_set_mac_addr(mac_hdr + WLAN_HDR_ADDR1_OFFSET, mac_addr);

    /* 4表示将指针定位到RA,Set RA to the address of the STA requesting authentication */
    oal_set_mac_addr(mac_hdr + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    return WLAN_MAX_RTS_LEN;
}

/*****************************************************************************
 函 数 名  : hmac_slp_rts_set_tx_ctrl
 功能描述  : 设置rts帧的tx_ctrl
*****************************************************************************/
OSAL_STATIC osal_void hmac_slp_rts_set_tx_ctrl(const hmac_vap_stru *hmac_vap, mac_tx_ctl_stru *tx_ctl)
{
    /* 避免节能，将用户索引置为INVAILD             */
    tx_ctl->tx_user_idx = MAC_INVALID_USER_ID;
    tx_ctl->ac  = WLAN_WME_AC_MGMT;

    tx_ctl->retried_num = 0;
    tx_ctl->frame_header_length = WLAN_MAX_RTS_LEN;
    tx_ctl->netbuf_num = 1;
    tx_ctl->mpdu_payload_len = 0;
    tx_ctl->duration_hw_bypass = 1;
    tx_ctl->slp_frame_id = MAC_SLP_GPIO_FRAME; /* 设置脉冲帧的标识 */
    tx_ctl->frame_type = WLAN_CB_FRAME_TYPE_MGMT;

    /* set mpdu num & seq num */
    tx_ctl->mpdu_num = 1;
    tx_ctl->msdu_num = 1;
    tx_ctl->tx_vap_index = hmac_vap->vap_id;
    if (hmac_vap->hal_vap != OSAL_NULL) {
        tx_ctl->tx_hal_vap_index = hmac_vap->hal_vap->vap_id;
    }
}

/*****************************************************************************
 函 数 名  : hmac_slp_send_rts_gpio
 功能描述  : 发送rts脉冲帧
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_slp_send_rts_gpio(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u16 duration,
    osal_u8 *rst_frame)
{
    osal_u16         rts_len;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)rst_frame;

    rts_len = hmac_slp_encap_rts(hmac_vap, rst_frame, hmac_user->user_mac_addr, duration);
    if (rts_len == 0) {
        oam_error_log1(0, OAM_SF_SLP, "vap_id[%d] {hmac_slp_send_rts::frame_len=0.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    hmac_slp_rts_set_tx_ctrl(hmac_vap, tx_ctl);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_slp_send_cts_gpio
 功能描述  : 发送cts脉冲帧
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_slp_send_cts_gpio(osal_u8 *src_frame, osal_u8 *gpio_frame)
{
    errno_t         ret;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)gpio_frame;

    ret = memcpy_s(gpio_frame, OAL_MAX_CB_LEN + OAL_MAX_MAC_HDR_LEN, src_frame, OAL_MAX_CB_LEN + OAL_MAX_MAC_HDR_LEN);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_SLP, "{hmac_slp_send_cts_gpio::memcpy fail.}");
    }
    /* 设置脉冲帧的标识 */
    tx_ctl->slp_frame_id = MAC_SLP_GPIO_FRAME;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_slp_encap_rm_request
 功能描述  : 组slp测距协商帧
*****************************************************************************/
OSAL_STATIC osal_u16 hmac_slp_encap_rm_request(hmac_vap_stru *hmac_vap, const osal_u8 *mac_addr,
    osal_u8 *frame_buf, const osal_u8 *data, osal_u16 data_len)
{
    osal_u8      *mac_header = OAL_PTR_NULL;
    osal_u8      *payload_addr = OAL_PTR_NULL;
    osal_u16     *payload_temp = OAL_PTR_NULL;
    errno_t      ret;
    osal_u32      idx = 0;

    mac_header = frame_buf + OAL_MAX_CB_LEN;
    payload_addr = mac_header + OAL_MAX_MAC_HDR_LEN;

    /*************************************************************************/
    /* Management Frame Format */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    /*************************************************************************/
    /* Set the fields in the frame header */
    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /* 设置地址1，与STA连接的AP MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, mac_addr);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置地址3，为VAP的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    payload_addr[idx++] = MAC_ACTION_CATEGORY_VENDOR;
    payload_addr[idx++] = ((osal_u32)MAC_WLAN_OUI_PUBLIC >> NUM_16_BITS) & 0xff;
    payload_addr[idx++] = ((osal_u32)MAC_WLAN_OUI_PUBLIC >> NUM_8_BITS)  & 0xff;
    payload_addr[idx++] = ((osal_u32)MAC_WLAN_OUI_PUBLIC) & 0xff;
    payload_addr[idx++] = MAC_SLP_RM_IE;
    payload_addr[idx++] = SLP_RM_REQUEST; /* subtype not used */

    payload_temp = (osal_u16 *)&payload_addr[idx];
    *payload_temp = data_len;
    idx = idx + 2; /* 采用u16保存数据长度，往后偏移2个u8 */

    ret = memcpy_s(&payload_addr[idx], data_len, data, data_len);
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_SLP, "{hmac_slp_encap_rm_request::memory copy failed[%d].}", ret);
        return 0;
    }

    return (osal_u16)(MAC_80211_FRAME_LEN + idx + data_len);
}

/*****************************************************************************
 函 数 名  : hmac_slp_rm_action_set_tx_ctrl
 功能描述  : 设置SLP协商帧的tx_ctrl
*****************************************************************************/
OSAL_STATIC osal_void hmac_slp_rm_action_set_tx_ctrl(const hmac_vap_stru* hmac_vap, mac_tx_ctl_stru *tx_ctl,
    osal_u16 user_idx, osal_u16 frame_len)
{
    tx_ctl->tx_user_idx = (osal_u8)user_idx;
    tx_ctl->ac  = WLAN_WME_AC_MGMT;

    tx_ctl->retried_num = 0;
    tx_ctl->frame_header_length = MAC_80211_FRAME_LEN;
    tx_ctl->netbuf_num = 1;
    tx_ctl->mpdu_payload_len = frame_len - MAC_80211_FRAME_LEN;
    tx_ctl->slp_frame_id = MAC_SLP_RM_ACTION_FRAME;

    tx_ctl->frame_type = WLAN_CB_FRAME_TYPE_MGMT;

    /* slp的管理帧默认不加密 */
    tx_ctl->cipher_key_type = WLAN_KEY_TYPE_TX_GTK;
    tx_ctl->cipher_protocol_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;
    tx_ctl->cipher_key_id = 0;

    /* set mpdu num & seq num */
    tx_ctl->mpdu_num = 1;
    tx_ctl->msdu_num = 1;
    tx_ctl->tx_vap_index = hmac_vap->vap_id;
    if (hmac_vap->hal_vap != OSAL_NULL) {
        tx_ctl->tx_hal_vap_index = hmac_vap->hal_vap->vap_id;
    }

    tx_ctl->ismcast = (hmac_vap->multi_user_idx == user_idx) ? OSAL_TRUE : OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_slp_send_rm_request
 功能描述  : 发送SLP协商帧
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_slp_send_rm_request(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *data, osal_u16 data_len, osal_u8 *rm_request_frame)
{
    osal_u16         frame_len;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)rm_request_frame;
    const osal_u8 *mac_addr = BROADCAST_MACADDR;
    osal_u16 user_idx;

    user_idx = hmac_vap->multi_user_idx;
    if (hmac_user != OSAL_NULL) {
        mac_addr = hmac_user->user_mac_addr;
        user_idx = hmac_user->assoc_id;
    }

    /* 帧长度不能超过申请的netbuf长度 */
    frame_len = hmac_slp_encap_rm_request(hmac_vap, mac_addr, rm_request_frame, data, data_len);
    if (frame_len == 0) {
        oam_error_log1(0, OAM_SF_SLP, "vap_id[%d] {hmac_slp_send_rm_request::frame_len=0.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    hmac_slp_rm_action_set_tx_ctrl(hmac_vap, tx_ctl, user_idx, frame_len);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_handle_slp_rm_request
 功能描述  : 处理SLP协商请求（发送参数准备）
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_handle_slp_rm_request(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 rm_type)
{
    mac_slp_rm_stru slp_rm;
    osal_u16 i;
    frw_msg msg_to_device = {0};
    osal_u32 ret;

    memset_s(&msg_to_device, sizeof(frw_msg), 0, sizeof(frw_msg));

    for (i = 0; i < WLAN_SHORT_NETBUF_SIZE; i++) {
        frame_data[i] = 1;
    }

    if (rm_type > BROADCAST) {
        return OAL_FAIL;
    }

    oam_warning_log1(0, OAM_SF_ANY, "hmac_handle_slp_rm_request: start rm request[%d]", rm_type);

    memset_s(&slp_rm, sizeof(slp_rm), 0, sizeof(slp_rm));
    slp_rm.rm_type = rm_type;
    slp_rm.slp_timer_val = MAC_SLP_SESSION_TIME;

    if (rm_type == BROADCAST) {
        /* ms转化为us * 1000 */
        hmac_slp_send_cts(hmac_vap, MAC_SLP_CTS_RTS_TIME * 1000, &slp_rm.slp_cts_data[0]);
        hmac_slp_send_rm_request(hmac_vap, OSAL_NULL, frame_data, sizeof(frame_data), &slp_rm.slp_rm_request_data[0]);
        hmac_slp_send_cts_gpio(&slp_rm.slp_cts_data[0], &slp_rm.slp_gpio_data[0]);
    } else if (rm_type == UNICAST) {
        /* ms转化为us * 1000 */
        hmac_slp_send_cts(hmac_vap, MAC_SLP_CTS_RTS_TIME * 1000, &slp_rm.slp_cts_data[0]);
        hmac_slp_send_rm_request(hmac_vap, hmac_user, frame_data, sizeof(frame_data), &slp_rm.slp_rm_request_data[0]);
        /* ms转化为us * 1000 */
        hmac_slp_send_rts_gpio(hmac_vap, hmac_user, MAC_SLP_CTS_RTS_TIME * 1000, &slp_rm.slp_gpio_data[0]);
    }

    frw_msg_init((osal_u8 *)&slp_rm, sizeof(slp_rm), OSAL_NULL, 0, &msg_to_device);
    ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_SLP_RM_START, &msg_to_device,
        OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SLP, "{hmac_handle_slp_rm_request:: send msg to device fail=%d!", ret);
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_slp_tx_disable
 功能描述  : 获取当前SLP运行状态
*****************************************************************************/
OSAL_STATIC osal_bool hmac_slp_tx_disable(osal_void)
{
    if (hmac_get_slp_switch_status() == OSAL_FALSE) {
        return OSAL_FALSE;
    }

    return (osal_bool)hal_gp_get_slp_tx_ctrl();
}

/*****************************************************************************
 函 数 名  : hmac_rx_slp_action
 功能描述  : 处理接收到的slp action帧
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_rx_slp_action(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    dmac_rx_ctl_stru *rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(*netbuf); /* 获取帧头信息 */
    osal_u8 *frame_hdr = (osal_u8 *)mac_get_rx_cb_mac_hdr(&(rx_ctrl->rx_info));
    slp_nb_payload_and_cfo_req slp_req;
    mac_slp_action_hdr_stru *slp_action_hdr = OSAL_NULL;
    osal_u8 frame_type = mac_get_frame_type(frame_hdr);
    osal_u8 frame_sub_type = mac_get_frame_sub_type(frame_hdr);

    if (hmac_get_slp_switch_status() == OSAL_FALSE) {
        return OAL_CONTINUE;
    }

    if ((hmac_vap->vap_state != MAC_VAP_STATE_UP) && (hmac_vap->vap_state != MAC_VAP_STATE_PAUSE)) {
        return OAL_CONTINUE;
    }

    if ((frame_type != WLAN_FC0_TYPE_MGT) || (frame_sub_type != WLAN_FC0_SUBTYPE_ACTION)) {
        return OAL_CONTINUE;
    }

    slp_action_hdr = (mac_slp_action_hdr_stru *)oal_netbuf_rx_data(*netbuf);
    if (slp_action_hdr->category != MAC_ACTION_CATEGORY_VENDOR) {
        return OAL_CONTINUE;
    }

    memset_s(&slp_req, sizeof(slp_req), 0, sizeof(slp_req));

    switch (slp_action_hdr->oui_sub_type) {
        case SLP_RM_REQUEST:
        case SLP_RM_RESULT_REPORT:
            slp_req.cfo_req.cfo = rx_ctrl->rx_statistic.phase_incr;
            /* 适配后需要申请内存的方式，并拷贝数据 */
            slp_req.payload_req.payload = (osal_u8 *)slp_action_hdr + sizeof(mac_slp_action_hdr_stru);
            slp_req.payload_req.payload_len = slp_action_hdr->len;
            oam_warning_log3(0, OAM_SF_ANY, "hmac_rx_slp_action:type[%d] cfo[%d] payload_len[%d]",
                slp_action_hdr->oui_sub_type, slp_req.cfo_req.cfo, slp_req.payload_req.payload_len);
            break;
        default:
            break;
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_slp_encap_rm_result
 功能描述  : 组slp测距报告帧
*****************************************************************************/
OSAL_STATIC osal_u16 hmac_slp_encap_rm_result(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr,
    oal_netbuf_stru *netbuf, const osal_u8 *data, osal_u16 data_len)
{
    osal_u8      *mac_header = OAL_PTR_NULL;
    osal_u8      *payload = OAL_PTR_NULL;
    osal_u16     *payload_temp = OAL_PTR_NULL;
    errno_t      ret;
    osal_u32      idx = 0;

    mac_header = oal_netbuf_header(netbuf);
    payload = oal_netbuf_data_offset(netbuf, MAC_80211_FRAME_LEN);

    /*************************************************************************/
    /* Management Frame Format */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    /*************************************************************************/
    /* Set the fields in the frame header */
    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /* 设置地址1，与STA连接的AP MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, mac_addr);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置地址3，为VAP的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    payload[idx++] = MAC_ACTION_CATEGORY_VENDOR;
    payload[idx++] = ((osal_u32)MAC_WLAN_OUI_PUBLIC >> NUM_16_BITS) & 0xff;
    payload[idx++] = ((osal_u32)MAC_WLAN_OUI_PUBLIC >> NUM_8_BITS)  & 0xff;
    payload[idx++] = ((osal_u32)MAC_WLAN_OUI_PUBLIC) & 0xff;
    payload[idx++] = MAC_SLP_RM_IE;
    payload[idx++] = SLP_RM_RESULT_REPORT; /* subtype not used */

    payload_temp = (osal_u16 *)&payload[idx];
    *payload_temp = data_len;
    idx = idx + 2; /* 采用u16保存数据长度，往后偏移2个u8 */

    ret = memcpy_s(&payload[idx], data_len, data, data_len);
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_SLP, "{hmac_slp_encap_rm_result::memory copy failed[%d].}", ret);
        return 0;
    }

    return (osal_u16)(MAC_80211_FRAME_LEN + idx + data_len);
}

/*****************************************************************************
 函 数 名  : hmac_handle_slp_rm_result
 功能描述  : 处理SLP测距结果报告请求（发送参数准备）
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_handle_slp_rm_result(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    oal_netbuf_stru *netbuf = OSAL_NULL;
    osal_u16 i;
    osal_u32 ret;
    osal_u16 frame_len = 0;
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;

    for (i = 0; i < WLAN_SHORT_NETBUF_SIZE; i++) {
        frame_data[i] = 0xa;
    }

    oam_warning_log0(0, OAM_SF_SLP, "hmac_handle_slp_rm_result: start report rm result");

    /* 申请管理帧内存 */
    netbuf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_SMGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (netbuf == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SLP, "vap_id[%d] {hmac_handle_slp_rm_result::netbuf null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    frame_len = hmac_slp_encap_rm_result(hmac_vap, hmac_user->user_mac_addr, netbuf, frame_data, sizeof(frame_data));
    if (frame_len == 0) {
        oam_error_log1(0, OAM_SF_SLP, "vap_id[%d] {hmac_handle_slp_rm_result::frame_len=0.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);

    tx_ctl->tx_user_idx = (osal_u8)hmac_user->assoc_id;
    tx_ctl->ac  = WLAN_WME_AC_MGMT;
    //  发送需要填写tx_vap_index
    tx_ctl->tx_vap_index = hmac_vap->vap_id;
    tx_ctl->retried_num = 0;
    tx_ctl->mpdu_num = 1;
    tx_ctl->netbuf_num = 1;
    tx_ctl->mpdu_payload_len = frame_len;
    tx_ctl->slp_frame_id = MAC_SLP_REPORT_ACTION_FRAME; /* SLP结果上报帧走正常的发送流程 */
    tx_ctl->frame_type = WLAN_CB_FRAME_TYPE_MGMT;

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, netbuf, frame_len, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_slp_send_rm_request_test
 功能描述  : 发送SLP测距请求（发送参数准备）
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_slp_send_rm_request_test(hmac_vap_stru *hmac_vap, slp_nb_payload_and_device_req *rm_req)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    slp_communication_mode communication_mode;
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN] = { 0 };
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (hmac_vap == OSAL_NULL || rm_req == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_get_slp_switch_status() == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_SLP, "hmac_slp_send_rm_request_test: SLP NOT ENABLE");
        return OAL_FAIL;
    }

    communication_mode = rm_req->device_para.communication_mode;

    if (hmac_device_calc_up_vap_num_etc(hmac_device) > 1) {
        oam_warning_log0(0, OAM_SF_SLP, "{hmac_slp_send_rm_request::not support more than one vap up slp rm");
        return OAL_FAIL;
    }

    if (hmac_vap->p2p_mode != WLAN_LEGACY_VAP_MODE) {
        oam_warning_log1(0, OAM_SF_SLP, "vap_id[%d]{hmac_slp_send_rm_request:p2p not support SLP RM", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    if (hmac_vap->vap_state != MAC_VAP_STATE_UP && hmac_vap->vap_state != MAC_VAP_STATE_PAUSE) {
        oam_warning_log1(0, OAM_SF_SLP, "{hmac_slp_send_rm_request::device not assoc[%d]", hmac_vap->vap_state);
        return OAL_FAIL;
    }

    memcpy_s(mac_addr, WLAN_MAC_ADDR_LEN, rm_req->device_para.addr, WLAN_MAC_ADDR_LEN);

    switch (communication_mode) {
        case UNICAST:
            hmac_user = hmac_vap_get_user_by_addr_etc(hmac_vap, mac_addr, sizeof(mac_addr));
            if (hmac_user != OSAL_NULL) {
                hmac_handle_slp_rm_request(hmac_vap, hmac_user, UNICAST);
            } else {
                oam_warning_log4(0, OAM_SF_SLP, "hmac_slp_send_rm_request_test: user not found[%x:%x:%x:%x:x:x]",
                    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 打印mac地址0:1:2:3地址 */
                return OAL_FAIL;
            }
            break;
        case BROADCAST:
            hmac_handle_slp_rm_request(hmac_vap, OSAL_NULL, BROADCAST);
            break;

        default:
            oam_warning_log1(0, OAM_SF_SLP, "hmac_slp_send_rm_request: err communication_mode[%d]", communication_mode);
            break;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_slp_report_rm_result_test
 功能描述  : 发送SLP测距报告请求（发送参数准备）
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_slp_report_rm_result_test(hmac_vap_stru *hmac_vap, slp_nb_payload_and_device_req *rm_req)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN];

    if (osal_unlikely(hmac_vap == OSAL_NULL) || osal_unlikely(rm_req == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_get_slp_switch_status() == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_SLP, "hmac_slp_report_rm_result_test: SLP NOT ENABLE");
        return OAL_FAIL;
    }

    memcpy_s(mac_addr, WLAN_MAC_ADDR_LEN, rm_req->device_para.addr, WLAN_MAC_ADDR_LEN);

    hmac_user = hmac_vap_get_user_by_addr_etc(hmac_vap, mac_addr, sizeof(mac_addr));
    if (hmac_user != OSAL_NULL) {
        hmac_handle_slp_rm_result(hmac_vap, hmac_user);
    } else {
        oam_warning_log4(0, OAM_SF_SLP, "hmac_slp_report_rm_result: user not found[%x:%x:%x:%x:x:x]",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 打印mac地址0:1:2:3地址 */
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_slp_get_gtk
 功能描述  : 获取设备GTK密钥
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_slp_get_gtk(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 gtk_idx = 0;
    osal_u8 gtk_key[WLAN_WPA_KEY_LEN] = { 0 };      /* 密钥 */
    errno_t ret;

    unref_param(param);

    if (hmac_get_slp_switch_status() == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_SLP, "hmac_ccpriv_slp_get_gtk: SLP NOT ENABLE");
        return OAL_FAIL;
    }

    if (hmac_vap->vap_state != MAC_VAP_STATE_UP && hmac_vap->vap_state != MAC_VAP_STATE_PAUSE) {
        oam_warning_log1(0, OAM_SF_SLP, "hmac_ccpriv_slp_get_gtk: device not assoc[%d]", hmac_vap->vap_state);
        return OAL_FAIL;
    }

    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (hmac_user == OSAL_NULL) {
        return OAL_FAIL;
    }

    hmac_get_slp_gtk_idx(hmac_vap->vap_id, &gtk_idx);

    if (gtk_idx > (WLAN_NUM_TK + WLAN_NUM_IGTK - 1) || gtk_idx == WLAN_SLP_GTK_IDX_INVAILD) {
        oam_warning_log1(0, OAM_SF_SLP, "hmac_ccpriv_slp_get_gtk:gtk_idx[%d] is invaild", gtk_idx);
        return OAL_FAIL;
    }

    ret = memcpy_s(gtk_key, WLAN_WPA_KEY_LEN, hmac_user->key_info.key[gtk_idx].key,
        hmac_user->key_info.key[gtk_idx].key_len);
    if (ret != EOK) {
        oam_warning_log1(0, OAM_SF_SLP, "hmac_slp_get_gtk: memcpy fail[%d]", hmac_user->key_info.key[gtk_idx].key_len);
        return OAL_FAIL;
    }

    wifi_printf("[SLP] GTK get\r\n"); // 联调后修改为组播密钥上报

    return OAL_SUCC;
}

hmac_netbuf_hook_stru slp_netbuf_hook = {
    .hooknum = HMAC_FRAME_MGMT_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_rx_slp_action,
};

/*****************************************************************************
 函 数 名  : hmac_slp_set_enable
 功能描述  : 使能/去使能SLP特性开关
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_slp_set_enable(hmac_vap_stru *hmac_vap, osal_u8 slp_en)
{
    frw_msg msg_to_device = {0};
    osal_s32 ret;

    frw_msg_init((osal_u8 *)&slp_en, sizeof(osal_u8), OSAL_NULL, 0, &msg_to_device);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_SLP_ENABLE, &msg_to_device, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SLP, "{hmac_slp_set_enable:: send msg to device fail=%d!", ret);
        return ret;
    }

    g_slp_switch = (osal_bool)slp_en;

    if (g_slp_switch == OSAL_TRUE) {
        ret = (osal_s32)hmac_register_netbuf_hook(&slp_netbuf_hook);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_RX, "{hmac_slp_set_enable:: MGMT RX IN register_netbuf_hooks error!");
            return ret;
        }
        /* 对外接口注册 */
        hmac_feature_hook_register(HMAC_FHOOK_SLP_TX_DISABLE, hmac_slp_tx_disable);
    } else {
        hmac_unregister_netbuf_hook(&slp_netbuf_hook);
        /* 对外接口去注册 */
        hmac_feature_hook_unregister(HMAC_FHOOK_SLP_TX_DISABLE);
    }

    oam_warning_log1(0, OAM_SF_SLP, "hmac_slp_set_enable:slp_en[%d]", slp_en);

    return OAL_SUCC;
}

OSAL_STATIC osal_s32 hmac_ccpriv_slp_set_enable(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    uint32_t       ret;
    uint32_t       value;

    ret = hmac_ccpriv_get_u8_with_check_max(&param, 1, (osal_u8 *)&value);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_slp_set_enable:: value invalid.}");
        return ret;
    }

    ret = hmac_slp_set_enable(hmac_vap, value);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_slp_get_user_info_test
 功能描述  : 获取设备下挂用户信息
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_slp_get_userinfo(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    struct osal_list_head *entry, *dlist_tmp;
    osal_u8 *mac_addr = OSAL_NULL;

    unref_param(param);

    if (hmac_get_slp_switch_status() == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_SLP, "hmac_slp_get_user_info_test: SLP NOT ENABLE");
        return OAL_FAIL;
    }

    if (hmac_vap->vap_state != MAC_VAP_STATE_UP && hmac_vap->vap_state != MAC_VAP_STATE_PAUSE) {
        oam_warning_log1(0, OAM_SF_SLP, "hmac_ccpriv_slp_get_userinfo: device not assoc[%d]", hmac_vap->vap_state);
        return OAL_FAIL;
    }

    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OSAL_NULL) {
            continue;
        }

        mac_addr = hmac_user->user_mac_addr;
        oam_warning_log4(0, OAM_SF_SLP, "[SLP]hmac_ccpriv_slp_get_userinfo: user info[%x:%x:%x:%x:xx:xx]",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 打印mac地址0:1:2:3地址 */
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_s32 hmac_ccpriv_slp_start_rm_request(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    slp_nb_payload_and_device_req rm_req;
    uint32_t                      ret;
    uint32_t                      value;

    memset_s(&rm_req, sizeof(slp_nb_payload_and_device_req), 0, sizeof(slp_nb_payload_and_device_req));

    ret = hmac_ccpriv_get_u8_with_check_max(&param, 1, (osal_u8 *)&value);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{uapi_ccpriv_slp_start_rm_request::tid num invalid.}");
        return ret;
    }
    rm_req.device_para.communication_mode = value;

    if (rm_req.device_para.communication_mode == UNICAST) {
            /* 解析bssid */
        ret = hmac_ccpriv_get_mac_addr_etc(&param, rm_req.device_para.addr);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_CFG, "{hmac_ccpriv_slp_start_rm_request::get addr error.}");
            return OAL_FAIL;
        }
    }

    hmac_slp_send_rm_request_test(hmac_vap, &rm_req);

    return OAL_SUCC;
}

OSAL_STATIC osal_s32 hmac_ccpriv_slp_report_rm_result(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    slp_nb_payload_and_device_req rm_req;
    uint32_t                      ret;

    memset_s(&rm_req, sizeof(slp_nb_payload_and_device_req), 0, sizeof(slp_nb_payload_and_device_req));

    /* 解析bssid */
    ret = hmac_ccpriv_get_mac_addr_etc(&param, rm_req.device_para.addr);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_ccpriv_slp_report_rm_result::get addr error.}");
        return OAL_FAIL;
    }

    hmac_slp_report_rm_result_test(hmac_vap, &rm_req);

    return OAL_SUCC;
}

osal_u32 hmac_slp_init(osal_void)
{
    static const ccpriv_cmd_func_stru cmd_func[] = {
        {(const osal_s8 *)"slp_set_enable",  hmac_ccpriv_slp_set_enable},
        {(const osal_s8 *)"slp_get_userinfo",  hmac_ccpriv_slp_get_userinfo},
        {(const osal_s8 *)"slp_get_gtk",  hmac_ccpriv_slp_get_gtk},
        {(const osal_s8 *)"slp_start_rm_request",  hmac_ccpriv_slp_start_rm_request},
        {(const osal_s8 *)"slp_report_rm_result",  hmac_ccpriv_slp_report_rm_result},
    };

    /* ccpriv命令注册 */
    hmac_ccpriv_register_array(cmd_func, osal_array_size(cmd_func));

    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_GTK, hmac_slp_add_gtk_idx);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_GTK, hmac_slp_del_gtk_idx);

    return OAL_SUCC;
}

osal_void hmac_slp_deinit(osal_void)
{
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_GTK, hmac_slp_add_gtk_idx);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_GTK, hmac_slp_del_gtk_idx);

    hmac_ccpriv_unregister((const osal_s8 *)"slp_set_enable");
    hmac_ccpriv_unregister((const osal_s8 *)"slp_get_userinfo");
    hmac_ccpriv_unregister((const osal_s8 *)"slp_get_gtk");
    hmac_ccpriv_unregister((const osal_s8 *)"slp_start_rm_request");
    hmac_ccpriv_unregister((const osal_s8 *)"slp_report_rm_result");
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
