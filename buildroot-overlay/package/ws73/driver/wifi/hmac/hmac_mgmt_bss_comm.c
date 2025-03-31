/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
 * 文 件 名   : hmac_mgmt_bss_comm.c
 * 生成日期   : 2013年4月10日
 * 功能描述   : HMAC模块下，AP与STA公用的管理帧接口实现的源文件
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_mgmt_bss_comm.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "oal_kernel_file.h"
#endif
#include "wlan_spec.h"
#include "wlan_msg.h"
#include "mac_vap_ext.h"
#include "mac_ie.h"
#include "mac_frame.h"
#include "mac_resource_ext.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_fsm.h"
#include "hmac_encap_frame.h"
#include "hmac_tx_amsdu.h"
#include "hmac_mgmt_ap.h"
#include "hmac_mgmt_sta.h"
#include "hmac_blockack.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#include "wlan_msg.h"
#include "hmac_twt.h"
#include "securec.h"
#include "hmac_dfx.h"
#include "hmac_feature_dft.h"
#include "hmac_sdp.h"
#ifdef _PRE_WLAN_FEATURE_TXOPPS
#include "hmac_txopps.h"
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif
#include "hmac_feature_interface.h"
#include "hmac_btcoex_ba.h"
#include "hmac_tx_encap.h"
#include "hmac_scan.h"
#include "hmac_psm_ap.h"
#include "msg_twt_rom.h"
#include "msg_ba_manage_rom.h"
#include "common_log_dbg_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_MGMT_BSS_COMM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*
行:代表VAP 的协议能力
例:代表USER 的协议能力
*/
#ifdef _PRE_WLAN_FEATURE_11AX
const osal_u8 g_auc_avail_protocol_mode_etc[WLAN_PROTOCOL_BUTT][WLAN_PROTOCOL_BUTT] = {
    /**
    --------11A--------------------------11B------------------11G---------------ONE_11G----------------TWO_11G----------
    ----------HT------------------------VHT------------------HT_ONLY----------------VHT_ONLY----------HT_11G---------
    -------HE
    **/
    {WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,     WLAN_PROTOCOL_BUTT,
        WLAN_LEGACY_11A_MODE,   WLAN_LEGACY_11A_MODE,    WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,
        WLAN_LEGACY_11A_MODE}, // 11A
    {WLAN_PROTOCOL_BUTT, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE,   WLAN_LEGACY_11B_MODE,
        WLAN_LEGACY_11B_MODE,   WLAN_LEGACY_11B_MODE,    WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,
        WLAN_LEGACY_11B_MODE},   // 11B
    {WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,   WLAN_LEGACY_11G_MODE,
        WLAN_LEGACY_11G_MODE,   WLAN_LEGACY_11G_MODE,    WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_LEGACY_11G_MODE,
        WLAN_LEGACY_11G_MODE},   // 11G
    {WLAN_PROTOCOL_BUTT, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
        WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,
        WLAN_PROTOCOL_BUTT},   // ONE_11G
    {WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_TWO_11G_MODE,
        WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,
        WLAN_PROTOCOL_BUTT},   // TWO_11G
    {WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
        WLAN_HT_MODE,           WLAN_HT_MODE,            WLAN_HT_ONLY_MODE,  WLAN_PROTOCOL_BUTT, WLAN_HT_11G_MODE,
        WLAN_HT_MODE},        // HT
    {WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
        WLAN_HT_MODE,           WLAN_VHT_MODE,           WLAN_HT_ONLY_MODE,  WLAN_HT_ONLY_MODE,  WLAN_PROTOCOL_BUTT,
        WLAN_VHT_MODE},       // VHT
    {WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,     WLAN_PROTOCOL_BUTT,
        WLAN_HT_ONLY_MODE,      WLAN_HT_ONLY_MODE,       WLAN_HT_ONLY_MODE,  WLAN_HT_ONLY_MODE,  WLAN_HT_ONLY_MODE,
        WLAN_HT_ONLY_MODE},        // HT_ONLY
    {WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,     WLAN_PROTOCOL_BUTT,
        WLAN_PROTOCOL_BUTT,     WLAN_VHT_ONLY_MODE,      WLAN_PROTOCOL_BUTT, WLAN_VHT_ONLY_MODE, WLAN_PROTOCOL_BUTT,
        WLAN_PROTOCOL_BUTT},       // VHT_ONLY
    {WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT, WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,   WLAN_LEGACY_11G_MODE,
        WLAN_HT_11G_MODE,       WLAN_PROTOCOL_BUTT,      WLAN_HT_ONLY_MODE,  WLAN_PROTOCOL_BUTT, WLAN_HT_11G_MODE,
        WLAN_PROTOCOL_BUTT},     // HT_11G
    {WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
        WLAN_HT_MODE,           WLAN_VHT_MODE,           WLAN_HT_ONLY_MODE,  WLAN_VHT_ONLY_MODE, WLAN_PROTOCOL_BUTT,
        WLAN_HE_MODE}        /* he */
};
#else
const osal_u8 g_auc_avail_protocol_mode_etc[WLAN_PROTOCOL_BUTT][WLAN_PROTOCOL_BUTT] = {
    {WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,     WLAN_PROTOCOL_BUTT,
        WLAN_LEGACY_11A_MODE,   WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT},
    {WLAN_PROTOCOL_BUTT, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE,   WLAN_LEGACY_11B_MODE,
        WLAN_LEGACY_11B_MODE,   WLAN_LEGACY_11B_MODE, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT},
    {WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,   WLAN_LEGACY_11G_MODE,
        WLAN_LEGACY_11G_MODE,   WLAN_LEGACY_11G_MODE, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_LEGACY_11G_MODE},
    {WLAN_PROTOCOL_BUTT, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
        WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_LEGACY_11G_MODE},
    {WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_TWO_11G_MODE,
        WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_LEGACY_11G_MODE},
    {WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
        WLAN_HT_MODE,           WLAN_HT_MODE,         WLAN_HT_ONLY_MODE,  WLAN_PROTOCOL_BUTT, WLAN_HT_11G_MODE},
    {WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
        WLAN_HT_MODE,           WLAN_VHT_MODE,        WLAN_HT_ONLY_MODE,  WLAN_VHT_ONLY_MODE, WLAN_PROTOCOL_BUTT},
    {WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,     WLAN_PROTOCOL_BUTT,
        WLAN_HT_ONLY_MODE,      WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,  WLAN_HT_ONLY_MODE,  WLAN_HT_ONLY_MODE},
    {WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT, WLAN_PROTOCOL_BUTT,     WLAN_PROTOCOL_BUTT,
        WLAN_PROTOCOL_BUTT,     WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT, WLAN_VHT_ONLY_MODE, WLAN_PROTOCOL_BUTT},
    {WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT, WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,   WLAN_LEGACY_11G_MODE,
        WLAN_HT_11G_MODE,       WLAN_PROTOCOL_BUTT,   WLAN_HT_ONLY_MODE,  WLAN_PROTOCOL_BUTT, WLAN_HT_11G_MODE}
};
#endif

osal_u32 hmac_mgmt_tx_addba_timeout_etc(osal_void *p_arg);

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_rx_ba_session_decr_etc
*****************************************************************************/
osal_void hmac_rx_ba_session_decr_etc(hmac_vap_stru *hmac_vap, osal_u8 tidno)
{
    if (mac_mib_get_RxBASessionNumber(hmac_vap) == 0) {
        oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_rx_ba_session_decr_etc::tid[%d] rx_session already zero.}",
            hmac_vap->vap_id, tidno);
        return;
    }

    mac_mib_decr_RxBASessionNumber(hmac_vap);

    oam_warning_log3(0, OAM_SF_BA, "vap_id[%d] {hmac_rx_ba_session_decr_etc::tid[%d] tx_session[%d] remove.}",
        hmac_vap->vap_id, tidno,
        mac_mib_get_RxBASessionNumber(hmac_vap));
}

/*****************************************************************************
 函 数 名  : hmac_tx_ba_session_decr_etc
*****************************************************************************/
osal_void hmac_tx_ba_session_decr_etc(hmac_vap_stru *hmac_vap, osal_u8 tidno)
{
    if (mac_mib_get_TxBASessionNumber(hmac_vap) == 0) {
        oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_tx_ba_session_decr_etc::tid[%d] tx_session already zero.}",
            hmac_vap->vap_id, tidno);
        return;
    }

    mac_mib_decr_TxBASessionNumber(hmac_vap);

    oam_warning_log3(0, OAM_SF_BA, "vap_id[%d] {hmac_tx_ba_session_decr_etc::tid[%d] tx_session[%d] remove.}",
        hmac_vap->vap_id, tidno,
        mac_mib_get_TxBASessionNumber(hmac_vap));
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_addba_req_etc
 功能描述  : 组装ADDBA_REQ帧
*****************************************************************************/
OAL_STATIC osal_u16 hmac_mgmt_encap_addba_req_etc(hmac_vap_stru *hmac_vap, osal_u8 *data,
    hmac_ba_tx_hdl_stru *tx_ba, osal_u8 tid)
{
    osal_u16  us_index;
    osal_u16  ba_param;

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

    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* DA is address of STA requesting association */
    oal_set_mac_addr(data + 4, tx_ba->dst_addr);  /* 偏移4位 */

    /* SA的值为dot11MACAddress的值 */
    oal_set_mac_addr(data + 10, mac_mib_get_station_id(hmac_vap));  /* 偏移10位 */

    oal_set_mac_addr(data + 16, hmac_vap->bssid);  /* 偏移16位 */
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /* 将索引指向frame body起始位置 */
    us_index = MAC_80211_FRAME_LEN;

    /* 设置Category */
    data[us_index++] = MAC_ACTION_CATEGORY_BA;

    /* 设置Action */
    data[us_index++] = MAC_BA_ACTION_ADDBA_REQ;

    /* 设置Dialog Token */
    data[us_index++] = tx_ba->dialog_token;

    /*
        设置Block Ack Parameter set field
        bit0 - AMSDU Allowed
        bit1 - Immediate or Delayed block ack
        bit2-bit5 - TID
        bit6-bit15 -  Buffer size
    */
    ba_param = tx_ba->amsdu_supp;         /* bit0 */
    ba_param |= (tx_ba->ba_policy << 1);  /* bit1 */
    ba_param |= (tid << 2);                   /* bit2 */

    ba_param |= (osal_u16)(tx_ba->baw_size << 6);   /* bit6 */

    data[us_index++] = (osal_u8)(ba_param & 0xFF);
    data[us_index++] = (osal_u8)((ba_param >> 8) & 0xFF);  /* 偏移8位 */

    /* 设置BlockAck timeout */
    data[us_index++] = (osal_u8)(tx_ba->ba_timeout & 0xFF);
    data[us_index++] = (osal_u8)((tx_ba->ba_timeout >> 8) & 0xFF);  /* 偏移8位 */

    /*
        Block ack starting sequence number字段由硬件设置
        bit0-bit3 fragmentnumber
        bit4-bit15: sequence number
    */

    /* buf_seq此处暂不填写，在dmac侧会补充填写 */
    *(osal_u16 *)&data[us_index++] = 0;
    us_index++;

    /* 返回的帧长度中不包括FCS */
    return us_index;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_addba_rsp_etc
 功能描述  : 组装ADDBA_RSP帧
*****************************************************************************/
OAL_STATIC osal_u16 hmac_mgmt_encap_addba_rsp_etc(hmac_vap_stru *hmac_vap, osal_u8 *data,
    hmac_ba_rx_stru *addba_rsp, osal_u8 tid, osal_u8 status)
{
    osal_u16  us_index;
    osal_u16  ba_param;
    osal_void *fhook = NULL;

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
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* DA is address of STA requesting association */
    oal_set_mac_addr(data + 4, addba_rsp->transmit_addr);  /* 偏移4字节 */

    /* SA is the dot11MACAddress */
    oal_set_mac_addr(data + 10, mac_mib_get_station_id(hmac_vap));  /* 偏移10字节 */

    oal_set_mac_addr(data + 16, hmac_vap->bssid);  /* 偏移16字节 */
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*             ADDBA Response Frame - Frame Body                         */
    /*    ---------------------------------------------------------------    */
    /*    | Category | Action | Dialog | Status  | Parameters | Timeout |    */
    /*    ---------------------------------------------------------------    */
    /*    | 1        | 1      | 1      | 2       | 2          | 2       |    */
    /*    ---------------------------------------------------------------    */
    /*                                                                       */
    /*************************************************************************/

    /* Initialize index and the frame data pointer */
    us_index = MAC_80211_FRAME_LEN;

    /* Action Category设置 */
    data[us_index++] = MAC_ACTION_CATEGORY_BA;

    /* 特定Action种类下的action的帧类型 */
    data[us_index++] = MAC_BA_ACTION_ADDBA_RSP;

    /* Dialog Token域设置，需要从req中copy过来 */
    data[us_index++] = addba_rsp->dialog_token;

    /* 状态域设置 */
    data[us_index++] = status;
    data[us_index++]  = 0;

    /* Block Ack Parameter设置 */
    /* B0 - AMSDU Support, B1- Immediate or Delayed block ack */
    /* B2-B5 : TID, B6-B15: Buffer size */
    ba_param  = addba_rsp->amsdu_supp;                    /* BIT0 */
    ba_param |= (addba_rsp->ba_policy << 1);              /* BIT1 */
    ba_param |= (tid << 2);                                   /* BIT2 */

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_BA_ADDBA);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_adjust_addba_rsp_basize_cb)fhook)(hmac_vap, addba_rsp);  /* 更新 ba_resp_buf_size */
    }
    ba_param |= (osal_u16)(addba_rsp->ba_resp_buf_size << 6);   /* BIT6 */

    data[us_index++] = (osal_u8)(ba_param & 0xFF);
    data[us_index++] = (osal_u8)((ba_param >> 8) & 0xFF);  /* 偏移8位 */

    /* BlockAck timeout值设置,解决timeout兼容性问题 */
    data[us_index++] = 0x00;
    data[us_index++] = 0x00;

    /* 返回的帧长度中不包括FCS */
    return us_index;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_delba_etc
 功能描述  : 组装DELBA帧
*****************************************************************************/
OAL_STATIC osal_u16 hmac_mgmt_encap_delba_etc(hmac_vap_stru *hmac_vap, osal_u8 *data, osal_u8 *addr,
    osal_u8 tid, mac_delba_initiator_enum_uint8 initiator, osal_u8 reason)
{
    osal_u16  us_index;

    if (addr == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_encap_delba_etc::null param.}");
        return 0;
    }

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
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* duration */
    data[2] = 0;  /* 2索引值 */
    data[3] = 0;  /* 3索引值 */

    /* DA is address of STA requesting association */
    oal_set_mac_addr(data + 4, addr);  /* 偏移4字节 */

    /* SA is the dot11MACAddress */
    oal_set_mac_addr(data + 10, mac_mib_get_station_id(hmac_vap));  /* 偏移10字节 */

    oal_set_mac_addr(data + 16, hmac_vap->bssid);  /* 偏移16字节 */

    /* seq control */
    data[22] = 0;  /* 22索引值 */
    data[23] = 0;  /* 23索引值 */

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*             DELBA Response Frame - Frame Body                         */
    /*        -------------------------------------------------              */
    /*        | Category | Action |  Parameters | Reason code |              */
    /*        -------------------------------------------------              */
    /*        | 1        | 1      |       2     | 2           |              */
    /*        -------------------------------------------------              */
    /*                          Parameters                                   */
    /*                  -------------------------------                      */
    /*                  | Reserved | Initiator |  TID  |                     */
    /*                  -------------------------------                      */
    /*             bit  |    11    |    1      |  4    |                     */
    /*                  --------------------------------                     */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    us_index = MAC_80211_FRAME_LEN;

    /* Category */
    data[us_index++] = MAC_ACTION_CATEGORY_BA;

    /* Action */
    data[us_index++] = MAC_BA_ACTION_DELBA;

    /* DELBA parameter set */
    /* B0 - B10 -reserved */
    /* B11 - initiator */
    /* B12-B15 - TID */
    data[us_index++]  = 0;
    data[us_index]    = (osal_u8)(initiator << 3);  /* 偏移3位 */
    data[us_index++] |= (osal_u8)(tid << 4);  /* 偏移4位 */

    /* Reason field */
    /* Reason can be either of END_BA, QSTA_LEAVING, UNKNOWN_BA */
    data[us_index++] = reason;
    data[us_index++] = 0;

    /* 返回的帧长度中不包括FCS */
    return us_index;
}

/*****************************************************************************
 功能描述  : 填充addba_req的netbuf
*****************************************************************************/
OAL_STATIC osal_u16 hmac_fill_tx_addba_req_netbuf(oal_netbuf_stru *addba_req, hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, const mac_action_mgmt_args_stru *action_args)
{
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;
    hmac_ba_tx_hdl_stru  tx_ba = {0};
    osal_u16       frame_len;
    osal_u8        tidno = (osal_u8)(action_args->arg1);
    hmac_ctx_action_event_stru wlan_ctx_action = {0};

    OAL_NETBUF_PREV(addba_req) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(addba_req) = OAL_PTR_NULL;

    hmac_vap->ba_dialog_token++;
    tx_ba.dialog_token = hmac_vap->ba_dialog_token;  /* 保证ba会话创建能够区分 */
    tx_ba.baw_size     = (osal_u8)(action_args->arg2);
    tx_ba.ba_policy    = (osal_u8)(action_args->arg3);
    tx_ba.ba_timeout   = (osal_u16)(action_args->arg4);
    tx_ba.dst_addr    = hmac_user->user_mac_addr;

    /* 发端对AMPDU+AMSDU的支持 */
    tx_ba.amsdu_supp = (oal_bool_enum_uint8)mac_mib_get_AmsduPlusAmpduActive(hmac_vap);

    if (tx_ba.amsdu_supp == OAL_FALSE) {
        hmac_user_set_amsdu_not_support(hmac_user, tidno);
    } else {
        hmac_user_set_amsdu_support(hmac_user, tidno);
    }

    /* 调用封装管理帧接口 */
    frame_len = hmac_mgmt_encap_addba_req_etc(hmac_vap, oal_netbuf_data(addba_req), &tx_ba, tidno);
    memset_s((osal_u8 *)&wlan_ctx_action, OAL_SIZEOF(wlan_ctx_action), 0, OAL_SIZEOF(wlan_ctx_action));

    /* 赋值要传入Dmac的信息 */
    wlan_ctx_action.frame_len        = frame_len;
    wlan_ctx_action.hdr_len          = MAC_80211_FRAME_LEN;
    wlan_ctx_action.action_category  = MAC_ACTION_CATEGORY_BA;
    wlan_ctx_action.action           = MAC_BA_ACTION_ADDBA_REQ;
    wlan_ctx_action.user_idx         = hmac_user->assoc_id;
    wlan_ctx_action.tidno            = tidno;
    wlan_ctx_action.dialog_token     = tx_ba.dialog_token;
    wlan_ctx_action.ba_policy        = tx_ba.ba_policy;
    wlan_ctx_action.baw_size         = tx_ba.baw_size;
    wlan_ctx_action.ba_timeout       = tx_ba.ba_timeout;
    wlan_ctx_action.amsdu_supp       = tx_ba.amsdu_supp;

    (osal_void)memcpy_s((osal_u8 *)(oal_netbuf_data(addba_req) + frame_len),
        OAL_SIZEOF(hmac_ctx_action_event_stru), (osal_u8 *)&wlan_ctx_action,
        OAL_SIZEOF(hmac_ctx_action_event_stru));
    oal_netbuf_put(addba_req, (frame_len + OAL_SIZEOF(hmac_ctx_action_event_stru)));

    /* 初始化CB */
    memset_s(oal_netbuf_cb(addba_req), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(addba_req);
    mac_get_cb_mpdu_len(tx_ctl) = frame_len + OAL_SIZEOF(hmac_ctx_action_event_stru);
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    mac_get_cb_frame_subtype(tx_ctl) = WLAN_ACTION_BA_ADDBA_REQ;
    return mac_get_cb_mpdu_len(tx_ctl);
}

OAL_STATIC osal_u32 hmac_mgmt_tx_addba_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const mac_action_mgmt_args_stru *action_args)
{
    osal_u32       ul_ret;
    oal_netbuf_stru *addba_req = OAL_PTR_NULL;
    osal_u16       frame_len;

    /* 申请ADDBA_REQ管理帧内存 */
    addba_req = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (addba_req == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_tx_addba_req::addba_req null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    frame_len = hmac_fill_tx_addba_req_netbuf(addba_req, hmac_vap, hmac_user, action_args);

    /* 抛到DMAC */
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, addba_req, frame_len);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_tx_addba_req::hmac_addba_req_dispatch_event fail ret[%u].}",
            hmac_vap->vap_id, ul_ret);
        oal_netbuf_free(addba_req);
    }

    return ul_ret;
}


/****************************************************************************
功能描述 : tx mgmt addba req to dmac
*****************************************************************************/
osal_u32 hmac_mgmt_tx_addba_req_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_action_mgmt_args_stru *action_args)
{
    hmac_device_stru           *hmac_device = OAL_PTR_NULL;
    osal_u8                   tidno;
    osal_u32                  ul_ret;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (action_args == OAL_PTR_NULL)) {
        common_log_dbg0(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_req_etc::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 确定vap处于工作状态 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_BUTT) {
        common_log_dbg2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_tx_addba_req_etc:: vap has been down/del, vap_state[%d]}",
            hmac_vap->vap_id, hmac_vap->vap_state);
        return OAL_FAIL;
    }

    /* 获取device结构 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        common_log_dbg1(0, OAM_SF_BA, "vap_id[%d]{hmac_mgmt_tx_addba_req_etc:device null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    tidno = (osal_u8)(action_args->arg1);
    /* tx addba */
    ul_ret = hmac_mgmt_tx_addba_req(hmac_vap, hmac_user, action_args);
    if (ul_ret != OAL_SUCC) {
        common_log_dbg1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_tx_addba_req_etc::hmac_mgmt_tx_addba_req fail.}",
            hmac_vap->vap_id);
        return ul_ret;
    }

   /* 对tid对应的txBA会话状态加锁 */
    osal_spin_lock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));
    /* 更新对应的TID信息 */
    hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status    = HMAC_BA_INPROGRESS;
    hmac_user->tx_tid_queue[tidno].ba_tx_info.dialog_token = hmac_vap->ba_dialog_token;
    hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_policy    = (osal_u8)(action_args->arg3);

    mac_mib_incr_TxBASessionNumber(hmac_vap);

    /* 启动ADDBA超时计时器 */
    frw_create_timer_entry(&hmac_user->tx_tid_queue[tidno].ba_tx_info.addba_timer,
                           hmac_mgmt_tx_addba_timeout_etc,
                           WLAN_ADDBA_TIMEOUT,
                           &hmac_user->tx_tid_queue[tidno].ba_tx_info.alarm_data,
                           OAL_FALSE);

    /* 对tid对应的tx BA会话状态解锁 */
    osal_spin_unlock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : 填充 addba rsp帧 netbuf ，抛到dmac
*****************************************************************************/
OAL_STATIC osal_u32 hmac_mgmt_tx_addba_rsp(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_ba_rx_stru *ba_rx_info, osal_u8 tid, osal_u8 status)
{
    hmac_ctx_action_event_stru wlan_ctx_action = {0};
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;
    oal_netbuf_stru *addba_rsp = OAL_PTR_NULL;
    osal_u16       frame_len;
    osal_u32       ul_ret;

    /* 申请ADDBA_RSP管理帧内存 */
    addba_rsp = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (addba_rsp == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_BA,
            "vap_id[%d] {hmac_mgmt_tx_addba_rsp:: addba_rsp mem alloc failed.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_NETBUF_PREV(addba_rsp) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(addba_rsp) = OAL_PTR_NULL;

    frame_len = hmac_mgmt_encap_addba_rsp_etc(hmac_vap, oal_netbuf_data(addba_rsp), ba_rx_info, tid,
        status);

    memset_s((osal_u8 *)&wlan_ctx_action, OAL_SIZEOF(wlan_ctx_action), 0, OAL_SIZEOF(wlan_ctx_action));

    wlan_ctx_action.action_category  = MAC_ACTION_CATEGORY_BA;
    wlan_ctx_action.action           = MAC_BA_ACTION_ADDBA_RSP;
    wlan_ctx_action.hdr_len          = MAC_80211_FRAME_LEN;
    wlan_ctx_action.baw_size         = ba_rx_info->ba_resp_buf_size;
    wlan_ctx_action.frame_len        = frame_len;
    wlan_ctx_action.ba_timeout       = ba_rx_info->ba_timeout;
    wlan_ctx_action.back_var         = ba_rx_info->back_var;
    wlan_ctx_action.lut_index        = ba_rx_info->lut_index;
    wlan_ctx_action.baw_start        = ba_rx_info->baw_start;
    wlan_ctx_action.ba_policy        = ba_rx_info->ba_policy;
    wlan_ctx_action.user_idx         = hmac_user->assoc_id;
    wlan_ctx_action.tidno            = tid;
    wlan_ctx_action.status           = status;

    (osal_void)memcpy_s((osal_u8 *)(oal_netbuf_data(addba_rsp) + frame_len),
        OAL_SIZEOF(hmac_ctx_action_event_stru), (osal_u8 *)&wlan_ctx_action,
        OAL_SIZEOF(hmac_ctx_action_event_stru));
    oal_netbuf_put(addba_rsp, (frame_len + OAL_SIZEOF(hmac_ctx_action_event_stru)));

    memset_s(oal_netbuf_cb(addba_rsp), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(addba_rsp);
    mac_get_cb_tx_user_idx(tx_ctl)   = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_wme_tid_type(tx_ctl) = tid;
    mac_get_cb_mpdu_len(tx_ctl) = frame_len + OAL_SIZEOF(hmac_ctx_action_event_stru);
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    mac_get_cb_frame_subtype(tx_ctl) = WLAN_ACTION_BA_ADDBA_RSP;

    /* 抛dmac处理 */
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, addba_rsp, mac_get_cb_mpdu_len(tx_ctl));
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_tx_addba_rsp:: dispatch_event fail. ret[%u]}",
            hmac_vap->vap_id, ul_ret);
        oal_netbuf_free(addba_rsp);
    } else {
        ba_rx_info->ba_status = HMAC_BA_COMPLETE;
    }
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : tx addba rsp帧
*****************************************************************************/
osal_u32 hmac_mgmt_tx_addba_rsp_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_ba_rx_stru *ba_rx_info, osal_u8 tid, osal_u8 status)
{
    hmac_device_stru                *hmac_device = OAL_PTR_NULL;
    osal_u32                       ul_ret;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (ba_rx_info == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_rsp_etc::send addba rsp failed, null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 确定vap处于工作状态 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_tx_addba_rsp_etc::vap has been down/del, vap_state[%d].}",
            hmac_vap->vap_id, hmac_vap->vap_state);
        return OAL_FAIL;
    }

    /* 获取device结构 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_tx_addba_rsp_etc::send addba rsp failed, device null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_mgmt_tx_addba_rsp(hmac_vap, hmac_user, ba_rx_info, tid, status);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_tx_addba_rsp:: dispatch_event fail. ret[%u]}",
            hmac_vap->vap_id, ul_ret);
    }

    return ul_ret;
}
/*****************************************************************************
 功能描述  : 填充delba netbuf，初始化cb
*****************************************************************************/
OAL_STATIC osal_u16 hmac_fill_tx_delba_netbuf(oal_netbuf_stru *delba, hmac_user_stru *hmac_user,
    hmac_vap_stru *hmac_vap, mac_action_mgmt_args_stru *action_args)
{
    hmac_ctx_action_event_stru        wlan_ctx_action = {0};
    mac_delba_initiator_enum_uint8    initiator;
    osal_u16                        frame_len;
    osal_u8                         tidno;
    mac_tx_ctl_stru                  *tx_ctl = OAL_PTR_NULL;

    initiator = (mac_delba_initiator_enum_uint8)(action_args->arg2);
    tidno     = (osal_u8)(action_args->arg1);

    OAL_NETBUF_PREV(delba) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(delba) = OAL_PTR_NULL;

    /* 调用封装管理帧接口 */
    frame_len = hmac_mgmt_encap_delba_etc(hmac_vap, (osal_u8 *)OAL_NETBUF_HEADER(delba),
        action_args->arg5, tidno, initiator, (osal_u8)action_args->arg3);
    memset_s((osal_u8 *)&wlan_ctx_action, OAL_SIZEOF(wlan_ctx_action), 0, OAL_SIZEOF(wlan_ctx_action));
    wlan_ctx_action.frame_len        = frame_len;
    wlan_ctx_action.hdr_len          = MAC_80211_FRAME_LEN;
    wlan_ctx_action.action_category  = MAC_ACTION_CATEGORY_BA;
    wlan_ctx_action.action           = MAC_BA_ACTION_DELBA;
    wlan_ctx_action.user_idx         = hmac_user->assoc_id;
    wlan_ctx_action.tidno            = tidno;
    wlan_ctx_action.initiator        = initiator;

    memcpy_s((osal_u8 *)(oal_netbuf_data(delba) + frame_len), OAL_SIZEOF(hmac_ctx_action_event_stru),
        (osal_u8 *)&wlan_ctx_action, OAL_SIZEOF(hmac_ctx_action_event_stru));
    oal_netbuf_put(delba, (frame_len + OAL_SIZEOF(hmac_ctx_action_event_stru)));

    /* 初始化CB */
    memset_s(oal_netbuf_cb(delba), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(delba);
    mac_get_cb_mpdu_len(tx_ctl)      = frame_len + OAL_SIZEOF(hmac_ctx_action_event_stru);
    mac_get_cb_frame_type(tx_ctl)    = WLAN_CB_FRAME_TYPE_ACTION;
    mac_get_cb_frame_subtype(tx_ctl) = WLAN_ACTION_BA_DELBA;
    return mac_get_cb_mpdu_len(tx_ctl);
}

OAL_STATIC osal_void hmac_tx_delba_complete_proc(mac_delba_initiator_enum_uint8 initiator,
    hmac_device_stru *hmac_device, hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap, osal_u8 tidno)
{
    if (initiator == MAC_RECIPIENT_DELBA) {
        /* 更新对应的TID信息 */
        hmac_ba_reset_rx_handle_etc(hmac_device, hmac_user, tidno, OAL_FALSE);
    } else {
        /* 更新对应的TID信息 */
        hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status = HMAC_BA_INIT;
        hmac_user->ba_flag[tidno] = 0;

        hmac_tx_ba_session_decr_etc(hmac_vap, hmac_user->tx_tid_queue[tidno].tid);

        /* 还原设置AMPDU下AMSDU的支持情况 */
        hmac_user_set_amsdu_support(hmac_user, tidno);
    }
}

/*****************************************************************************
 功能描述  : 发送DELBA处理接口
*****************************************************************************/
osal_u32 hmac_mgmt_tx_delba_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_action_mgmt_args_stru  *action_args)
{
    hmac_device_stru                 *hmac_device = OAL_PTR_NULL;
    oal_netbuf_stru                  *delba = OAL_PTR_NULL;
    mac_delba_initiator_enum_uint8    initiator;
    osal_u32                        ul_ret;
    osal_u8                         tidno;
    osal_u16                        frame_len;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (action_args == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_tx_delba_check_param::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 需确定vap处于工作状态 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_tx_delba_check_param:: vap has been down/del, vap_state[%d].}",
                         hmac_vap->vap_id, hmac_vap->vap_state);
        return OAL_FAIL;
    }

    /* 获取device结构 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_tx_delba_check_param:device null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    initiator = (mac_delba_initiator_enum_uint8)(action_args->arg2);
    tidno     = (osal_u8)(action_args->arg1);

    /* 对tid对应的tx BA会话状态加锁 */
    osal_spin_lock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));

    /* 接收方向判断接收方向status，发送方向判断发送方向status */
    /* 接收方向delba发送失败，或者对端没有收到，要再次发送delba时需要放宽判断,这里主要保证rx delba始终可以发送即可，无其他影响 */
    if (initiator == MAC_ORIGINATOR_DELBA) {
        if (hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status == HMAC_BA_INIT) {
            osal_spin_unlock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));
            return OAL_SUCC;
        }
    }

    /* 申请DEL_BA管理帧内存 */
    delba = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (delba == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_tx_delba_etc::delba null.}", hmac_vap->vap_id);
        osal_spin_unlock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填充betbuf */
    frame_len = hmac_fill_tx_delba_netbuf(delba, hmac_user, hmac_vap, action_args);

    /* 抛到DMAC */
    osal_spin_unlock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, delba, frame_len);
    osal_spin_lock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));
    if (ul_ret != OAL_SUCC) {
        /* 释放管理帧内存到netbuf内存池 */
        oal_netbuf_free(delba);
        /* 对tid对应的tx BA会话状态解锁 */
        osal_spin_unlock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));
        return ul_ret;
    }

    hmac_tx_delba_complete_proc(initiator, hmac_device, hmac_user, hmac_vap, tidno);
    /* 对tid对应的tx BA会话状态解锁 */
    osal_spin_unlock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));

    return OAL_SUCC;
}

/****************************************************************************
 功能描述  : 初始化接收侧BA会话句柄 - 获取baw_size
*****************************************************************************/
OAL_STATIC osal_void hmac_init_rx_addba_req_get_baw_size(const osal_u8 *payload, hmac_ba_rx_stru *ba_rx_stru)
{
    mac_rx_buffer_size_stru *rx_buffer_size_stru = OSAL_NULL;

    ba_rx_stru->baw_size  = (payload[3] & 0xC0) >> 6; /* 3索引值 右移6位 */
    ba_rx_stru->baw_size |= (payload[4] << 2); /* 4索引值 左移2位 */
    if (ba_rx_stru->baw_size <= 1) { /* 1的场景: 解决goldenap新固件版本兼容性问题 */
        ba_rx_stru->baw_size = WLAN_AMPDU_RX_BUFFER_SIZE;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (hwifi_get_ampdu_rx_max_num() > 0) {
        ba_rx_stru->baw_size = hwifi_get_ampdu_rx_max_num();
    }
#endif

    rx_buffer_size_stru = mac_get_pst_rx_buffer_size_stru();
    if (rx_buffer_size_stru->rx_buffer_size_set_en == OAL_TRUE) { /* 增加命令，手动配置接收聚合个数 */
        ba_rx_stru->baw_size = rx_buffer_size_stru->rx_buffer_size;
    }

    ba_rx_stru->baw_size = OAL_MIN(ba_rx_stru->baw_size, WLAN_AMPDU_RX_BUFFER_SIZE);
    ba_rx_stru->ba_resp_buf_size = (osal_u8)ba_rx_stru->baw_size;
}

/*****************************************************************************
 功能描述  : 初始化接收侧BA会话句柄
*****************************************************************************/
OAL_STATIC osal_void hmac_init_rx_addba_req_ba(hmac_user_stru *hmac_user, osal_u8 tid, osal_u8 *payload,
    hmac_ba_rx_stru *ba_rx_stru, hmac_vap_stru *hmac_vap)
{
    osal_u8 reorder_index;

    ba_rx_stru->ba_status = HMAC_BA_INIT;
    ba_rx_stru->dialog_token = payload[2];  /* 2索引值 */

    /* 初始化reorder队列 */
    for (reorder_index = 0; reorder_index < WLAN_AMPDU_RX_BUFFER_SIZE; reorder_index++) {
        ba_rx_stru->re_order_list[reorder_index].in_use     = 0;
        ba_rx_stru->re_order_list[reorder_index].seq_num = 0;
        ba_rx_stru->re_order_list[reorder_index].num_buf = 0;
        oal_netbuf_list_head_init(&(ba_rx_stru->re_order_list[reorder_index].netbuf_head));
    }

    /* 初始化接收窗口 */
    ba_rx_stru->baw_start = (payload[7] >> 4) | (payload[8] << 4); /* 7、8索引值 偏移4位 */

    /* get buffer size */
    hmac_init_rx_addba_req_get_baw_size(payload, ba_rx_stru);

    ba_rx_stru->baw_end   = (osal_u16)hmac_ba_seq_add(ba_rx_stru->baw_start, (ba_rx_stru->baw_size - 1));
    ba_rx_stru->baw_tail  = (osal_u16)hmac_ba_seqno_sub(ba_rx_stru->baw_start, 1);
    ba_rx_stru->baw_head  = (osal_u16)hmac_ba_seqno_sub(ba_rx_stru->baw_start, HMAC_BA_BMP_SIZE);
    ba_rx_stru->mpdu_cnt  = 0;
    ba_rx_stru->is_ba     = OAL_TRUE;  // Ba session is processing

    /* 初始化定时器资源 */
    ba_rx_stru->alarm_data.ba           = ba_rx_stru;
    ba_rx_stru->alarm_data.mac_user_idx  = hmac_user->assoc_id;
    ba_rx_stru->alarm_data.vap_id        = hmac_vap->vap_id;
    ba_rx_stru->alarm_data.tid           = tid;
    ba_rx_stru->alarm_data.timeout_times = 0;
    ba_rx_stru->alarm_data.direction     = MAC_RECIPIENT_DELBA;
    ba_rx_stru->timer_triggered             = OAL_FALSE;

    /* Ba会话参数初始化 */
    ba_rx_stru->ba_timeout = payload[5] | (payload[6] << 8); /* 5、6索引值 左移8位 */
    ba_rx_stru->amsdu_supp = ((hmac_vap->rx_ampduplusamsdu_active != 0) ? OAL_TRUE : OAL_FALSE);
    ba_rx_stru->back_var   = MAC_BACK_COMPRESSED;
    ba_rx_stru->transmit_addr = hmac_user->user_mac_addr;
    ba_rx_stru->ba_policy      = (payload[3] & 0x02) >> 1; /* 3索引值 */
}

/*****************************************************************************
 功能描述  : 回复rsp
*****************************************************************************/
OAL_STATIC osal_void hmac_rx_addba_req_reply(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 tid, hmac_ba_rx_stru *ba_rx_stru, hmac_device_stru *hmac_device)
{
    osal_u8 status;
    osal_u32 ul_ret;

    /* profiling测试中，接收端不删除ba */
    frw_create_timer_entry(&(hmac_user->tx_tid_queue[tid].ba_timer),
                           hmac_ba_timeout_fn_etc,
                           hmac_vap->rx_timeout[wlan_wme_tid_to_ac(tid)],
                           &(ba_rx_stru->alarm_data),
                           OAL_FALSE);

    mac_mib_incr_RxBASessionNumber(hmac_vap);

    /* 判断建立能否成功 */
    status = hmac_mgmt_check_set_rx_ba_ok_etc(hmac_vap, hmac_user, ba_rx_stru, hmac_device,
        &(hmac_user->tx_tid_queue[tid]));
    if (status == MAC_SUCCESSFUL_STATUSCODE) {
        hmac_user->tx_tid_queue[tid].ba_rx_info->ba_status = HMAC_BA_INPROGRESS;
    }

    ul_ret = hmac_mgmt_tx_addba_rsp_etc(hmac_vap, hmac_user, ba_rx_stru, tid, status);

    oam_warning_log4(0, OAM_SF_BA,
        "{process addba req receive and send addba rsp, tid[%d], status[%d], baw_size[%d], RXAMSDU[%d]}",
        tid, status, ba_rx_stru->baw_size, ba_rx_stru->amsdu_supp);

    if ((status != MAC_SUCCESSFUL_STATUSCODE) || (ul_ret != OAL_SUCC)) {
        hmac_ba_reset_rx_handle_etc(hmac_device, hmac_user, tid, OAL_FALSE);
    }
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_rx_addba_req_etc
 功能描述  : 从空口接收ADDBA_REQ帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_rx_addba_req_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload)
{
    hmac_device_stru   *hmac_device = OAL_PTR_NULL;
    osal_u8           tid;
    hmac_ba_rx_stru    *ba_rx_stru = OAL_PTR_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (payload == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_rx_addba_req_etc::addba req receive failed,null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 11n以上能力才可接收ampdu */
    if ((!(hmac_vap->protocol >= WLAN_HT_MODE)) ||
        (!(hmac_user->protocol_mode >= WLAN_HT_MODE))) {
        oam_warning_log3(0, OAM_SF_BA,
            "vap_id[%d] {ampdu is not supprot or not open, vap protocol mode=%d,user protocol mode = %d}",
            hmac_vap->vap_id, hmac_vap->protocol, hmac_user->protocol_mode);
        return OAL_SUCC;
    }

    /* 获取device结构 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_addba_req_etc fail device null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /******************************************************************/
    /*       ADDBA Request Frame - Frame Body                         */
    /* ---------------------------------------------------------------*/
    /* | Category | Action | Dialog | Parameters | Timeout | SSN    | */
    /* ---------------------------------------------------------------*/
    /* | 1        | 1      | 1      | 2          | 2       | 2      | */
    /* ---------------------------------------------------------------*/
    /*                                                                */
    /******************************************************************/

    tid = (payload[3] & 0x3C) >> 2;  /* 3索引值 偏移2位 */
    if (tid >= WLAN_TID_MAX_NUM) {
        oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_addba_req_etc fail tid %d overflow.}",
            hmac_vap->vap_id, tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    if (hmac_user->tx_tid_queue[tid].ba_rx_info != OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_BA,
            "vap_id[%d] {hmac_mgmt_rx_addba_req_etc::addba req received, but tid [%d] already set up.}",
            hmac_vap->vap_id, tid);
        /* addba rsp发送失败,再次受到addba req重新建立BA */
        hmac_ba_reset_rx_handle_etc(hmac_device, hmac_user, tid, OAL_FALSE);
    }

    hmac_user->tx_tid_queue[tid].ba_rx_info = (hmac_ba_rx_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        (osal_u16)OAL_SIZEOF(hmac_ba_rx_stru), OAL_TRUE);
    if (hmac_user->tx_tid_queue[tid].ba_rx_info == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_BA,
            "vap_id[%d] {hmac_mgmt_rx_addba_req_etc::addba req receive failed, ba_rx_hdl mem alloc failed.tid[%d]}",
            hmac_vap->vap_id, tid);
        return OAL_ERR_CODE_PTR_NULL;
    }
    ba_rx_stru = hmac_user->tx_tid_queue[tid].ba_rx_info;
    /* 初始化接收侧BA会话句柄 */
    hmac_init_rx_addba_req_ba(hmac_user, tid, payload, ba_rx_stru, hmac_vap);
    /* 回 rsp */
    hmac_rx_addba_req_reply(hmac_vap, hmac_user, tid, ba_rx_stru, hmac_device);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 抛事件到DMAC处理
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rx_addba_rsp_dispatch_event(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload)
{
    hmac_ctx_action_event_stru addba_rsp;
    osal_u16 baw_size;
    osal_u8 tidno = (payload[5] & 0x3C) >> 2; /* 5索引值 右移2位 */

    (osal_void)memset_s(&addba_rsp, OAL_SIZEOF(addba_rsp), 0, OAL_SIZEOF(addba_rsp));

    /******************************************************************/
    /*       ADDBA Response Frame - Frame Body                        */
    /* ---------------------------------------------------------------*/
    /* | Category | Action | Dialog | Status  | Parameters | Timeout| */
    /* ---------------------------------------------------------------*/
    /* | 1        | 1      | 1      | 2       | 2          | 2      | */
    /* ---------------------------------------------------------------*/
    /*                                                                */
    /******************************************************************/
    /* 获取帧体信息，由于DMAC的同步，填写事件payload */
    addba_rsp.action_category = MAC_ACTION_CATEGORY_BA;
    addba_rsp.action          = MAC_BA_ACTION_ADDBA_RSP;
    addba_rsp.user_idx        = hmac_user->assoc_id;
    addba_rsp.status          = payload[3];                /* 3索引值 */
    addba_rsp.tidno           = tidno;
    addba_rsp.dialog_token    = payload[2];                /* 2索引值 */
    addba_rsp.ba_policy       = ((payload[5] & 0x02) >> 1);         /* 5索引值 */
    addba_rsp.ba_timeout      = payload[7] | (payload[8] << 8); /* 7、8索引值 左移8位 */
    addba_rsp.amsdu_supp      = payload[5] & BIT0;                  /* 5索引值 */

     /* 5、6索引值 右移6位 左移2位 */
    baw_size = (osal_u16)(((payload[5] & 0xC0) >> 6) | (payload[6] << 2));
    if ((baw_size == 0) || (baw_size > WLAN_AMPDU_TX_MAX_BUF_SIZE)) {
        baw_size = WLAN_AMPDU_TX_MAX_BUF_SIZE;
    }

    addba_rsp.baw_size = baw_size;

    hmac_rx_process_ba_sync_event(hmac_vap, &addba_rsp);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_addba_rsp_ba_proc
 功能描述  : 处理host BA句柄
*****************************************************************************/
OAL_STATIC osal_void hmac_rx_addba_rsp_ba_proc(const osal_u8 *payload, hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, hmac_device_stru *hmac_device)
{
    /******************************************************************/
    /*       ADDBA Response Frame - Frame Body                        */
    /* ---------------------------------------------------------------*/
    /* | Category | Action | Dialog | Status  | Parameters | Timeout| */
    /* ---------------------------------------------------------------*/
    /* | 1        | 1      | 1      | 2       | 2          | 2      | */
    /* ---------------------------------------------------------------*/
    /*                                                                */
    /******************************************************************/
    oal_bool_enum_uint8 amsdu_supp = payload[5] & BIT0;        /* 5索引值 */
    osal_u8           tidno      = (payload[5] & 0x3C) >> 2; /* 5索引值 右移2位 */
    osal_u8           ba_status  = payload[3];               /* 3索引值 */
    hmac_tid_stru      *pst_tid       = &(hmac_user->tx_tid_queue[tidno]);
    unref_param(hmac_device);
    /* 先抛事件，再处理host BA句柄，防止异步发送ADDBA REQ */
    if (ba_status == MAC_SUCCESSFUL_STATUSCODE && pst_tid->ba_tx_info.ba_status != HMAC_BA_INIT) {
        /* 设置hmac模块对应的BA句柄的信息 */
        pst_tid->ba_tx_info.ba_status     = HMAC_BA_COMPLETE;
        pst_tid->ba_tx_info.addba_attemps = 0;

        if (amsdu_supp == OSAL_TRUE && mac_mib_get_AmsduPlusAmpduActive(hmac_vap) == OSAL_TRUE) {
            hmac_user_set_amsdu_support(hmac_user, tidno);
        } else {
            hmac_user_set_amsdu_not_support(hmac_user, tidno);
        }
    } else { /* BA被拒绝 */
        common_log_dbg3(0, OAM_SF_BA,
            "vap_id[%d] {hmac_rx_addba_rsp_ba_proc::addba rsp  status err[%d].tid[%d],DEL BA.}",
            hmac_vap->vap_id, ba_status, tidno);

        hmac_tx_ba_session_decr_etc(hmac_vap, tidno);

        /* 先抛事件删除dmac旧BA句柄后，再重置HMAC模块信息，确保删除dmac ba事件在下一次ADDBA REQ事件之前到达dmac */
        pst_tid->ba_tx_info.ba_status = HMAC_BA_INIT;
    }
}

OAL_STATIC osal_u32 hmac_rx_addba_rsp_ba_status_proc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload)
{
    osal_u8                   dialog_token;
    osal_u8                   ba_policy;
    osal_u8                   ba_status;
    osal_u8                   tidno;
    hmac_tid_stru              *pst_tid;
    /******************************************************************/
    /*       ADDBA Response Frame - Frame Body                        */
    /* ---------------------------------------------------------------*/
    /* | Category | Action | Dialog | Status  | Parameters | Timeout| */
    /* ---------------------------------------------------------------*/
    /* | 1        | 1      | 1      | 2       | 2          | 2      | */
    /* ---------------------------------------------------------------*/
    /*                                                                */
    /******************************************************************/
    tidno        = (payload[5] & 0x3C) >> 2; /* 5索引值 右移2位 */
    dialog_token = payload[2]; /* 2索引值 */
    ba_status    = payload[3]; /* 3索引值 */
    ba_policy    = ((payload[5] & 0x02) >> 1); /* 5索引值 */

    pst_tid = &(hmac_user->tx_tid_queue[tidno]);

    /* BA状态成功，但token、policy不匹配，需要删除聚合 */
    /* 使用超时机制去删除聚合。降低主动删除聚合后，立马收到匹配的ADDBA RSP，
    造成上述两个操作向DMAC抛事件，在DMAC中又不能保序执行带来的异常报错 */
    if ((pst_tid->ba_tx_info.ba_status == HMAC_BA_INPROGRESS) && (ba_status == MAC_SUCCESSFUL_STATUSCODE)) {
        if ((dialog_token != pst_tid->ba_tx_info.dialog_token) ||
            (ba_policy != pst_tid->ba_tx_info.ba_policy)) {
            common_log_dbg1(0, OAM_SF_BA,
                "{hmac_rx_addba_rsp_ba_status_proc::addba rsp tid[%d], status SUCC,but token/policy wr}", tidno);
            common_log_dbg4(0, OAM_SF_BA,
                "{hmac_rx_addba_rsp_ba_status_proc::rsp policy[%d],req policy[%d], rsp dialog[%d], req dialog[%d]}",
                ba_policy, pst_tid->ba_tx_info.ba_policy, dialog_token, pst_tid->ba_tx_info.dialog_token);
            return OAL_SUCC;
        }
    }

    /* 停止计时器 */
    if (pst_tid->ba_tx_info.addba_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&pst_tid->ba_tx_info.addba_timer);
    }

    if (pst_tid->ba_tx_info.ba_status == HMAC_BA_INIT) {
        common_log_dbg2(0, OAM_SF_BA,
            "vap_id[%d] {hmac_rx_addba_rsp_ba_status_proc::rsp is received when ba status is HMAC_BA_INIT.tid[%d]}",
            hmac_vap->vap_id, tidno);
        return OAL_SUCC;
    }

    if (pst_tid->ba_tx_info.ba_status == HMAC_BA_COMPLETE) {
        common_log_dbg2(0, OAM_SF_BA, "{hmac_rx_addba_rsp_ba_status_proc::addba rsp is received when ba status is \
            HMAC_BA_COMPLETE or dialog_token wrong.tid[%d], status[%d]}", tidno, pst_tid->ba_tx_info.ba_status);
        return OAL_SUCC;
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_rx_addba_rsp_etc
 功能描述  : 从空口接收ADDBA_RSP帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_rx_addba_rsp_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload)
{
    hmac_device_stru           *hmac_device = OAL_PTR_NULL;
    osal_u8                   tidno;
    osal_u32                  ul_ret;
    hmac_tid_stru               *pst_tid = OAL_PTR_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (payload == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_rx_addba_rsp_etc::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取device结构 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_addba_rsp_etc::hmac_device null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /******************************************************************/
    /*       ADDBA Response Frame - Frame Body                        */
    /* ---------------------------------------------------------------*/
    /* | Category | Action | Dialog | Status  | Parameters | Timeout| */
    /* ---------------------------------------------------------------*/
    /* | 1        | 1      | 1      | 2       | 2          | 2      | */
    /* ---------------------------------------------------------------*/
    /*                                                                */
    /******************************************************************/
    tidno = (payload[5] & 0x3C) >> 2; /* 5索引值 右移2位 */

    /* 协议支持tid为0~15,02只支持tid0~7 */
    if (tidno >= WLAN_TID_MAX_NUM) {
        /* 对于tid > 7的resp直接忽略 */
        oam_warning_log4(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_addba_rsp_etc::addba rsp tid[%d]} token[%d] state[%d]",
            hmac_vap->vap_id, tidno, payload[2], payload[3]);  /* 2、3索引值 */
        return OAL_SUCC;
    }

    pst_tid = &(hmac_user->tx_tid_queue[tidno]);
    /* 对tid对应的tx BA会话状态加锁 */
    osal_spin_lock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));

    ul_ret = hmac_rx_addba_rsp_ba_status_proc(hmac_vap, hmac_user, payload);
    if (ul_ret != OAL_CONTINUE) {
        /* 对tid对应的tx BA会话状态解锁 */
        osal_spin_unlock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));
        if (ul_ret == OAL_SUCC && pst_tid->ba_tx_info.ba_status == HMAC_BA_INIT) {
            hmac_tx_ba_del(hmac_vap, hmac_user, tidno);
        }
        return ul_ret;
    }

    /* 先抛事件到DMAC 防止异步发送ADDBA REQ */
    osal_spin_unlock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));
    ul_ret = hmac_rx_addba_rsp_dispatch_event(hmac_vap, hmac_user, payload);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_BA,
            "vap_id[%d] {hmac_mgmt_rx_addba_rsp_etc::hmac_rx_addba_rsp_dispatch_event fail.ret[%u]",
            hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }
    osal_spin_lock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));

    /* 再处理host BA句柄 */
    hmac_rx_addba_rsp_ba_proc(payload, hmac_vap, hmac_user, hmac_device);

    /* 对tid对应的tx BA会话状态解锁 */
    osal_spin_unlock(&(hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status_lock));

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_rx_delba_dispatch_event(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 tid, osal_u8 initiator)
{
    hmac_ctx_action_event_stru ba_info;

    (osal_void)memset_s(&ba_info, OAL_SIZEOF(ba_info), 0, OAL_SIZEOF(ba_info));

    ba_info.action_category  = MAC_ACTION_CATEGORY_BA;
    ba_info.action           = MAC_BA_ACTION_DELBA;
    ba_info.user_idx         = hmac_user->assoc_id;
    ba_info.tidno            = tid;
    ba_info.initiator        = initiator;

    hmac_rx_process_ba_sync_event(hmac_vap, &ba_info);
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_rx_delba_reset_ba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const osal_u8 *payload, hmac_device_stru *hmac_device)
{
    osal_u8      tid       = (payload[3] & 0xF0) >> 4; /* 3 tid索引值 右移4位 */
    osal_u8      initiator = (payload[3] & 0x08) >> 3; /* 3 initiator索引值 右移3位 */
    hmac_tid_stru *pst_tid      = &(hmac_user->tx_tid_queue[tid]);

    /* 重置BA发送会话 */
    if (initiator == MAC_RECIPIENT_DELBA) {
        if (pst_tid->ba_tx_info.ba_status == HMAC_BA_INIT) {
            return OAL_SUCC;
        }

        hmac_user->ba_flag[tid]  = 0;

        /* 还原设置AMPDU下AMSDU的支持情况 */
        hmac_user_set_amsdu_support(hmac_user, tid);

        hmac_tx_ba_session_decr_etc(hmac_vap, hmac_user->tx_tid_queue[tid].tid);
    } else { /* 重置BA接收会话 */
        hmac_ba_reset_rx_handle_etc(hmac_device, hmac_user, tid, OAL_FALSE);
        common_log_dbg1(0, OAM_SF_BA,
            "vap_id[%d] {hmac_rx_delba_reset_ba::rcv rx dir del ba.}\r\n", hmac_vap->vap_id);
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_rx_delba_etc
 功能描述  : 从空口接收DEL_BA帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_rx_delba_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *payload)
{
    hmac_device_stru             *hmac_device = OAL_PTR_NULL;
    hmac_tid_stru                *pst_tid = OAL_PTR_NULL;
    osal_u8                     tid;
    osal_u8                     initiator;
    osal_u16                    reason;
    osal_u32                    ul_ret;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) ||
        (payload == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_rx_delba_etc::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取device结构 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_delba_etc::device null.}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /************************************************/
    /*       DELBA Response Frame - Frame Body      */
    /* -------------------------------------------- */
    /* | Category | Action | Parameters | Reason  | */
    /* -------------------------------------------- */
    /* | 1        | 1      | 2          | 2       | */
    /* -------------------------------------------- */
    /*                                              */
    /************************************************/
    tid       = (payload[3] & 0xF0) >> 4; /* 3 tid索引值 右移4位 */
    initiator = (payload[3] & 0x08) >> 3; /* 3 initiator索引值 右移3位 */
    reason    = (payload[4] & 0xFF) | ((payload[5] << 8) & 0xFF00); /* 4、5reason索引值 偏移8位 */

    /* tid保护，避免数组越界 */
    if (tid >= WLAN_TID_MAX_NUM) {
        oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_delba_etc::delba receive failed, tid %d overflow.}",
                         hmac_vap->vap_id, tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }
    pst_tid      = &(hmac_user->tx_tid_queue[tid]);

    common_log_dbg4(0, OAM_SF_BA,
        "vap_id[%d] {hmac_mgmt_rx_delba_etc::receive delba from peer sta, tid[%d], initiator[%d], reason[%d].}",
        hmac_vap->vap_id, tid, initiator, reason);

    /* 对tid对应的tx BA会话状态加锁 */
    osal_spin_lock(&(pst_tid->ba_tx_info.ba_status_lock));

    /* 重置ba回话 */
    ul_ret = hmac_rx_delba_reset_ba(hmac_vap, hmac_user, payload, hmac_device);
    if (ul_ret != OAL_CONTINUE) {
        osal_spin_unlock(&(pst_tid->ba_tx_info.ba_status_lock));
        return ul_ret;
    }

    osal_spin_unlock(&(pst_tid->ba_tx_info.ba_status_lock));
    ul_ret = hmac_rx_delba_dispatch_event(hmac_vap, hmac_user, tid, initiator);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }
    osal_spin_lock(&(pst_tid->ba_tx_info.ba_status_lock));

    /* DELBA事件先处理再改状态,防止addba req先处理 */
    if (initiator == MAC_RECIPIENT_DELBA) {
        pst_tid->ba_tx_info.ba_status = HMAC_BA_INIT;
    }

    /* 对tid对应的tx BA会话状态解锁 */
    osal_spin_unlock(&(pst_tid->ba_tx_info.ba_status_lock));

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_tx_addba_timeout_etc
 功能描述  : 发送ADDBA req帧超时处理
*****************************************************************************/
osal_u32 hmac_mgmt_tx_addba_timeout_etc(osal_void *p_arg)
{
    hmac_vap_stru                      *hmac_vap = OAL_PTR_NULL;         /* vap指针 */
    osal_u8                          *da = OAL_PTR_NULL;          /* 保存用户目的地址的指针 */
    hmac_user_stru                     *hmac_user = OAL_PTR_NULL;
    mac_action_mgmt_args_stru           action_args;
    hmac_ba_alarm_stru                 *alarm_data = OAL_PTR_NULL;
    osal_u32 ret;

    if (p_arg == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_timeout_etc::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    alarm_data = (hmac_ba_alarm_stru *)p_arg;

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(alarm_data->mac_user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_timeout_etc::hmac_user[%d] null.}",
                       alarm_data->mac_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    da  = hmac_user->user_mac_addr;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(alarm_data->vap_id);
    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_tx_addba_timeout_etc::hmac_user null.}",
            hmac_user->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 生成DELBA帧 */
    action_args.category = MAC_ACTION_CATEGORY_BA;
    action_args.action   = MAC_BA_ACTION_DELBA;
    action_args.arg1     = alarm_data->tid;        /* 该数据帧对应的TID号 */
    action_args.arg2     = MAC_ORIGINATOR_DELBA;         /* DELBA中，触发删除BA会话的发起端 */
    action_args.arg3     = MAC_QSTA_TIMEOUT;                  /* DELBA中代表删除reason */
    action_args.arg5    = da;                        /* DELBA中代表目的地址 */

    ret = hmac_mgmt_tx_delba_etc(hmac_vap, hmac_user, &action_args);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_BA,
            "{hmac_mgmt_tx_addba_timeout_etc::hmac_mgmt_tx_delba_etc failed[%d].}", ret);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_tx_mgmt_send_event_etc
 功能描述  : 发送管理帧抛事件
 输入参数  : 无
 输出参数  : 无
 修改历史      :
  1.日    期   : 2013年5月29日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_tx_mgmt_send_event_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *mgmt_frame,
    osal_u16 frame_len)
{
    osal_u32 ret;
    osal_u8 addr1[6] = {0};
    osal_u8 subtype = 0;
#ifdef _PRE_WLAN_DFT_STAT
    osal_u8 *mac_header = OAL_PTR_NULL;
#endif
    mac_tx_ctl_stru *tx_ctrl = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_STATISTIC_RECORD);
    unref_param(frame_len);
    if ((hmac_vap == OAL_PTR_NULL) || (mgmt_frame == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_tx_mgmt_send_event_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_frame);

#ifdef _PRE_WLAN_DFT_STAT
    mac_header = oal_netbuf_header(mgmt_frame);
    subtype = mac_frame_get_subtype_value(mac_header);
    mac_get_address1(mac_header, addr1);
#endif
    mac_get_cb_frame_header_length(tx_ctrl) = MAC_80211_FRAME_LEN;
    ret = hmac_tx_send_encap_mgmt(hmac_vap, mgmt_frame);
    if (ret != OAL_SUCC) {
        wifi_printf("vap_id[%d] {hmac_tx_mgmt_send_event_etc::frw_send_mgmt failed[%d].}\r\n",
            hmac_vap->vap_id, ret);
        return ret;
    }
    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_statistic_record_cb)fhook)(hmac_vap->vap_id, addr1, subtype,
            hmac_vap->vap_mode, OAM_OTA_FRAME_DIRECTION_TYPE_TX);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_reset_psm_etc
 功能描述  : 收到认证请求 关联请求时 重置用户的节能状态
 输入参数  : 无
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年5月17日
    修改内容   : 增加keepalive重置
*****************************************************************************/
osal_u32 hmac_mgmt_reset_psm_etc(hmac_vap_stru *hmac_vap, osal_u16 user_id)
{
    (osal_void)hmac_psm_reset(hmac_vap, user_id);
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_sa_query_del_user
 功能描述  : 开始SA Query timer1超时删除user
 输入参数  : hmac_vap:挂此user的vap
             addr:需要删除的user mac地址
 输出参数  : 无
 返 回 值  : OAL_SUCC:删除用户成功
 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_PMF
OAL_STATIC osal_u32 hmac_sa_query_del_user(hmac_vap_stru *hmac_vap, hmac_user_stru  *hmac_user)
{
    wlan_vap_mode_enum_uint8 vap_mode;
    osal_u32 ret;
    mac_sa_query_stru *sa_query_info = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_PMF, "{hmac_sa_query_del_user::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* pending SA Query requests 计数器清零 & sa query流程开始时间清零 */
    sa_query_info = &hmac_user->sa_query_info;
    sa_query_info->sa_query_count      = 0;
    sa_query_info->sa_query_start_time = 0;

    /* 修改 state & 删除 user */
    vap_mode = hmac_vap->vap_mode;
    switch (vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
            /* 抛事件上报内核，已经去关联某个STA */
            hmac_handle_disconnect_rsp_ap_etc(hmac_vap, hmac_user);
            break;

        case WLAN_VAP_MODE_BSS_STA:
            /* 上报内核sta已经和某个ap去关联 */
            hmac_sta_handle_disassoc_rsp_etc(hmac_vap, MAC_DEAUTH_LV_SS);
            break;
        default:
            break;
    }

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->chip_id, hmac_user->user_mac_addr,
            PMF_SA_QUERY_TIMEOUT_DEL_USER, USER_CONN_OFFLINE);
    }
    /* 删除user */
    ret = hmac_user_del_etc(hmac_vap, hmac_user);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_PMF, "vap_id[%d] {hmac_sa_query_del_user::hmac_user_del_etc failed[%d].}",
                       hmac_vap->device_id, ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_send_sa_query_req
 功能描述  : hmac_start_sa_query的下级函数，没有设置timer的功能
 输入参数  : hmac_vap:启动SA查询的mac vap结构体
             da:目标user的mac 地址
             is_protected:SA Query帧的加密标志位

 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数
 *****************************************************************************/
OAL_STATIC osal_u32 hmac_send_sa_query_req(hmac_vap_stru *hmac_vap,
    hmac_user_stru  *hmac_user, oal_bool_enum_uint8 is_protected, osal_u16 init_trans_id)
{
    osal_u16                    sa_query_len;
    oal_netbuf_stru              *sa_query    = OAL_PTR_NULL;
    mac_tx_ctl_stru              *tx_ctl      = OAL_PTR_NULL;
    osal_u32                    ul_ret;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_PMF, "{hmac_send_sa_query_req::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请SA Query 帧空间 */
    sa_query = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
        OAL_NETBUF_PRIORITY_MID);
    if (sa_query == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_PMF, "vap_id[%d] {hmac_send_sa_query_req::sa_query null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 设置 trans id */
    hmac_user->sa_query_info.sa_query_trans_id = init_trans_id + 1;

    /* 更新sa query request计数器 */
    hmac_user->sa_query_info.sa_query_count += 1;
    oam_warning_log2(0, OAM_SF_PMF, "vap_id[%d] {hmac_send_sa_query_req::sa_query_count %d.}", hmac_vap->vap_id,
                     hmac_user->sa_query_info.sa_query_count);

    /* 封装SA Query request帧 */
    memset_s(oal_netbuf_cb(sa_query), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    sa_query_len = hmac_encap_sa_query_req_etc(hmac_vap,
        (osal_u8 *)OAL_NETBUF_HEADER(sa_query),
        hmac_user->user_mac_addr,
        hmac_user->sa_query_info.sa_query_trans_id);

    /* 单播管理帧加密 */
    if (is_protected == OAL_TRUE) {
        mac_set_protectedframe((osal_u8 *)OAL_NETBUF_HEADER(sa_query));
    }

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(sa_query); /* 获取cb结构体 */
    mac_get_cb_mpdu_len(tx_ctl)     = sa_query_len;               /* dmac发送需要的mpdu长度 */
    mac_get_cb_tx_user_idx(tx_ctl)  = (osal_u8)hmac_user->assoc_id; /* 发送完成需要获取user结构体 */

    oal_netbuf_put(sa_query, sa_query_len);

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, sa_query, sa_query_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(sa_query);
        oam_warning_log2(0, OAM_SF_PMF, "vap_id[%d] {hmac_send_sa_query_req::hmac_tx_mgmt_send_event_etc failed[%d].}",
                         hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_pmf_check_err_code_etc
 功能描述  : 判断是否对收到的去关联去认证帧进行PMF特性的处理
 输出参数  : 无
 返 回 值  : OAL_SUCC: 允许进行PMF特性的处理(如开启sa query流程)
 修改历史      :
  1.日    期   : 2014年6月25日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_pmf_check_err_code_etc(const hmac_user_stru *user_base_info, oal_bool_enum_uint8 is_protected,
    const osal_u8 *mac_hdr)
{
    oal_bool_enum_uint8 aim_err_code;
    osal_u16 err_code;

    err_code = OAL_MAKE_WORD16(mac_hdr[MAC_80211_FRAME_LEN], mac_hdr[MAC_80211_FRAME_LEN + 1]);
    aim_err_code = ((err_code == MAC_NOT_AUTHED) || (err_code == MAC_NOT_ASSOCED)) ? OAL_TRUE : OAL_FALSE;

    if ((user_base_info->cap_info.pmf_active == OAL_TRUE) && (aim_err_code == OAL_TRUE) &&
        (is_protected == OAL_FALSE)) {
        return OAL_SUCC;
    }

    return OAL_FAIL;
}

/*****************************************************************************
 函 数 名  : hmac_sa_query_interval_timeout_etc
 功能描述  : 开始SA Query timer2超时操作
 输入参数  : p_arg:timer2超时处理入参结构体
 输出参数  : 无
 返 回 值  : OAL_SUCC:超时处理成功
 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_sa_query_interval_timeout_etc(osal_void *p_arg)
{
    hmac_interval_timer_stru      *interval_timer_arg = OAL_PTR_NULL;
    osal_u32                     relt;
    osal_u32                     now;
    osal_u32                     passed;

    if (p_arg == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sa_query_interval_timeout_etc::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    interval_timer_arg = (hmac_interval_timer_stru *)p_arg;

    if ((interval_timer_arg->hmac_user == OAL_PTR_NULL) ||
        (interval_timer_arg->hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC)) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_sa_query_interval_timeout_etc::invalid param.}",
            interval_timer_arg->hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断是否超时 */
    now = (osal_u32)oal_time_get_stamp_ms();
    passed = (osal_u32)oal_time_get_runtime(interval_timer_arg->hmac_user->sa_query_info.sa_query_start_time, now);
    if (passed >= mac_mib_get_dot11AssociationSAQueryMaximumTimeout(interval_timer_arg->hmac_vap)) {
        /* change state & sa_query_count=0 */
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_sa_query_interval_timeout_etc::Deleting user.}",
            interval_timer_arg->hmac_vap->vap_id);
        relt = hmac_sa_query_del_user(interval_timer_arg->hmac_vap, interval_timer_arg->hmac_user);
        if (relt != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_sa_query_interval_timeout_etc::hmac_sa_query_del_user failed[%d].}",
                interval_timer_arg->hmac_vap->vap_id, relt);
            return OAL_ERR_CODE_PMF_SA_QUERY_DEL_USER_FAIL;
        }
        return OAL_SUCC;
    }

    /* 循环发送sa query request */
    relt = hmac_send_sa_query_req(interval_timer_arg->hmac_vap,
        interval_timer_arg->hmac_user,
        interval_timer_arg->is_protected,
        interval_timer_arg->trans_id);
    if (relt != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_sa_query_interval_timeout_etc::hmac_send_sa_query_req failed[%d].}",
            interval_timer_arg->hmac_vap->vap_id, relt);
        return OAL_ERR_CODE_PMF_SA_QUERY_REQ_SEND_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_set_sa_query_over_timer(hmac_vap_stru *hmac_vap, hmac_user_stru  *hmac_user,
    osal_u16 init_trans_id, oal_bool_enum_uint8 is_protected)
{
    hmac_interval_timer_stru     *interval_timer_arg = OAL_PTR_NULL;
    osal_u32                    retry_timeout;

    /* 设置timer超时函数入参 */
    interval_timer_arg = (hmac_interval_timer_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        OAL_SIZEOF(hmac_interval_timer_stru), OAL_TRUE);
    if (interval_timer_arg == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_set_sa_query_over_timer::interval_timer_arg null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    interval_timer_arg->hmac_vap     = hmac_vap;
    interval_timer_arg->hmac_user   = hmac_user;
    interval_timer_arg->is_protected = is_protected;
    interval_timer_arg->trans_id     = init_trans_id;
    retry_timeout = mac_mib_get_dot11AssociationSAQueryRetryTimeout(hmac_vap);

    /* 记录sa query流程开始时间,单位ms */
    hmac_user->sa_query_info.sa_query_start_time = (osal_u32)oal_time_get_stamp_ms();
    oam_warning_log2(0, OAM_SF_ANY,
        "vap_id[%d] {hmac_set_sa_query_over_timer::Start a SA query process, retry_timeout %d.}",
        hmac_vap->vap_id, retry_timeout);

    /* 设置间隔定时器 */
    frw_create_timer_entry(&(hmac_user->sa_query_info.sa_query_interval_timer),
        hmac_sa_query_interval_timeout_etc, retry_timeout, interval_timer_arg, OAL_TRUE);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_start_sa_query_etc
 功能描述  : 开始SA Query 查询流程
 输入参数  : hmac_vap:启动SA查询的mac vap结构体
             da:目标user的mac 地址
             is_protected:SA Query帧的加密标志位
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_start_sa_query_etc(hmac_vap_stru *hmac_vap, hmac_user_stru  *hmac_user,
    oal_bool_enum_uint8 is_protected)
{
    osal_u16                    init_trans_id;
    osal_u32                    ul_ret;

    /* 入参判断 */
    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_start_sa_query_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断vap有无pmf能力 */
    if (hmac_user->cap_info.pmf_active != OAL_TRUE) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_start_sa_query_etc::pmf_active is down.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PMF_DISABLED;
    }

    /* 避免重复启动SA Query流程 */
    if (hmac_user->sa_query_info.sa_query_count != 0) {
        oam_info_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_start_sa_query_etc::SA Query is already in process.}",
            hmac_vap->vap_id);
        return OAL_SUCC;
    }

    /* 获得初始trans_id */
    init_trans_id = (osal_u16)oal_time_get_stamp_ms();

    ul_ret = hmac_set_sa_query_over_timer(hmac_vap, hmac_user, init_trans_id, is_protected);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_start_sa_query_etc::hmac_set_sa_query_over_timer fail.}",
            hmac_vap->vap_id);
        return ul_ret;
    }

    /* 发送SA Query request，开始查询流程 */
    ul_ret = hmac_send_sa_query_req(hmac_vap, hmac_user, is_protected, init_trans_id);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_start_sa_query_etc::hmac_send_sa_query_req failed[%d].}",
            hmac_vap->vap_id, ul_ret);
        return OAL_ERR_CODE_PMF_SA_QUERY_REQ_SEND_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_send_sa_query_rsq
 功能描述  : 发送SA query response帧
 输入参数  : hmac_vap:发送SA query responsed的mac vap结构体
             hdr:接收的sa query request帧的帧头指针
             is_protected:SA Query帧的加密标志位
输出参数  : 无
 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_send_sa_query_rsp_etc(hmac_vap_stru *hmac_vap, osal_u8 *hdr, oal_bool_enum_uint8 is_protected)
{
    osal_u16                    sa_query_len;
    oal_netbuf_stru              *sa_query = OAL_PTR_NULL;
    mac_tx_ctl_stru              *tx_ctl      = OAL_PTR_NULL;
    osal_u16                    user_idx     = MAC_INVALID_USER_ID;
    osal_u32                    ul_ret;

    if (hmac_vap == OAL_PTR_NULL || hdr == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_send_sa_query_rsp_etc::param null.}");
        return;
    }
    oam_info_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_send_sa_query_rsp_etc::AP ready to TX a sa query rsp.}",
        hmac_vap->vap_id);

    sa_query = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
        OAL_NETBUF_PRIORITY_MID);
    if (sa_query == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_send_sa_query_rsp_etc::sa_query null.}", hmac_vap->vap_id);
        return;
    }

    memset_s(oal_netbuf_cb(sa_query), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    sa_query_len = hmac_encap_sa_query_rsp_etc(hmac_vap, hdr, (osal_u8 *)OAL_NETBUF_HEADER(sa_query));

    /* 单播管理帧加密 */
    if (is_protected == OAL_TRUE) {
        mac_set_protectedframe((osal_u8 *)OAL_NETBUF_HEADER(sa_query));
    }

    /* 填写正确的assoc id */
    ul_ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, ((mac_ieee80211_frame_stru *)hdr)->address2,
        &user_idx);
    if (ul_ret != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, sa_query);
        oam_error_log2(0, OAM_SF_ANY, "vap[%d] send_sa_query_rsp_etc::find user fail:%d.", hmac_vap->vap_id, ul_ret);
        oal_netbuf_free(sa_query);
        return;
    }

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(sa_query); /* 获取cb结构体 */
    mac_get_cb_mpdu_len(tx_ctl)     = sa_query_len;               /* dmac发送需要的mpdu长度 */
    mac_get_cb_tx_user_idx(tx_ctl)  = (osal_u8)user_idx;          /* 发送完成需要获取user结构体 */

    oal_netbuf_put(sa_query, sa_query_len);

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, sa_query, sa_query_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(sa_query);
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d]{hmac_send_sa_query_rsp_etc:hmac_tx_mgmt_send_event_etc failed[%d]}",
            hmac_vap->vap_id, ul_ret);
    }

    return;
}
#endif

OAL_STATIC osal_u32 hmac_pre_tx_deauth_netbuf(oal_netbuf_stru *deauth, hmac_vap_stru *hmac_vap, osal_u8 *da,
    osal_u16 err_code)
{
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;
    osal_u16 deauth_len;
    osal_u32 ul_ret;

    memset_s(oal_netbuf_cb(deauth), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    deauth_len = hmac_mgmt_encap_deauth_etc(hmac_vap, (osal_u8 *)OAL_NETBUF_HEADER(deauth), da,
        err_code);
    if (deauth_len == 0) {
        oam_error_log1(0, OAM_SF_AUTH, "vap_id[%d] {hmac_pre_tx_deauth_netbuf:: deauth_len = 0.}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oal_netbuf_put(deauth, deauth_len);

    /* 增加发送去认证帧时的维测信息 */
    oam_warning_log1(0, OAM_SF_CONN, "{hmac_mgmt_send_deauth_frame_etc:: status code[%d]}", err_code);
    oam_warning_log4(0, OAM_SF_CONN, "{hmac_mgmt_send_deauth_frame_etc:: DEAUTH TX : to %02X:%02X:%02X:%02X:XX:XX}",
        da[0], da[1], da[2], da[3]); /* 1、2、3mac地址位 */

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(deauth);                              /* 获取cb结构体 */
    mac_get_cb_mpdu_len(tx_ctl) = deauth_len;                                      /* dmac发送需要的mpdu长度 */
    /* 发送完成需要获取user结构体 */
    ul_ret = hmac_vap_set_cb_tx_user_idx(hmac_vap, tx_ctl, da);
    if (ul_ret != OAL_SUCC) {
    oam_warning_log1(0, OAM_SF_CONN, "vap_id[%d] {hmac_pre_tx_deauth_netbuf:: fail}", hmac_vap->vap_id);
    oam_warning_log4(0, OAM_SF_CONN, "{hmac_pre_tx_deauth_netbuf:: fail to find user by %02X:%02X:%02X:%02X:XX:XX}",
        da[0], da[1], da[2], da[3]); /* 1、2、3mac地址位 */
    }

    if (err_code == MAC_AUTH_TIMEOUT) {
        tx_ctl->is_get_from_ps_queue = OSAL_TRUE;
        tx_ctl->tx_user_idx = MAC_INVALID_USER_ID;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_send_deauth_frame_etc
 功能描述  : 发送去认证帧
 输入参数  : 无
 输出参数  : 无
 修改历史      :
  1.日    期   : 2013年7月1日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_mgmt_send_deauth_frame_etc(hmac_vap_stru *hmac_vap, osal_u8 *da, osal_u16 err_code,
    oal_bool_enum_uint8 is_protected)
{
    oal_netbuf_stru  *deauth = OAL_PTR_NULL;
    mac_tx_ctl_stru  *tx_ctl = OAL_PTR_NULL;
    osal_u32        ul_ret;
    unref_param(is_protected);

    if (hmac_vap == OAL_PTR_NULL || da == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_mgmt_send_deauth_frame_etc::param null.}");
        return;
    }

    /* 发送去认证前需确定vap处于工作状态 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log2(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_mgmt_send_deauth_frame_etc:: vap has been down/del, vap_state[%d].}",
            hmac_vap->vap_id, hmac_vap->vap_state);
        return;
    }

    deauth = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
        OAL_NETBUF_PRIORITY_MID);
    if (deauth == OAL_PTR_NULL) {
        /* Reserved Memory pool tried for high priority deauth frames */
        deauth = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
            OAL_NETBUF_PRIORITY_MID);
        if (deauth == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_AUTH, "vap_id[%d] {hmac_mgmt_send_deauth_frame_etc::deauth null.}",
                hmac_vap->vap_id);
            return;
        }
    }

    ul_ret = hmac_pre_tx_deauth_netbuf(deauth, hmac_vap, da, err_code);
    if (ul_ret != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, deauth);
        oam_error_log1(0, OAM_SF_AUTH, "vap_id[%d] {hmac_mgmt_send_deauth_frame_etc::hmac_pre_tx_deauth_netbuf fail.}",
            hmac_vap->vap_id);
        oal_netbuf_free(deauth);
        return;
    }

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(deauth);

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, deauth, mac_get_cb_mpdu_len(tx_ctl));
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(deauth);
        oam_warning_log2(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_mgmt_send_deauth_frame_etc::hmac_tx_mgmt_send_event_etc failed[%d].}",
            hmac_vap->vap_id, ul_ret);
    }
}

OAL_STATIC osal_u32 hmac_pre_tx_disassoc_netbuf(oal_netbuf_stru *disassoc, hmac_vap_stru *hmac_vap,
    osal_u8 *da, osal_u16 err_code)
{
    osal_u32        ul_ret;
    osal_u16        disassoc_len;
    mac_tx_ctl_stru  *tx_ctl = OAL_PTR_NULL;
    osal_u32        pedding_data = 0;

    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    msg_info.data = (osal_u8 *)&pedding_data;
    msg_info.data_len = OAL_SIZEOF(osal_u32);
    hmac_config_scan_abort_etc(hmac_vap, &msg_info);

    memset_s(oal_netbuf_cb(disassoc), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    disassoc_len = hmac_mgmt_encap_disassoc_etc(hmac_vap, (osal_u8 *)OAL_NETBUF_HEADER(disassoc), da,
        err_code);
    if (disassoc_len == 0) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_pre_tx_disassoc_netbuf:: disassoc_len = 0.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 增加发送去关联帧时的维测信息 */
    oam_warning_log1(0, OAM_SF_CONN, "{DISASSOC tx, Because of err_code[%d].}", err_code);
    oam_warning_log4(0, OAM_SF_CONN, "{DISASSOC tx, send disassoc frame to dest addr, da[%02X:%02X:%02X:%02X:XX:XX].}",
        da[0], da[1], da[2], da[3]); /* 1、2、3mac地址位 */

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(disassoc);
    mac_get_cb_mpdu_len(tx_ctl)  = disassoc_len;
    /* 填写非法值,发送完成之后获取用户为NULL,直接释放去认证帧  */
    ul_ret = hmac_vap_set_cb_tx_user_idx(hmac_vap, tx_ctl, da);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d] hmac_pre_tx_disassoc_netbuf fail", hmac_vap->vap_id);
        oam_warning_log4(0, OAM_SF_ASSOC,
            "{hmac_pre_tx_disassoc_netbuf::fail to find user by %02X:%02X:%02X:%02X:XX:XX.}",
            da[0], da[1], da[2], da[3]); /* 1、2、3mac地址位 */
    }

    oal_netbuf_put(disassoc, disassoc_len);
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_send_disassoc_frame_etc
 功能描述  : 发送去关联帧
 输入参数  : vap指针, DA, errcode
 输出参数  : 无
 返 回 值  : 无
 修改历史      :
  1.日    期   : 2014年1月2日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_mgmt_send_disassoc_frame_etc(hmac_vap_stru *hmac_vap, osal_u8 *da, osal_u16 err_code,
    oal_bool_enum_uint8 is_protected)
{
    oal_netbuf_stru  *disassoc = OAL_PTR_NULL;
    mac_tx_ctl_stru  *tx_ctl = OAL_PTR_NULL;
    osal_u32        ul_ret;
    unref_param(is_protected);

    if (hmac_vap == OAL_PTR_NULL || da == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_mgmt_send_disassoc_frame_etc::param null.}");
        return;
    }

    /* 发送去关联前需确定vap处于工作状态 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_mgmt_send_disassoc_frame_etc:: vap has been down/del, vap_state[%d].}",
            hmac_vap->vap_id, hmac_vap->vap_state);
        return;
    }

    disassoc = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
        OAL_NETBUF_PRIORITY_MID);
    if (disassoc == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_mgmt_send_disassoc_frame_etc::disassoc null.}",
            hmac_vap->vap_id);
        return;
    }

    ul_ret = hmac_pre_tx_disassoc_netbuf(disassoc, hmac_vap, da, err_code);
    if (ul_ret != OAL_CONTINUE) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, disassoc);
        oam_error_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_mgmt_send_disassoc_frame_etc::hmac_pre_tx_disassoc_netbuf fail}", hmac_vap->vap_id);
        oal_netbuf_free(disassoc);
        return;
    }

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(disassoc);

    /* 加入发送队列 */
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, disassoc, mac_get_cb_mpdu_len(tx_ctl));
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(disassoc);
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_mgmt_send_disassoc_frame_etc::hmac_tx_mgmt_send_event_etc failed[%d].}",
            hmac_vap->vap_id, ul_ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_update_assoc_user_qos_table_etc
 功能描述  : 更新对应关联实体的qos能力信息:函数的功能是在关联用户WMM使能的情况下
             往dmac抛事件写寄存器，如果关联用户之前就是WMM使能的，则不必重复写寄
             存器，如果之前不是WMM使能的，现在也不是WMM使能的，则不用写寄存器
 输入参数  : payload :指向帧体的指针
             msg_len  :帧的长度
             info_elem_offset:目前帧体位置的偏移
             hmac_user      :指向hmac_user的指针
 输出参数  : 无
 返 回 值  : osal_void
 修改历史      :
  1.日    期   : 2013年10月24日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_mgmt_update_assoc_user_qos_table_etc(osal_u8 *payload, osal_u16 msg_len,
    hmac_user_stru *hmac_user)
{
    osal_u8                                *ie                = OAL_PTR_NULL;
    hmac_vap_stru                             *hmac_vap           = OAL_PTR_NULL;

    /* 如果关联用户之前就是wmm使能的，什么都不用做，直接返回  */
    if (hmac_user->cap_info.qos == OAL_TRUE) {
        oam_info_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_mgmt_update_assoc_user_qos_table_etc::assoc user is wmm cap already.}",
            hmac_user->vap_id);
        return;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_mgmt_update_assoc_user_qos_table_etc::hmac_vap null.}",
            hmac_user->vap_id);
        return;
    }

    ie = hmac_get_wmm_ie_etc(payload, msg_len);
    if (ie != OAL_PTR_NULL) {
        /* 当用户关联的时候，要根据用户信息和AP自身信息来更新用户最终可用的qos信息。 */
        hmac_user_set_qos_etc(hmac_user,
            hmac_vap->mib_info->wlan_mib_sta_config.dot11_qos_option_implemented);
    } else if (is_sta(hmac_vap)) {
        /* 如果关联用户之前就是没有携带wmm ie, 再查找HT CAP能力 */
        ie = mac_find_ie_etc(MAC_EID_HT_CAP, payload, msg_len);
        if (ie != OAL_PTR_NULL) {
            /* 再查找HT CAP能力第2字节BIT 5 short GI for 20M 能力位 */
            if ((ie[1] >= 2) && ((ie[2] & BIT5) != 0)) {
                hmac_user_set_qos_etc(hmac_user, OAL_TRUE);
            }
        }
    }

    /* 如果关联用户到现在仍然不是wmm使能的，什么也不做，直接返回 */
    if (hmac_user->cap_info.qos == OAL_FALSE) {
        oam_info_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_mgmt_update_assoc_user_qos_table_etc::assoc user is not wmm cap.}",
            hmac_user->vap_id);
        return;
    }
}

/*****************************************************************************
 函 数 名  : hmac_check_bss_cap_info_etc
 功能描述  : check capabilities info field 中mac信息，如BSS type, Privacy等是否与VAP相符
 输入参数  : 无
 输出参数  : 无
 修改历史      :
  1.日    期   : 2013年12月6日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_check_bss_cap_info_etc(osal_u16 us_cap_info, hmac_vap_stru *hmac_vap)
{
    osal_u32                            ul_ret;
    wlan_mib_desired_bsstype_enum_uint8   bss_type;

    /* 获取CAP INFO里BSS TYPE */
    bss_type  =  hmac_get_bss_type_etc(us_cap_info);
    /* 比较BSS TYPE是否一致 不一致，如果是STA仍然发起入网，增加兼容性，其它模式则返回不支持 */
    if (bss_type != mac_mib_get_desired_bss_type(hmac_vap)) {
        oam_warning_log3(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_check_bss_cap_info_etc::vap_bss_type[%d] is different from asoc_user_bss_type[%d].}",
            hmac_vap->vap_id,
            mac_mib_get_desired_bss_type(hmac_vap), bss_type);
    }

    if (mac_mib_get_WPSActive(hmac_vap) == OAL_TRUE) {
        return OAL_TRUE;
    }

    /* 比较CAP INFO中privacy位，检查是否加密，加密不一致，返回失败 */
    ul_ret = hmac_check_mac_privacy_etc(us_cap_info, (osal_u8 *)hmac_vap);
    if (ul_ret != OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_check_bss_cap_info_etc::mac privacy capabilities is different.}", hmac_vap->vap_id);
    }

    return OAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_set_user_protocol_mode_etc
 功能描述  : 获取用户的协议模式
 输入参数  : 无
 输出参数  : 无
 修改历史      :
  1.日    期   : 2013年10月16日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_set_user_protocol_mode_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    /* 获取HT和VHT结构体指针 */
    mac_user_ht_hdl_stru         *mac_ht_hdl  = &(hmac_user->ht_hdl);
    mac_vht_hdl_stru             *pst_mac_vht_hdl = &(hmac_user->vht_hdl);
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru              *pst_mac_he_hdl  = &(hmac_user->he_hdl);

    if (pst_mac_he_hdl->he_capable == OAL_TRUE) {
        hmac_user_set_protocol_mode_etc(hmac_user, WLAN_HE_MODE);
    } else if (pst_mac_vht_hdl->vht_capable == OAL_TRUE)
#else
    if (pst_mac_vht_hdl->vht_capable == OAL_TRUE)
#endif
    {
        hmac_user_set_protocol_mode_etc(hmac_user, WLAN_VHT_MODE);
    } else if (mac_ht_hdl->ht_capable == OAL_TRUE) {
        hmac_user_set_protocol_mode_etc(hmac_user, WLAN_HT_MODE);
    } else if (hmac_vap->channel.band == WLAN_BAND_5G) {
        /* 判断是否是5G */
        hmac_user_set_protocol_mode_etc(hmac_user, WLAN_LEGACY_11A_MODE);
    } else {
        if (hmac_is_support_11grate_etc(hmac_user->op_rates.rs_rates,
            hmac_user->op_rates.rs_nrates) == OAL_TRUE) {
            hmac_user_set_protocol_mode_etc(hmac_user, WLAN_LEGACY_11G_MODE);
            if (hmac_is_support_11brate_etc(hmac_user->op_rates.rs_rates,
                hmac_user->op_rates.rs_nrates) == OAL_TRUE) {
                hmac_user_set_protocol_mode_etc(hmac_user, WLAN_MIXED_ONE_11G_MODE);
            }
        } else if (hmac_is_support_11brate_etc(hmac_user->op_rates.rs_rates,
            hmac_user->op_rates.rs_nrates) == OAL_TRUE) {
            hmac_user_set_protocol_mode_etc(hmac_user, WLAN_LEGACY_11B_MODE);
        } else {
            oam_warning_log1(0, OAM_SF_ANY, "vap[%d] {hmac_set_user_protocol_mode_etc:fail}", hmac_vap->vap_id);
        }
    }

    oam_warning_log3(0, OAM_SF_ASSOC, "vap[%d] {hmac_set_user_protocol_mode_etc: protocol[%d],protocol_mode[%d]}",
        hmac_vap->vap_id, hmac_vap->protocol, hmac_user->protocol_mode);

    /* 兼容性问题：思科AP 2.4G（11b）和5G(11a)共存时发送的assoc rsp帧携带的速率分别是11g和11b，导致STA创建用户时通知算法失败，
    Autorate失效，DBAC情况下，DBAC无法启动已工作的VAP状态无法恢复的问题 临时方案，建议针对对端速率异常的情况统一分析优化 */
    if (((hmac_user->protocol_mode == WLAN_LEGACY_11B_MODE) && (hmac_vap->protocol == WLAN_LEGACY_11A_MODE)) ||
        ((hmac_user->protocol_mode == WLAN_LEGACY_11G_MODE) && (hmac_vap->protocol == WLAN_LEGACY_11B_MODE))) {
        hmac_user_set_protocol_mode_etc(hmac_user, hmac_vap->protocol);
        (osal_void)memcpy_s((osal_void *)&(hmac_user->op_rates), OAL_SIZEOF(mac_rate_stru),
            (osal_void *)&(hmac_vap->curr_sup_rates.rate), OAL_SIZEOF(mac_rate_stru));
    }

    /* 兼容性问题：ws880配置11a时beacon和probe resp帧中协议vht能力，vap的protocol能力要根据关联响应帧的实际能力刷新成实际11a能力 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (hmac_vap->protocol == WLAN_VHT_MODE) && (hmac_user->protocol_mode == WLAN_LEGACY_11A_MODE)) {
        hmac_vap_init_by_protocol_etc(hmac_vap, hmac_user->protocol_mode);
    }
}
/*****************************************************************************
 函 数 名  : hmac_user_init_rates_etc
 功能描述  : 初始化用户速率集
*****************************************************************************/
osal_void hmac_user_init_rates_etc(hmac_user_stru *hmac_user)
{
    memset_s((osal_u8 *)(&hmac_user->op_rates), OAL_SIZEOF(hmac_user->op_rates), 0, OAL_SIZEOF(hmac_user->op_rates));
}

#ifdef _PRE_WLAN_FEATURE_PMF
/*****************************************************************************
 函 数 名  : hmac_rx_sa_query_req_etc
 功能描述  : AP在UP状态下的接收SA Query request帧处理
 输入参数  : hmac_vap: HMAC VAP指针
             pst_netbuf  : 管理帧所在的sk_buff
             is_protected 此管理帧是否受单播保护
*****************************************************************************/
osal_void hmac_rx_sa_query_req_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_bool_enum_uint8 is_protected)
{
    osal_u8                   *sa  = OAL_PTR_NULL;
    hmac_user_stru              *hmac_user = OAL_PTR_NULL;
    osal_u8                   *mac_hdr = OAL_PTR_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_sa_query_req_etc::null param.}");
        return;
    }

    mac_hdr = OAL_NETBUF_HEADER(pst_netbuf);

    mac_rx_get_sa((mac_ieee80211_frame_stru *)mac_hdr, &sa);
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, sa);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_RX,
            "vap_id[%d] {hmac_rx_sa_query_req_etc::hmac_user null.}", hmac_vap->vap_id);
        return;
    }

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    if (is_protected != hmac_user->cap_info.pmf_active) {
        oam_error_log1(0, OAM_SF_RX,
            "vap_id[%d] {hmac_rx_sa_query_req_etc::PMF check failed.}", hmac_vap->vap_id);
        return;
    }

    /* sa Query rsp发送 */
    hmac_send_sa_query_rsp_etc(hmac_vap, mac_hdr, is_protected);

    return;
}
/*****************************************************************************
    函 数 名  : hmac_rx_sa_query_rsp_etc
    功能描述  : UP状态下的接收SA Query rsponse帧处理
    输入参数  : hmac_vap   : HMAC VAP指针
                pst_netbuf     : 管理帧所在的sk_buff
                is_protected: 单播管理帧加密的标志位
*****************************************************************************/
osal_void hmac_rx_sa_query_rsp_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_bool_enum_uint8 is_protected)
{
    osal_u8                   *mac_hdr = OAL_PTR_NULL;
    osal_u8                   *sa = OAL_PTR_NULL;
    hmac_user_stru              *hmac_user = OAL_PTR_NULL;
    osal_u16                  *pus_trans_id = OAL_PTR_NULL;
    mac_sa_query_stru           *sa_query_info = OAL_PTR_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_AMPDU, "{hmac_rx_sa_query_rsp_etc::param null.}");
        return;
    }

    mac_hdr  = OAL_NETBUF_HEADER(pst_netbuf);

    mac_rx_get_sa((mac_ieee80211_frame_stru *)mac_hdr, &sa);
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, sa);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_AMPDU, "vap_id[%d] {hmac_rx_sa_query_rsp_etc::hmac_user null.}",
            hmac_vap->vap_id);
        return;
    }

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    if (is_protected != hmac_user->cap_info.pmf_active) {
        oam_error_log1(0, OAM_SF_AMPDU, "vap_id[%d] {hmac_rx_sa_query_rsp_etc::PMF check failed.}",
            hmac_vap->vap_id);
        return;
    }

    /* 对比trans_id */
    pus_trans_id = (osal_u16 *)(mac_hdr + MAC_80211_FRAME_LEN + 2); /* 2id长度 */
    sa_query_info = &hmac_user->sa_query_info;

    /* 收到有效的SA query reqponse，保留这条有效的SA */
    if (osal_memcmp(pus_trans_id, &(sa_query_info->sa_query_trans_id), 2) == 0) { /* 2id长度 */
        /* pending SA Query requests 计数器清零 & sa query流程开始时间清零 */
        sa_query_info->sa_query_count      = 0;
        sa_query_info->sa_query_start_time = 0;

        /* 删除timer */
        if (sa_query_info->sa_query_interval_timer.is_registerd != OAL_FALSE) {
            frw_destroy_timer_entry(&(sa_query_info->sa_query_interval_timer));
        }

        /* 删除timers的入参存储空间 */
        if (sa_query_info->sa_query_interval_timer.timeout_arg != OAL_PTR_NULL) {
            oal_mem_free((osal_void *)sa_query_info->sa_query_interval_timer.timeout_arg, OAL_TRUE);
            sa_query_info->sa_query_interval_timer.timeout_arg = OAL_PTR_NULL;
        }
    }
    return;
}
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 函 数 名 : hmac_mgmt_rx_mu_edca_control_etc
 功能描述     : 解析mu edca control frame并将解析结果下发到dmac
*****************************************************************************/
osal_void  hmac_mgmt_rx_mu_edca_control_etc(hmac_vap_stru *hmac_vap, osal_u8 *payload_len)
{
    osal_u8 *payload = payload_len;
    osal_u8 affected_acs = 0;
    mac_mu_edca_control *mu_edca_ctl_field = (mac_mu_edca_control *)&affected_acs;
    mac_mu_edca_control tmp_edca_control_field;

    memset_s(&tmp_edca_control_field, sizeof(tmp_edca_control_field), 0, sizeof(tmp_edca_control_field));

    if (mac_mib_get_he_option_implemented(hmac_vap) == OSAL_FALSE) {
        return;
    }

    payload += MAC_ACTION_OFFSET_BODY;
    (osal_void)memcpy_s((osal_u8 *)(&tmp_edca_control_field), sizeof(mac_mu_edca_control),
        payload, sizeof(mac_mu_edca_control));

    /* 协议中规定的B0~B3分别对应BK、BE、VI、VO，而硬件刚好相反，对应VO、VI、BE、BK，因此要做个翻转 */
    mu_edca_ctl_field->affected_acs_b0 = tmp_edca_control_field.affected_acs_b3;
    mu_edca_ctl_field->affected_acs_b1 = tmp_edca_control_field.affected_acs_b2;
    mu_edca_ctl_field->affected_acs_b2 = tmp_edca_control_field.affected_acs_b1;
    mu_edca_ctl_field->affected_acs_b3 = tmp_edca_control_field.affected_acs_b0;

    hal_set_affected_acs(affected_acs);

    return;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_send_mgmt_to_host_etc
 功能描述  : 上报接收到的管理帧
 输入参数  : hmac_vap_stru  *hmac_vap,
             osal_u8      *buf,
             osal_u16      len,
             osal_slong         l_freq
*****************************************************************************/
osal_void hmac_send_mgmt_to_host_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf, osal_u16 len,
    osal_slong l_freq)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    mac_rx_ctl_stru *rx_info = OAL_PTR_NULL;
    osal_u8 *mgmt_data = OAL_PTR_NULL;
    osal_s32 ret;
    frw_msg msg_info = {0};
    hmac_rx_mgmt_event_stru mgmt_frame = {0};

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_send_mgmt_to_host_etc::hmac_device null.}",
            hmac_vap->vap_id);
        /* 管理帧丢弃 */
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, buf);
        return;
    }

    rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb(buf);
    mgmt_data = (osal_u8 *)oal_memalloc(len);
    if (mgmt_data == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_send_mgmt_to_host_etc::mgmt_data null.}",
            hmac_vap->vap_id);
        /* 管理帧丢弃 */
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, buf);
        return;
    }
    (osal_void)memcpy_s(mgmt_data, len, (osal_u8 *)mac_get_rx_cb_mac_header_addr(rx_info), len);

    mgmt_frame.buf = (osal_u8 *)mgmt_data;
    mgmt_frame.len  = len;
    mgmt_frame.l_freq  = l_freq;

    OAL_NETBUF_LEN(buf) = len;

    (osal_void)memcpy_s(mgmt_frame.ac_name, OAL_IF_NAME_SIZE, hmac_vap->net_device->name, OAL_IF_NAME_SIZE);

    msg_info.data = (osal_u8 *)&mgmt_frame;
    msg_info.data_len = OAL_SIZEOF(hmac_rx_mgmt_event_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_RX_MGMT, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        oal_free(mgmt_data);
#ifdef _PRE_WLAN_DFT_STAT
        hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_M_SEND_WAL_FAIL_PKTS);
    } else {
        hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_M_SEND_WAL_SUCC_PKTS);
#endif
    }
    return;
}

OAL_STATIC osal_void hmac_fill_tx_pwas_mgmt_netbuf(oal_netbuf_stru *netbuf_mgmt_tx, hmac_vap_stru *hmac_vap,
    const mac_mgmt_frame_stru *pst_mgmt_tx)
{
    osal_u16       user_idx;
    // 解决管理帧发送过程中已关联的用户user_index的问题，从而解决向已关联用户发送管理帧时节能缓存的问题
    osal_u8        ua_dest_mac_addr[WLAN_MAC_ADDR_LEN];
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;

    memset_s(oal_netbuf_cb(netbuf_mgmt_tx), OAL_SIZEOF(mac_tx_ctl_stru), 0, OAL_SIZEOF(mac_tx_ctl_stru));

    /* 填充netbuf */
    (osal_void)memcpy_s((osal_u8 *)OAL_NETBUF_HEADER(netbuf_mgmt_tx), pst_mgmt_tx->len,
        pst_mgmt_tx->frame, pst_mgmt_tx->len);
    oal_netbuf_put(netbuf_mgmt_tx, pst_mgmt_tx->len);
    /* 解决管理帧发送过程中已关联的用户user_index的问题，从而解决向已关联用户发送管理帧时节能缓存的问题 */
    mac_get_address1((osal_u8 *)pst_mgmt_tx->frame, ua_dest_mac_addr);
    user_idx = MAC_INVALID_USER_ID;
    /* 管理帧可能发给已关联的用户，也可能发给未关联的用户。已关联的用户可以找到，未关联的用户找不到。不用判断返回值 */
    hmac_vap_find_user_by_macaddr_etc(hmac_vap, ua_dest_mac_addr, &user_idx);

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf_mgmt_tx);    /* 获取cb结构体 */
    mac_get_cb_mpdu_len(tx_ctl)       = pst_mgmt_tx->len;         /* dmac发送需要的mpdu长度 */
    mac_get_cb_tx_user_idx(tx_ctl)    = (osal_u8)user_idx;                 /* 发送完成需要获取user结构体 */
    mac_get_cb_is_need_resp(tx_ctl)   = OAL_TRUE;
    mac_get_cb_is_needretry(tx_ctl)   = OAL_TRUE;
    mac_get_cb_mgmt_frame_id(tx_ctl)  = pst_mgmt_tx->mgmt_frame_id;
    /* 仅用subtype做识别, 置action会误入dmac_tx_action_process */
    mac_get_cb_frame_type(tx_ctl)     = WLAN_CB_FRAME_TYPE_MGMT;
    mac_get_cb_frame_subtype(tx_ctl)  = WLAN_ACTION_P2PGO_FRAME_SUBTYPE;
}

/*****************************************************************************
 函 数 名  : hmac_wpas_mgmt_tx_etc
 功能描述  : 发送wpa_supplicant action frame
*****************************************************************************/
osal_s32 hmac_wpas_mgmt_tx_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_netbuf_stru                *netbuf_mgmt_tx = OAL_PTR_NULL;
    osal_u32                      ul_ret;
    mac_mgmt_frame_stru            *pst_mgmt_tx = OAL_PTR_NULL;
    hmac_device_stru                *hmac_device = OAL_PTR_NULL;

    if (hmac_vap == OAL_PTR_NULL || msg->data == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{hmac_wpas_mgmt_tx_etc::param null.}");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_DFT_STAT
    hmac_pkts_tx_stat(hmac_vap->vap_id, 1, TX_M_FROM_WAL_PKTS);
#endif
    pst_mgmt_tx = (mac_mgmt_frame_stru *)msg->data;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_P2P, "vap_id[%d] {hmac_wpas_mgmt_tx_etc::hmac_device null.}", hmac_vap->vap_id);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        return OAL_ERR_CODE_PTR_NULL;
    }

    wifi_printf("vap_id[%d] {hmac_wpas_mgmt_tx_etc::mgmt frame id=[%d]}\r\n", hmac_vap->vap_id,
        pst_mgmt_tx->mgmt_frame_id);

    /* 申请netbuf 空间 */
    netbuf_mgmt_tx = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, pst_mgmt_tx->len,
        OAL_NETBUF_PRIORITY_MID);
    if (netbuf_mgmt_tx == OAL_PTR_NULL) {
        /* Reserved Memory pool tried for high priority deauth frames */
        netbuf_mgmt_tx = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
            OAL_NETBUF_PRIORITY_MID);
        if (netbuf_mgmt_tx == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_P2P, "vap_id[%d] {hmac_wpas_mgmt_tx_etc::pst_mgmt_tx null.}", hmac_vap->vap_id);
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
            return OAL_FAIL;
        }
    }

    /* 填充netbuf_mgmt_tx */
    hmac_fill_tx_pwas_mgmt_netbuf(netbuf_mgmt_tx, hmac_vap, pst_mgmt_tx);

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, netbuf_mgmt_tx, pst_mgmt_tx->len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(netbuf_mgmt_tx);
        oam_warning_log2(0, OAM_SF_P2P, "vap_id[%d] {hmac_wpas_mgmt_tx_etc::hmac_tx_mgmt_send_event_etc failed[%d].}",
            hmac_vap->vap_id, ul_ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#if defined(_PRE_WLAN_FEATURE_HS20) || defined(_PRE_WLAN_FEATURE_P2P) || defined(_PRE_WLAN_FEATURE_WPA3)
/*****************************************************************************
 函 数 名  : hmac_rx_mgmt_send_to_host_etc
 功能描述  : 接收到管理帧并上报host
 输入参数  : hmac_vap_stru *hmac_vap
             oal_netbuf_stru *pst_netbuf
*****************************************************************************/
osal_void hmac_rx_mgmt_send_to_host_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru            *rx_info = OAL_PTR_NULL;
    osal_slong                     l_freq;

    rx_info  = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    l_freq = oal_ieee80211_channel_to_frequency(rx_info->channel_number,
        (rx_info->channel_number > 14) ? OAL_IEEE80211_BAND_5GHZ : OAL_IEEE80211_BAND_2GHZ); /* 14信道号 */
    hmac_send_mgmt_to_host_etc(hmac_vap, pst_netbuf, rx_info->frame_len, l_freq);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_mgmt_tx_event_status_etc
 功能描述  : 处理dmac上报管理帧发送的结果,上报WAL
*****************************************************************************/
osal_s32 hmac_mgmt_tx_event_status_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    dmac_tx_status_stru *tx_status_param = OAL_PTR_NULL;
    osal_s32 ret;

    if (msg->data == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{hmac_mgmt_tx_event_status_etc::param is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    tx_status_param = (dmac_tx_status_stru *)msg->data;
    oam_warning_log4(0, OAM_SF_P2P,
        "vap_id[%d] {dmac tx mgmt status report.userindx[%d], tx mgmt status[%d], frame_id[%d]}",
        hmac_vap->vap_id,
        tx_status_param->user_idx,
        tx_status_param->dscr_status,
        tx_status_param->mgmt_frame_id);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_MGMT_TX_STATUS, FRW_POST_PRI_LOW, hmac_vap->vap_id, msg);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_vap_set_user_avail_rates_etc
 功能描述  : 获取用户和VAP可用的11a/b/g公共速率
*****************************************************************************/
osal_void hmac_vap_set_user_avail_rates_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    mac_curr_rateset_stru *hmac_vap_rate = OAL_PTR_NULL;
    hmac_rate_stru         *hmac_user_rate = OAL_PTR_NULL;
    mac_rate_stru          avail_op_rates;
    osal_u8              mac_vap_rate_num;
    osal_u8              hmac_user_rate_num;
    osal_u8              vap_rate_idx;
    osal_u8              user_rate_idx;
    osal_u8              user_avail_rate_idx = 0;

    /* 获取VAP和USER速率的结构体指针 */
    hmac_vap_rate    = &(hmac_vap->curr_sup_rates);
    hmac_user_rate   = &(hmac_user->op_rates);
    memset_s((osal_u8 *)(&avail_op_rates), OAL_SIZEOF(mac_rate_stru), 0, OAL_SIZEOF(mac_rate_stru));

    mac_vap_rate_num     = hmac_vap_rate->rate.rs_nrates;
    hmac_user_rate_num    = hmac_user_rate->rs_nrates;

    for (vap_rate_idx = 0; vap_rate_idx < mac_vap_rate_num; vap_rate_idx++) {
        for (user_rate_idx = 0; user_rate_idx < hmac_user_rate_num; user_rate_idx++) {
            if ((hmac_vap_rate->rate.rs_rates[vap_rate_idx].mac_rate > 0) &&
                (is_equal_rates(hmac_vap_rate->rate.rs_rates[vap_rate_idx].mac_rate,
                    hmac_user_rate->rs_rates[user_rate_idx]))) {
                avail_op_rates.rs_rates[user_avail_rate_idx] =
                    hmac_user_rate->rs_rates[user_rate_idx];
                user_avail_rate_idx++;
                avail_op_rates.rs_nrates++;
            }
        }
    }

    hmac_user_set_avail_op_rates_etc(hmac_user, avail_op_rates.rs_nrates, avail_op_rates.rs_rates);
    oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_vap_set_user_avail_rates_etc::avail_op_rates_num=[%d].}",
        hmac_vap->vap_id, avail_op_rates.rs_nrates);
}

OAL_STATIC osal_void hmac_vht_cap_ie_config_part_vht_hdl(hmac_vap_stru *hmac_vap, const osal_u8 *vht_cap_ie,
    mac_vht_hdl_stru *pst_mac_vht_hdl, osal_u16 msg_idx, hmac_user_stru *hmac_user)
{
    /* 解析VHT capablities info field */
    osal_u16 vht_cap_filed_low = OAL_MAKE_WORD16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]);
    /* 2、3取vht_cap_ie对应比特位 */
    osal_u16 vht_cap_filed_high = OAL_MAKE_WORD16(vht_cap_ie[msg_idx + 2], vht_cap_ie[msg_idx + 3]);
    osal_u32 vht_cap_field = OAL_MAKE_WORD32(vht_cap_filed_low, vht_cap_filed_high);

    /* 解析max_mpdu_length 参见11ac协议 Table 8-183u */
    pst_mac_vht_hdl->max_mpdu_length_flag = (vht_cap_field & (BIT1 | BIT0));
    if (pst_mac_vht_hdl->max_mpdu_length_flag == 0) {
        pst_mac_vht_hdl->max_mpdu_length = 3895; /* 3895表示pudu最大长度 */
    } else if (pst_mac_vht_hdl->max_mpdu_length_flag == 1) {
        pst_mac_vht_hdl->max_mpdu_length = 7991; /* 7991表示pudu最大长度 */
    } else if (pst_mac_vht_hdl->max_mpdu_length_flag == 2) {  /* 当bit_max_mpdu_length为2 */
        pst_mac_vht_hdl->max_mpdu_length = 11454; /* 11454表示pudu最大长度 */
    }

    /* 解析supported_channel_width */
    pst_mac_vht_hdl->supported_channel_width = ((vht_cap_field & (BIT3 | BIT2)) >> 2);  /* 右移2 */

    /* 解析rx_ldpc */
    pst_mac_vht_hdl->rx_ldpc = ((vht_cap_field & BIT4) >> 4);  /* 右移4 */

    /* 解析short_gi_80mhz和short_gi_160mhz支持情况 */
    pst_mac_vht_hdl->short_gi_80mhz = ((vht_cap_field & BIT5) >> 5);  /* 右移5 */
    pst_mac_vht_hdl->short_gi_80mhz &= mac_mib_get_vht_short_gi_option_in80_implemented(hmac_vap);

    pst_mac_vht_hdl->short_gi_160mhz = OAL_FALSE;
    /* 解析tx_stbc 和rx_stbc */
    pst_mac_vht_hdl->tx_stbc = ((vht_cap_field & BIT7) >> 7); /* 右移7 */
    pst_mac_vht_hdl->rx_stbc = ((vht_cap_field & (BIT10 | BIT9 | BIT8)) >> 8);  /* 右移8 */

    /* 解析su_beamformer_cap和su_beamformee_cap */
    pst_mac_vht_hdl->su_beamformer_cap = ((vht_cap_field & BIT11) >> 11);  /* 右移11 */
    pst_mac_vht_hdl->su_beamformee_cap = ((vht_cap_field & BIT12) >> 12);  /* 右移12 */

    /* 解析num_bf_ant_supported */
    pst_mac_vht_hdl->num_bf_ant_supported = ((vht_cap_field & (BIT15 | BIT14 | BIT13)) >> 13);  /* 右移13 */

    /* 以对端天线数初始化可用最大空间流 */
    hmac_user->avail_bf_num_spatial_stream = pst_mac_vht_hdl->num_bf_ant_supported;

    /* 解析num_sounding_dim */
    pst_mac_vht_hdl->num_sounding_dim = ((vht_cap_field & (BIT18 | BIT17 | BIT16)) >> 16);  /* 右移16 */

    /* 解析mu_beamformer_cap和mu_beamformee_cap */
    pst_mac_vht_hdl->mu_beamformer_cap = ((vht_cap_field & BIT19) >> 19);  /* 右移19 */
    pst_mac_vht_hdl->mu_beamformee_cap = ((vht_cap_field & BIT20) >> 20);  /* 右移20 */

    /* 解析vht_txop_ps */
    pst_mac_vht_hdl->vht_txop_ps = ((vht_cap_field & BIT21) >> 21);  /* 右移21 */
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    if ((pst_mac_vht_hdl->vht_txop_ps == OAL_TRUE) && (mac_mib_get_txopps(hmac_vap) == OAL_TRUE)) {
        hmac_vap_set_txopps(hmac_vap, OAL_TRUE);
    }
#endif
    /* 解析htc_vht_capable */
    pst_mac_vht_hdl->htc_vht_capable = ((vht_cap_field & BIT22) >> 22);  /* 右移22 */

    /* 解析max_ampdu_len_exp */
    pst_mac_vht_hdl->max_ampdu_len_exp = ((vht_cap_field & (BIT25 | BIT24 | BIT23)) >> 23);  /* 右移23 */

    /* 解析vht_link_adaptation */
    pst_mac_vht_hdl->vht_link_adaptation = ((vht_cap_field & (BIT27 | BIT26)) >> 26); /* 右移26 */

    /* 解析rx_ant_pattern */
    pst_mac_vht_hdl->rx_ant_pattern = ((vht_cap_field & BIT28) >> 28); /* 右移28 */

    /* 解析tx_ant_pattern */
    pst_mac_vht_hdl->tx_ant_pattern = ((vht_cap_field & BIT29) >> 29); /* 右移29 */
}

/*****************************************************************************
 函 数 名  : hmac_proc_vht_cap_ie_etc
 功能描述  : 解析的11ac capabilities IE信息
*****************************************************************************/
osal_u32 hmac_proc_vht_cap_ie_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *vht_cap_ie)
{
    mac_vht_hdl_stru   *pst_mac_vht_hdl = OAL_PTR_NULL;
    mac_vht_hdl_stru    mac_vht_hdl;
    osal_u16          rx_mcs_map;
    osal_u16          tx_mcs_map;
    osal_u16          rx_high_log_gi_data;
    osal_u16          tx_high_log_gi_data;
    osal_u16          msg_idx = 0;

    /* 解析vht cap IE */
    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (vht_cap_ie == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_proc_vht_cap_ie_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (vht_cap_ie[1] < MAC_VHT_CAP_IE_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_proc_vht_cap_ie_etc::invalid vht cap ie len[%d].}", vht_cap_ie[1]);
        return OAL_FAIL;
    }

    /* 支持11ac，才进行后续的处理 */
    if (mac_mib_get_vht_option_implemented(hmac_vap) == OAL_FALSE) {
        return OAL_SUCC;
    }

    pst_mac_vht_hdl = &mac_vht_hdl;
    hmac_user_get_vht_hdl_etc(hmac_user, pst_mac_vht_hdl);

    /* 进入此函数代表user支持11ac */
    pst_mac_vht_hdl->vht_capable = OAL_TRUE;

#ifdef _PRE_WLAN_FEATURE_11AC2G
    /* 定制化实现如果不支持11ac2g模式，则关掉vht cap */
    if ((hmac_vap->cap_flag.support_11ac2g == OAL_FALSE) &&
        (hmac_vap->channel.band == WLAN_BAND_2G)) {
        pst_mac_vht_hdl->vht_capable = OAL_FALSE;
    }
#endif

    msg_idx += MAC_IE_HDR_LEN;

    hmac_vht_cap_ie_config_part_vht_hdl(hmac_vap, vht_cap_ie, pst_mac_vht_hdl, msg_idx, hmac_user);

    msg_idx += MAC_VHT_CAP_INFO_FIELD_LEN;

    /* 解析VHT Supported MCS Set field */

    /* 解析rx_mcs_map; 修改rx mcs map解析错误问题 */
    rx_mcs_map = OAL_MAKE_WORD16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]);

    (osal_void)memcpy_s(&(pst_mac_vht_hdl->rx_max_mcs_map), OAL_SIZEOF(mac_rx_max_mcs_map_stru),
        &rx_mcs_map, OAL_SIZEOF(mac_rx_max_mcs_map_stru));

    msg_idx += MAC_VHT_CAP_RX_MCS_MAP_FIELD_LEN;

    /* 解析rx_highest_supp_logGi_data */
    rx_high_log_gi_data = OAL_MAKE_WORD16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]);
    pst_mac_vht_hdl->rx_highest_rate = rx_high_log_gi_data & (0x1FFF);

    msg_idx += MAC_VHT_CAP_RX_HIGHEST_DATA_FIELD_LEN;

    /* 解析tx_mcs_map */
    tx_mcs_map = OAL_MAKE_WORD16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]);
    (osal_void)memcpy_s(&(pst_mac_vht_hdl->tx_max_mcs_map), OAL_SIZEOF(mac_tx_max_mcs_map_stru),
        &tx_mcs_map, OAL_SIZEOF(mac_tx_max_mcs_map_stru));

    msg_idx += MAC_VHT_CAP_TX_MCS_MAP_FIELD_LEN;

    /* 解析tx_highest_supp_log_gi_data */
    tx_high_log_gi_data = OAL_MAKE_WORD16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]);
    pst_mac_vht_hdl->tx_highest_rate = tx_high_log_gi_data & (0x1FFF);

    hmac_user_set_vht_hdl_etc(hmac_user, pst_mac_vht_hdl);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 函 数 名  : hmac_proc_he_cap_ie
 功能描述  : 解析的11ax capabilities IE信息
*****************************************************************************/
osal_u32 hmac_proc_he_cap_ie(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *he_cap_ie)
{
    mac_he_hdl_stru               *pst_mac_he_hdl = OAL_PTR_NULL;
    mac_he_hdl_stru                mac_he_hdl;
    mac_frame_he_cap_ie_stru       he_cap_value;
    osal_u32                     ul_ret;

    /* 解析he cap IE */
    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (he_cap_ie == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_proc_he_cap_ie::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (he_cap_ie[1] < MAC_HE_CAP_MIN_LEN) {
        oam_warning_log1(0, OAM_SF_11AX, "{hmac_proc_he_cap_ie::invalid he cap ie len[%d].}", he_cap_ie[1]);
        return OAL_FAIL;
    }

    /* 支持11ax，才进行后续的处理 */
    if (mac_mib_get_he_option_implemented(hmac_vap) == OAL_FALSE) {
        return OAL_SUCC;
    }

    pst_mac_he_hdl = &mac_he_hdl;
    mac_user_get_he_hdl(hmac_user, pst_mac_he_hdl);

    /* 解析 HE MAC Capabilities Info */
    memset_s(&he_cap_value, OAL_SIZEOF(he_cap_value), 0, OAL_SIZEOF(he_cap_value));
    ul_ret = hmac_ie_parse_he_cap(he_cap_ie, &he_cap_value);
    if (ul_ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    pst_mac_he_hdl->he_capable = OAL_TRUE;

    (osal_void)memcpy_s(&pst_mac_he_hdl->he_cap_ie, OAL_SIZEOF(he_cap_value), &he_cap_value, OAL_SIZEOF(he_cap_value));

    mac_user_set_he_hdl(hmac_user, pst_mac_he_hdl);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_proc_he_cap_ie
 功能描述  : 解析的11ax capabilities IE信息
*****************************************************************************/
osal_u32 hmac_proc_he_bss_color_change_announcement_ie(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *bss_color_ie)
{
    mac_frame_bss_color_change_annoncement_ie_stru  bss_color_info;
    mac_he_hdl_stru                                *pst_mac_he_hdl = OAL_PTR_NULL;

    if (mac_mib_get_he_option_implemented(hmac_vap) == OAL_FALSE) {
        return OAL_SUCC;
    }

    memset_s(&bss_color_info, OAL_SIZEOF(mac_frame_bss_color_change_annoncement_ie_stru), 0,
        OAL_SIZEOF(mac_frame_bss_color_change_annoncement_ie_stru));
    if (hmac_ie_parse_he_bss_color_change_announcement_ie(bss_color_ie, &bss_color_info) != OAL_SUCC) {
        return OAL_SUCC;
    }

    pst_mac_he_hdl                      = &(hmac_user->he_hdl);
    pst_mac_he_hdl->bss_color       = bss_color_info.new_bss_color;
    pst_mac_he_hdl->bss_color_exist = 1;
    pst_mac_he_hdl->he_oper_ie.bss_color.bss_color = bss_color_info.new_bss_color;

    oam_warning_log1(0, OAM_SF_ANY, "{hmac_proc_he_bss_color_change_announcement_ie::new_bss_color=[%d].}",
        pst_mac_he_hdl->bss_color);

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_add_and_clear_repeat_op_rates
 功能描述  : 将对端速率集中的速率记录到user中，并去除重复速率,当长度超出限制时告警
*****************************************************************************/
OSAL_STATIC osal_void hmac_add_and_clear_repeat_op_rates(osal_u8 *ie_rates, osal_u8 ie_num_rates,
    hmac_rate_stru *op_rates)
{
    osal_u8                ie_rates_idx;
    osal_u8                user_rates_idx;

    for (ie_rates_idx = 0; ie_rates_idx < ie_num_rates; ie_rates_idx++) {
        /* 判断该速率是否已经记录在op中 */
        for (user_rates_idx = 0; user_rates_idx < op_rates->rs_nrates; user_rates_idx++) {
            if (is_equal_rates(ie_rates[ie_rates_idx], op_rates->rs_rates[user_rates_idx])) {
                break;
            }
        }

        /* 相等时，说明ie中的速率与op中的速率都不相同，可以加入op的速率集中 */
        if (user_rates_idx == op_rates->rs_nrates) {
            /* 当长度超出限制时告警，不加入op rates中 */
            if (op_rates->rs_nrates == WLAN_USER_MAX_SUPP_RATES) {
                oam_warning_log0(0, OAM_SF_ANY,
                    "{hmac_add_and_clear_repeat_op_rates::user option rates more then WLAN_USER_MAX_SUPP_RATES.}");
                break;
            }
            op_rates->rs_rates[op_rates->rs_nrates++] = ie_rates[ie_rates_idx];
        }
    }
}

OAL_STATIC osal_u32 hmac_ie_proc_assoc_user_legacy_rate_ext(osal_u8 *payload, osal_u32 rx_len,
    hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap)
{
    osal_u8 *ie = OAL_PTR_NULL;
    osal_u8 num_ex_rates;

    ie = mac_find_ie_etc(MAC_EID_XRATES, payload, (osal_s32)rx_len);
    if (ie != OAL_PTR_NULL) {
        num_ex_rates = ie[1];
        /* AP模式下，协议大于11a且小等11G时，不允许对端携带拓展速率集，STA模式下不做限制 */
        if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
            ((hmac_vap->protocol <= WLAN_LEGACY_11G_MODE) && (hmac_vap->protocol > WLAN_LEGACY_11A_MODE))) {
            oam_warning_log3(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_ie_proc_assoc_user_legacy_rate:invaild xrates rate vap protocol[%d]num_ex_rates[%d]}",
                hmac_vap->vap_id, hmac_vap->protocol, num_ex_rates);
        } else {
            /* 长度告警处理同上 */
            if (num_ex_rates < MAC_MIN_XRATE_LEN) {
                oam_warning_log3(0, OAM_SF_ANY,
                    "vap_id[%d]{extended supported rates length less then MAC_MIN_RATES_LEN vap mode[%d]num_rates[%d]}",
                    hmac_vap->vap_id,
                    hmac_vap->vap_mode, num_ex_rates);
            } else {
                hmac_add_and_clear_repeat_op_rates(ie + MAC_IE_HDR_LEN, num_ex_rates,
                    &(hmac_user->op_rates));
            }
        }
    }

    if (ie == OAL_PTR_NULL && hmac_vap->channel.band == WLAN_BAND_2G) {
        /* 5G不存在扩展速率 */
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_ie_proc_assoc_user_legacy_rate::unsupport xrates vap mode[%d]}",
            hmac_vap->vap_id, hmac_vap->vap_mode);
    }

    if (hmac_user->op_rates.rs_nrates == 0) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_ie_proc_assoc_user_legacy_rate::rate is 0.}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ie_proc_assoc_user_legacy_rate
 功能描述  : 解析帧中legacy 速率集，更新到user的结构体速率变量成员中
*****************************************************************************/
osal_u32 hmac_ie_proc_assoc_user_legacy_rate(osal_u8 *payload, osal_u32 rx_len,
    hmac_user_stru *hmac_user)
{
    osal_u8               *ie = OAL_PTR_NULL;
    hmac_vap_stru            *hmac_vap = OAL_PTR_NULL;
    osal_u8                num_rates;

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ie_proc_assoc_user_legacy_rate::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* user的可选速率集此时应该为空 */
    if (hmac_user->op_rates.rs_nrates != 0) {
        oam_warning_log1(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_ie_proc_assoc_user_legacy_rate::user option rates is not 0.}", hmac_vap->vap_id);
    }

    /* 1.处理基础速率集 */
    ie = mac_find_ie_etc(MAC_EID_RATES, payload, (osal_s32)rx_len);
    /* STAUT兼容性测试在关联请求帧中supported rates长度为0 */
    if (ie != OAL_PTR_NULL) {
        num_rates = ie[1];
        /* 判断supported rates长度，当长度为0时告警，当长度超出限制时，在hmac_add_and_clear_repeat_op_rates里告警，
           extended supported rates同样处理 */
        if (num_rates < MAC_MIN_RATES_LEN) {
            oam_warning_log3(0, OAM_SF_ANY,
                "vap_id[%d] {supported rates length less then MAC_MIN_RATES_LEN vap mode[%d] num_rates[%d]}",
                hmac_vap->vap_id, hmac_vap->vap_mode, num_rates);
        } else {
            hmac_add_and_clear_repeat_op_rates(ie + MAC_IE_HDR_LEN, num_rates, &(hmac_user->op_rates));
        }
    } else {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_ie_proc_assoc_user_legacy_rate::unsupport basic rates vap mode[%d]}",
            hmac_vap->vap_id, hmac_vap->vap_mode);
    }

    /* 2.处理扩展速率集 */
    return hmac_ie_proc_assoc_user_legacy_rate_ext(payload, rx_len, hmac_user, hmac_vap);
}

#ifdef _PRE_WLAN_FEATURE_HS20
/*****************************************************************************
 函 数 名  : hmac_interworking_check
 功能描述  : 关联过程中interworking能力位检查
 输入参数  : param 扫描到的BSS结果
*****************************************************************************/
osal_u32 hmac_interworking_check(hmac_vap_stru *hmac_vap,  osal_u8 *param)
{
    osal_u8           *extend_cap_ie = OAL_PTR_NULL;
    mac_bss_dscr_stru   *bss_dscr = OAL_PTR_NULL;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL) || osal_unlikely(param == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_interworking_check:: check failed, null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    bss_dscr = (mac_bss_dscr_stru *)param;
    if (bss_dscr->mgmt_len < (MAC_80211_FRAME_LEN + MAC_SSID_OFFSET)) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_interworking_check:: mgmt_len(%d) < (80211_FRAME_LEN+SSID_OFFSET).}",
            hmac_vap->vap_id, bss_dscr->mgmt_len);
        return OAL_FAIL;
    }

    /* 查找interworking ie */
    extend_cap_ie = mac_find_ie_etc(MAC_EID_EXT_CAPS,
        bss_dscr->mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET,
        (osal_s32)(bss_dscr->mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET));
    if (extend_cap_ie == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_interworking_check:: extend_cap_ie is NULL, the mgmt_len is %d.}",
            hmac_vap->vap_id, bss_dscr->mgmt_len);
        return OAL_FAIL;
    }

    /*  未检测到interworking能力位，返回fail */
    if (extend_cap_ie[1] < 4 || !(extend_cap_ie[5] & 0x80)) {   /* 4能力范围，5索引值 */
        hmac_vap->cap_flag.is_interworking = OAL_FALSE;
        return OAL_FAIL;
    }

    hmac_vap->cap_flag.is_interworking = OAL_TRUE;
    return OAL_SUCC;
}
#endif // _PRE_WLAN_FEATURE_HS20

/*****************************************************************************
 功能描述  : ADDBA_REQ函数，需要将HMAC模块的生成的信息同步到DMAC模块
*****************************************************************************/
osal_u32 hmac_mgmt_tx_addba_req_send(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *event, oal_netbuf_stru *net_buff)
{
    hmac_tid_stru *tid = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u32 ret;
    frw_msg msg = {0};
    mac_mgmt_tx_addba_req_ba_sync ba = {0};

    /* 获取对应用户及对应的TID队列 */
    if (event->tidno >= WLAN_TID_MAX_NUM) {
        oam_error_log2(0, OAM_SF_BA, "vap[%d] hmac_mgmt_tx_addba_req_send:tidno[%d]", hmac_vap->vap_id, event->tidno);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, net_buff);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }
    tid = &(hmac_user->tx_tid_queue[event->tidno]);

    /* 同步ba_tx_hdl */
    ba.tidno = event->tidno;
    ba.is_ba = OSAL_TRUE;
    ba.ba_conn_status = HMAC_BA_INPROGRESS;
    ba.dialog_token = event->dialog_token;
    ba.ba_policy = event->ba_policy;
    ba.baw_size = event->baw_size;
    ba.ba_timeout = event->ba_timeout;
    ba.mac_user_idx = hmac_user->assoc_id;
    ba.back_var = (hmac_user->ht_hdl.ht_capable == OSAL_TRUE) ? MAC_BACK_COMPRESSED : MAC_BACK_BASIC;

    /* 抛事件至Device侧DMAC，同步ba_tx_hdl */
    frw_msg_init((osal_u8 *)&ba, sizeof(mac_mgmt_tx_addba_req_ba_sync), OSAL_NULL, 0, &msg);
    ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_DEVICE_TX_ADDBA_REQ, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        return ret;
    }

    /* 填写netbuf的cb字段，共发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buff);
    tx_ctl->tx_user_idx = (osal_u8)hmac_user->assoc_id;
    tx_ctl->mpdu_num = 1; /* 管理帧只有一个 */
    tx_ctl->tid = event->tidno;
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    hmac_tid_pause(tid, DMAC_TID_PAUSE_RESUME_TYPE_BA);

    wifi_printf("[AMPDU TX] {hmac_mgmt_tx_addba_req_send::USER ID[%d]TID NO[%d] BAWSIZE[%d] TXAMSDU[%d].}\r\n",
        ba.mac_user_idx, event->tidno, ba.baw_size, event->amsdu_supp);
    common_log_dbg2(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_req_send::tidno[%d] baw_size[%d].}", ba.tidno, ba.baw_size);

    /* 本来应该发送之前将netbuff中的action info删掉，通过传入实际长度，硬件应该不会发送后面的部分 */
    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, net_buff, event->frame_len, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        hmac_ba_reset_tx_handle(hmac_vap, hmac_user->assoc_id, event->tidno);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : ADDBA_RSP函数，需要将HMAC模块的获取的信息同步到DMAC模块
*****************************************************************************/
osal_u32 hmac_mgmt_tx_addba_rsp_send(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *net_buff)
{
    osal_u8 status = ctx_action_event->status;
    hmac_tid_stru *tid = &(hmac_user->tx_tid_queue[ctx_action_event->tidno]);
    hmac_ba_rx_hdl_stru *ba_rx_hdl = &tid->ba_rx_hdl;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u32 ret;
    osal_s32 sync_ret;
    frw_msg msg_info = {0};
    mac_mgmt_tx_addba_rsp_ba_sync ba_sync_stru = {0};

    /* 成功时，同步信息到DMAC模块，失败时，直接发送rsp帧，不同步到DMAC模块  */
    if (status == MAC_SUCCESSFUL_STATUSCODE) {
        ba_rx_hdl->ba_conn_status = HMAC_BA_COMPLETE;
        /* 同步ba_rx_hdl */
        ba_sync_stru.assoc_id = hmac_user->assoc_id;
        ba_sync_stru.tidno = ctx_action_event->tidno;
        ba_sync_stru.ba_conn_status = HMAC_BA_COMPLETE;
        ba_sync_stru.lut_index = ctx_action_event->lut_index;
        ba_sync_stru.ba_policy = ctx_action_event->ba_policy;
        ba_sync_stru.baw_start = ctx_action_event->baw_start;
        ba_sync_stru.baw_size = ctx_action_event->baw_size;
#ifdef _PRE_WLAN_FEATURE_11AX
        ba_sync_stru.ba_32bit_flag = hmac_user->he_hdl.he_cap_ie.he_mac_cap.ba_bitmap_support_32bit;
#endif
        ba_sync_stru.protocol_mode = hmac_user->protocol_mode;
        ba_sync_stru.protocol = hmac_vap->protocol;

        /* 抛事件至Device侧DMAC，同步ba_rx_hdl */
        frw_msg_init((osal_u8 *)&ba_sync_stru, sizeof(mac_mgmt_tx_addba_rsp_ba_sync), OSAL_NULL, 0, &msg_info);
        sync_ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_DEVICE_TX_ADDBA_RSP, &msg_info,
            OSAL_TRUE);
        if (sync_ret != OAL_SUCC) {
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
            return (osal_u32)sync_ret;
        }
    }

    /* 填写netbuf的cb字段，共发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buff);
    tx_ctl->tx_user_idx = (osal_u8)hmac_user->assoc_id;
    tx_ctl->mpdu_num = 1; /* 管理帧只有一个 */
    tx_ctl->tid = ctx_action_event->tidno;
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    wifi_printf("[AMPDU RX] {hmac_mgmt_tx_addba_rsp_send::TID[%d] STATUS[%d] LUT INDEX[%d] BAWSIZE[%d].}",
        ctx_action_event->tidno, status, ctx_action_event->lut_index, ctx_action_event->baw_size);
    common_log_dbg4(0, OAM_SF_TX, "[AMPDU RX] {hmac_mgmt_tx_addba_rsp_send::TID[%d] STATUS[%d] LUT INDEX[%d] \
        BAWSIZE[%d]}", ctx_action_event->tidno, status, ctx_action_event->lut_index, ctx_action_event->baw_size);

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, net_buff, ctx_action_event->frame_len, OSAL_TRUE);

    oam_warning_log4(0, OAM_SF_TX, "{hmac_mgmt_tx_addba_rsp_send::MAC ADDR [%02X:%02X:%02X:%02X:XX:XX].}",
        hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1],
        hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]); /* 1,2,3分别表示hw_addr的第1,2,3位的值 */
    return ret;
}

/*****************************************************************************
 功能描述  : HMAC模块删除BA会话后，DMAC需要进行同步的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_tx_delba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *event, oal_netbuf_stru *net_buff)
{
    hmac_tid_stru *tid = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u32 sync_ret;
    frw_msg msg_info = {0};
    mac_mgmt_delba_ba_sync ba_sync_stru = {0};

    tid = &(hmac_user->tx_tid_queue[event->tidno]);
    ba_sync_stru.assoc_id = hmac_user->assoc_id;
    ba_sync_stru.tidno = event->tidno;
    ba_sync_stru.initiator = event->initiator;
    ba_sync_stru.is_tx = OSAL_TRUE;

    if (event->initiator == MAC_ORIGINATOR_DELBA) {
        /* 开启TID队列状态 */
        hmac_tid_resume(tid, DMAC_TID_PAUSE_RESUME_TYPE_BA);
    } else {
        tid->ba_rx_hdl.ba_conn_status = HMAC_BA_INIT;
    }

    /* 抛事件至Device侧DMAC，同步重置会话 */
    frw_msg_init((osal_u8 *)&ba_sync_stru, sizeof(mac_mgmt_delba_ba_sync), OSAL_NULL, 0, &msg_info);
    sync_ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_DEVICE_DELBA, &msg_info, OSAL_TRUE);
    if (sync_ret != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        return sync_ret;
    }

    /* 填写netbuf的cb字段，共发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buff);
    tx_ctl->tx_user_idx = (osal_u8)hmac_user->assoc_id;
    tx_ctl->mpdu_num = 1; /* 管理帧只有一个 */
    tx_ctl->tid = event->tidno;
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    wifi_printf("{hmac_mgmt_tx_delba::USER ID[%d] TID[%d] DIRECTION[%d].}\n", ba_sync_stru.assoc_id,
        ba_sync_stru.tidno, ba_sync_stru.initiator);

    /* 调用发送管理帧接口 */
    return hmac_tx_mgmt(hmac_vap, net_buff, event->frame_len, OSAL_TRUE);
}

OSAL_STATIC osal_void hmac_mgmt_rx_addba_rsp_debug(mac_mgmt_rx_addba_rsp_ba_sync *ba_sync_stru)
{
    wifi_printf("[AMPDU TX] hmac_mgmt_rx_addba_rsp tidno[%d] \
        status[%d] dialog_token[%d] ba_policy[%d] ba_timeout[%d], baw_size[%d] amsdu_supp[%d] assoc_id[%d] \
        max_rx_ampdu_factor[%d] max_ampdu_len_exp[%d] protocol_mode[%d] min_mpdu_start_spacing[%d]\r\n",
        ba_sync_stru->tidno, ba_sync_stru->status, ba_sync_stru->dialog_token, ba_sync_stru->ba_policy,
        ba_sync_stru->ba_timeout, ba_sync_stru->baw_size, ba_sync_stru->amsdu_supp, ba_sync_stru->assoc_id,
        ba_sync_stru->max_rx_ampdu_factor, ba_sync_stru->max_ampdu_len_exp, ba_sync_stru->protocol_mode,
        ba_sync_stru->min_mpdu_start_spacing);

    common_log_dbg4(0, OAM_SF_BA, "{hmac_mgmt_rx_addba_rsp::tidno[%d] status[%d] baw_size[%d] amsdu_supp[%d]}",
        ba_sync_stru->tidno, ba_sync_stru->status, ba_sync_stru->baw_size, ba_sync_stru->amsdu_supp);
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_rx_addba_rsp
 功能描述  : 从空口接收到相应帧的处理
*****************************************************************************/
osal_u32 hmac_mgmt_rx_addba_rsp(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const hmac_ctx_action_event_stru *crx_action_event)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 tidno;
    osal_u32 sync_ret;
    frw_msg msg_info = {0};
    mac_mgmt_rx_addba_rsp_ba_sync ba_sync_stru = {0};

    if ((hmac_device == OSAL_NULL) || (hmac_vap == OSAL_NULL) || (crx_action_event == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_tx_delba::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(crx_action_event->user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_addba_rsp::hmac_user[%d] null.}",
            hmac_vap->vap_id, crx_action_event->user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    tidno = crx_action_event->tidno;
    if (tidno >= WLAN_TID_MAX_NUM) {
        oam_error_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_addba_rsp::invalid tidno:%d}", hmac_user->vap_id, tidno);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    hmac_tid_resume(&(hmac_user->tx_tid_queue[tidno]), DMAC_TID_PAUSE_RESUME_TYPE_BA);

    /* 同步ba_rx_hdl */
    ba_sync_stru.assoc_id = crx_action_event->user_idx;
    ba_sync_stru.tidno = crx_action_event->tidno;
    ba_sync_stru.status = crx_action_event->status;
    ba_sync_stru.dialog_token = crx_action_event->dialog_token;
    ba_sync_stru.ba_policy = crx_action_event->ba_policy;
    ba_sync_stru.ba_timeout = crx_action_event->ba_timeout;
    ba_sync_stru.baw_size = crx_action_event->baw_size;
    ba_sync_stru.amsdu_supp = crx_action_event->amsdu_supp;
    ba_sync_stru.max_rx_ampdu_factor = hmac_user->ht_hdl.max_rx_ampdu_factor;
    ba_sync_stru.max_ampdu_len_exp = hmac_user->vht_hdl.max_ampdu_len_exp;
    ba_sync_stru.protocol_mode = hmac_user->protocol_mode;
    ba_sync_stru.min_mpdu_start_spacing = hmac_user->ht_hdl.min_mpdu_start_spacing;

    hmac_mgmt_rx_addba_rsp_debug(&ba_sync_stru);

    /* 抛事件至Device侧DMAC，同步ba_rx_hdl */
    frw_msg_init((osal_u8 *)&ba_sync_stru, sizeof(mac_mgmt_rx_addba_rsp_ba_sync), OSAL_NULL, 0, &msg_info);
    sync_ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_DEVICE_RX_ADDBA_RSP, &msg_info,
        OSAL_TRUE);
    if (sync_ret != OAL_SUCC) {
        common_log_dbg1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_addba_rsp::sync_ret fail}", hmac_user->vap_id);
        (osal_void)hmac_mgmt_delba(hmac_vap, hmac_user, tidno, MAC_ORIGINATOR_DELBA, MAC_QSTA_SETUP_NOT_DONE);
        return sync_ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 从空口接收到DELBA帧的处理(HMAC模块抛事件同步到DMAC调用该函数)
*****************************************************************************/
osal_u32 hmac_mgmt_rx_delba(const hmac_vap_stru *hmac_vap, const hmac_ctx_action_event_stru *crx_action_event)
{
    hmac_user_stru     *hmac_user = OSAL_NULL;
    hmac_tid_stru      *tid = OSAL_NULL;
    osal_u8           tidno;
    osal_u8           initiator;
    osal_s32 sync_ret;
    frw_msg msg_info = {0};
    mac_mgmt_delba_ba_sync ba_sync_stru = {0};

    if ((hmac_vap == OSAL_NULL) || (crx_action_event == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_BA,
                       "{hmac_mgmt_rx_delba::param null, device=%d hmac_vap=%d crx_action_event=%d.}",
                       (uintptr_t)hmac_vap, (uintptr_t)crx_action_event);

        return OAL_ERR_CODE_PTR_NULL;
    }

    tidno      = crx_action_event->tidno;
    initiator  = crx_action_event->initiator;

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(crx_action_event->user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_delba::hmac_user[%d] null.}",
            hmac_vap->vap_id, crx_action_event->user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (tidno >= WLAN_TID_MAX_NUM) {
        oam_error_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_rx_delba::tidno over flow:%d}",
            hmac_user->vap_id, tidno);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    tid = &(hmac_user->tx_tid_queue[tidno]);
    if (initiator == MAC_RECIPIENT_DELBA) {
        /* 开启TID队列状态 */
        hmac_tid_resume(tid, DMAC_TID_PAUSE_RESUME_TYPE_BA);
    } else {
        tid->ba_rx_hdl.ba_conn_status = HMAC_BA_INIT;
    }

    ba_sync_stru.assoc_id = hmac_user->assoc_id;
    ba_sync_stru.tidno = tidno;
    ba_sync_stru.initiator = initiator;
    ba_sync_stru.is_tx = OSAL_FALSE;

    /* 抛事件至Device侧DMAC，同步重置会话 */
    frw_msg_init((osal_u8 *)&ba_sync_stru, sizeof(mac_mgmt_delba_ba_sync), OSAL_NULL, 0, &msg_info);
    sync_ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_DEVICE_DELBA, &msg_info, OSAL_TRUE);
    if (sync_ret != OAL_SUCC) {
        return (osal_u32)sync_ret;
    }

    wifi_printf("{hmac_mgmt_rx_delba::USER ID[%d], TID NO[%d], DIRECTION[%d]}\n",
        ba_sync_stru.assoc_id, ba_sync_stru.tidno, ba_sync_stru.initiator);
    return OAL_SUCC;
}

osal_u32 hmac_mgmt_scan_vap_down(const hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* vap down的时候如果设备处于扫描状态，并且down掉的vap和扫描的vap是同一个，则停止扫描 */
    if ((hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) &&
        (hmac_vap->vap_id == hmac_device->scan_params.vap_id)) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_mgmt_scan_vap_down::stop scan.}", hmac_vap->vap_id);
        hmac_scan_abort(hmac_device);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_delba
 功能描述  : DMAC触发删除BA会话的接口
*****************************************************************************/
osal_u32 hmac_mgmt_delba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 tid, osal_u8 initiator,
    osal_u8 reason)
{
    frw_msg msg = {0};
    dmac_ctx_action_event_stru wlan_ctx_action;

    if ((hmac_vap == OSAL_NULL) || (hmac_user == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_delba::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&wlan_ctx_action, sizeof(wlan_ctx_action), 0, sizeof(wlan_ctx_action));
    wlan_ctx_action.tidno = tid;
    wlan_ctx_action.status = reason;
    wlan_ctx_action.initiator = initiator;
    (osal_void)memcpy_s(wlan_ctx_action.mac_addr, WLAN_MAC_ADDR_LEN,
        hmac_user->user_mac_addr, WLAN_MAC_ADDR_LEN);

    msg.data = (osal_u8 *)&wlan_ctx_action;
    msg.data_len = sizeof(wlan_ctx_action);
    hmac_mgmt_rx_delba_event_etc(hmac_vap, &msg);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_set_cap_info_field
 功能描述  : hmac组帧设置cap info field
*****************************************************************************/
osal_void hmac_set_cap_info_field(hmac_vap_stru *hmac_vap, osal_u8 *buffer)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_cap_info_stru *cap_info = (mac_cap_info_stru *)buffer;
#endif

    hmac_set_cap_info_ap_etc(hmac_vap, buffer);

#ifdef _PRE_WLAN_FEATURE_P2P
    /* 解决P2P模式下Listen 状态下发送的帧的capability info ess bit不为0的问题 */
    if (hmac_vap_is_in_p2p_listen()) {
        cap_info->ess = 0;
    }
#endif
}
/*****************************************************************************
 函 数 名  : hmac_get_dsss_ie_channel_num
 功能描述  : 获取设置dsss ie的channel num
*****************************************************************************/
osal_u8 hmac_get_dsss_ie_channel_num(const hmac_vap_stru *hmac_vap, osal_u8 scan_flag)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 chan_num;
    osal_u8 scan_chan_idx;

    if ((hmac_vap == OSAL_NULL) || (hmac_vap->hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_get_dsss_ie_channel_num::hmac_vap or hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    chan_num = hmac_vap->channel.chan_number;
    scan_chan_idx = get_hal_dev_current_scan_idx(hmac_vap->hal_device);

    /* iwlist中，如果是ap执行扫描，需要将此处的信道号修改，所以在扫描结果中值需要判断scan_flag是不是为1，不区分sta和ap */
    if ((is_sta(hmac_vap) || (scan_flag == 1)) && (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING)) {
        if (scan_chan_idx < WLAN_MAX_CHANNEL_NUM) {
            chan_num = hmac_device->scan_params.channel_list[scan_chan_idx].chan_number;
        }
    }

    return chan_num;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
